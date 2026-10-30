/* FVDS NDS GPU copy probe (R14.1).
 *
 * PRODUCT 0 = reference: the displacement is applied by the CPU, the result is
 *              what a bitmap blit would show.
 * PRODUCT 1 = the same displacement executed by the 3D engine, sampling a
 *              texture uploaded once.
 *
 * SCEN 0 = no motion, 1 = integer vectors, 2 = half pixel vectors,
 *      3 = mixed motion compensation pattern.
 *
 * Coordinates are in t16 (1/16 px), so half a pixel is exactly 8.  The
 * destination pixel centre n sits at (n*16+8)*2 world units and source pixel
 * m maps to texture coordinate m*16+8; with that alignment the hardware
 * bilinear filter degenerates to nearest neighbour and the GPU result has to
 * match the CPU result exactly.
 *
 * Both products upload their result as a texture and draw it with identical
 * quads, so the only difference between the runs is who computed the texels.
 */
#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "refplanes.h"
#ifndef OFFS
#define OFFS 0
#endif
#ifndef TSC
#define TSC 2
#endif
#ifndef OFFD
#define OFFD 0
#endif

#define W 256
#define H 192
#define BLOCKX 32
#define BLOCKY 32
#define NCOL 8
#define NROW 6
#define NT 4   /* 4 tiles of 128x128, 32 KiB each */

static u16 tex[NT][128 * 128];
static u8 out[W * H];
static u16 fb[W * H];
static int tid[NT];

static unsigned long long t_up, t_pass, t_sync;
static unsigned fb_nz, band[12];
static unsigned qcount, qmin, qmax, qminy, cy0max;

static char dbgline[200];
static void dapp(const char *t)
{
    if (*t && strlen(dbgline) < sizeof(dbgline) - 16) strcat(dbgline, t);
}
static void dnum(unsigned v)
{
    char b[12]; int i = 0, j;
    char t[12];
    if (v == 0) { dapp("0"); return; }
    while (v) { b[i++] = (char)('0' + v % 10); v /= 10; }
    for (j = 0; j < i; j++) t[j] = b[i - 1 - j];
    t[i] = 0;
    dapp(t);
}
static void putdec(FILE *f, unsigned v)
{
    char b[12];
    int n = 0;
    if (!v) { fputc('0', f); return; }
    while (v) { b[n++] = '0' + (v % 10); v /= 10; }
    while (n) fputc(b[--n], f);
}

static void puthex(FILE *f, unsigned v)
{
    static const char h[] = "0123456789abcdef";
    int i;
    for (i = 28; i >= 0; i -= 4) fputc(h[(v >> i) & 15], f);
}

static int vec_x(int col, int row)
{
    switch (SCEN) {
    case 0: return 0;
    case 1: return (((col + row) % 5) - 2) * 16 + 64;
    case 2: return (((col * 3 + row) % 5) - 2) * 8 + 32;
    case 4: return 0;
    case 5: return 0;
    case 6: return (((col + row) % 5) - 2) * 32 + 128;
    case 7: return 0;
    default: return (((col * 3 + row) % 5) - 2) * 8 + 32;
    }
}

static int vec_y(int col, int row)
{
    switch (SCEN) {
    case 0: return 0;
    case 1: return (((col * 2 + row) % 3) - 1) * 16 + 32;
    case 2: return (((col + row * 2) % 3) - 1) * 8 + 16;
    case 4: return 32;
    case 5: return 16;
    case 6: return 0;
    case 7: return 48;
    default: return (((col * 2 + row) % 4) - 1) * 8 + 16;
    }
}

static int src_x(int col, int row) { return col * BLOCKX * 16 + vec_x(col, row); }
static int src_y(int col, int row) { return row * BLOCKY * 16 + vec_y(col, row); }

static int inside(int col, int row)
{
    int x = src_x(col, row), y = src_y(col, row);
    return x >= 0 && y >= 0 && x + BLOCKX * 16 <= W * 16 && y + BLOCKY * 16 <= H * 16;
}

/* PRODUCT 0: the CPU copy, nearest neighbour, into the 128x128 window */
static void fill_out(void)
{
    int col, row, r, c;
    memset(out, 0, sizeof out);
    for (row = 0; row < NROW; row++)
        for (col = 0; col < NCOL; col++) {
            int sx = src_x(col, row) >> 4, sy = src_y(col, row) >> 4;
            if (!inside(col, row)) continue;
            for (r = 0; r < BLOCKY; r++)
                for (c = 0; c < BLOCKX; c++)
                    out[(row * BLOCKY + r) * 256 + col * BLOCKX + c] =
                        ref_planes[0][(sy + r) * W + sx + c];
        }
}

static void fill_tex(void)
{
    int t, x, y;
    for (t = 0; t < NT; t++)
        for (y = 0; y < 128; y++)
            for (x = 0; x < 128; x++) {
                int sx = x + (t & 1) * 128, sy = y + (t >> 1) * 128;
                unsigned v;
                if (sx >= W || sy >= H) v = 0;
                else if (PRODUCT == 0) v = out[sy * W + sx];
                else v = ref_planes[0][sy * W + sx];
#ifdef RAMP
                {
                    unsigned rr = ((unsigned)(sx >> 4) & 15) * 2;
                    unsigned gg = ((unsigned)(sy >> 4) & 15) * 2;
                    tex[t][y * 128 + x] = (u16)(0x8000 | rr | (gg << 5));
                    (void)v;
                }
#else
                {
                    unsigned q = (v >> 3) & 31;
                    tex[t][y * 128 + x] = (u16)(0x8000 | q | (q << 5) | (q << 10));
                }
#endif
            }
}

static void setup_textures(void)
{
    unsigned long long a = cpuGetTiming(), b;
    vramSetBankA(VRAM_A_TEXTURE);
    vramSetBankB(VRAM_B_TEXTURE);
    vramSetBankC(VRAM_C_TEXTURE);
    vramSetBankD(VRAM_D_MAIN_BG);
    fill_tex();
    DC_FlushRange(tex, sizeof tex);
    {
        int t;
        for (t = 0; t < NT; t++) {
            glGenTextures(1, &tid[t]);
            glBindTexture(0, tid[t]);
            glTexImage2D(0, 0, GL_RGB, TEXTURE_SIZE_128, TEXTURE_SIZE_128,
                         0, TEXGEN_TEXCOORD, tex[t]);
        }
    }
    b = cpuGetTiming();
    t_up = (b - a) / 33;
}

static void draw(void)
{
    dbgline[0] = 0;
    unsigned long long a = cpuGetTiming(), b;
    int t, row, col, q;
    unsigned qn = 0, cmin = 65535, dbg = 0, cmax = 0, dmin = 65535, dmax = 0;
    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0));

#ifdef ONELIST
        glBegin(GL_QUADS);
        glNormal(NORMAL_PACK(0, inttov10(-1), 0));
        glColor3b(255, 255, 255);
#endif
        for (t = 0; t < NT; t++) {
        int ox = (t & 1) * 128, oy = (t >> 1) * 128;
        glBindTexture(0, tid[t]);
#ifdef ONELIST
#else
        glBegin(GL_QUADS);
        glNormal(NORMAL_PACK(0, inttov10(-1), 0));
        glColor3b(255, 255, 255);
#endif
        for (row = 0; row < NROW; row++)
            for (col = 0; col < NCOL; col++) {
                int dx = col * BLOCKX, dy = row * BLOCKY;
                int x0, x1, y0, y1, cx0, cy0;
                if (!inside(col, row)) continue;
                if (((dx >> 7) | ((dy >> 7) << 1)) != t) continue;
                x0 = (ox + dx - 128) * 32;
                x1 = (ox + dx + BLOCKX - 128) * 32;
                y0 = (96 - (oy + dy)) * 128 / 3;
                y1 = (96 - (oy + dy + BLOCKY)) * 128 / 3;
                cx0 = (src_x(col, row) - ox * 16) * TSC;
                cy0 = (src_y(col, row) - oy * 16) * TSC;
                qn++;
                if (dbg < 8) {
                    dbg++;
                    dapp(" Q"); dnum(qn); dapp(":"); dnum(t);
                    dapp(","); dnum((unsigned)x0); dapp(","); dnum((unsigned)y0);
                    dapp(","); dnum((unsigned)cx0); dapp(","); dnum((unsigned)cy0);
                }
                if ((unsigned)cx0 < cmin) cmin = cx0;
                if ((unsigned)cx0 > cmax) cmax = cx0;
                GFX_TEX_COORD = TEXTURE_PACK(cx0, cy0 + (BLOCKY - 1) * 16 * TSC);
                glVertex3v16(x0, y0, 0);
                GFX_TEX_COORD = TEXTURE_PACK(cx0 + BLOCKX * 16 * TSC, cy0 + (BLOCKY - 1) * 16 * TSC);
                glVertex3v16(x1, y0, 0);
                GFX_TEX_COORD = TEXTURE_PACK(cx0 + BLOCKX * 16 * TSC, cy0);
                glVertex3v16(x1, y1, 0);
                GFX_TEX_COORD = TEXTURE_PACK(cx0, cy0);
                glVertex3v16(x0, y1, 0);
            }
#ifndef ONELIST
        glEnd();
#endif
    }
#ifdef ONELIST
        glEnd();
#endif
    glFlush(0);
    b = cpuGetTiming();
    t_pass = (b - a) / 33;
    qcount = qn; qmin = cmin; qmax = cy0max = dmax; qminy = dmin;
    a = cpuGetTiming();
    while ((GFX_STATUS & (1u << 27)) && (cpuGetTiming() - a) < 100000) ;
    t_sync = (cpuGetTiming() - a) / 33;
}

int main(void)
{
    int logged = 0;
    (void)fb;

    cpuStartTiming(1);
    videoSetMode(MODE_0_3D);
    glInit();
#ifndef NOTEX
    glEnable(GL_TEXTURE_2D);
#endif
    glEnable(GL_ANTIALIAS);
    glClearColor(31, 0, 0, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);
    glViewport(0, 0, 255, 191);

    /* identity projection: vertices are given in normalised device units */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (PRODUCT == 0) fill_out();
    setup_textures();
    fatInitDefault();

    for (;;) {
        int x, y;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        if (0) glClearColor(0, 0, 31, 31);
        else glClearColor(31, 0, 0, 31);
        draw();
#ifdef TWICE
        draw();
#endif
        swiWaitForVBlank();
        glPopMatrix(1);
        glFlush(0);
        swiWaitForVBlank();

        if (++logged == 2) {
            FILE *f;
            fb_nz = 0;
            band[0] = out[256 * 100];
            band[1] = out[256 * 101];
            band[2] = out[256 * 150];
            band[3] = tex[0][128 * 100 + 50];
            band[4] = tex[1][128 * 100 + 50];
            band[5] = tex[2][128 * 20 + 50];
            f = fopen("fat:/gpu.txt", "w");
            if (f) {
                fputs("prod=", f); putdec(f, PRODUCT);
                fputs(" scen=", f); putdec(f, SCEN);
                fputs(" up_us=", f); putdec(f, (unsigned)t_up);
                fputs(" pass_us=", f); putdec(f, (unsigned)t_pass);
                fputs(" sync_us=", f); putdec(f, (unsigned)t_sync);
                fputs(" fb_nz=", f); putdec(f, fb_nz);
                fputs(" quads=", f); putdec(f, qcount);
                fputs(" cmin=", f); putdec(f, qmin);
                fputs(" cmax=", f); putdec(f, qmax);
                fputs(" cymin=", f); putdec(f, qminy);
                fputs(" cymax=", f); putdec(f, cy0max);
                fputs(" o100=", f); putdec(f, band[0]);
                fputs(" o150=", f); putdec(f, band[2]);
                fputs(" t100=", f); putdec(f, band[3]);
                fputs(" t150=", f); putdec(f, band[4]);
                fputs(" t20=", f); putdec(f, band[5]);
                fputs(" o=", f); putdec(f, out[0]); fputc(',', f); putdec(f, out[1]); fputc(',', f); putdec(f, out[16448]);
                fputs(" r=", f); putdec(f, ref_planes[0][0]); fputc(',', f); putdec(f, ref_planes[0][1]); fputc(',', f); putdec(f, ref_planes[0][16448]);
                { void *tp = glGetTexturePointer(tid[0]); unsigned short *t = tp;
                  fputs(" v0=", f); puthex(f, t[0]);
                  fputs(" v128=", f); puthex(f, t[128]);
                  fputs(" v5000=", f); puthex(f, t[5000]); }
                fputs(dbgline, f);
                fputs(" tex0=", f); puthex(f, (unsigned)(uintptr_t)glGetTexturePointer(tid[0]));
                fputs(" vram=", f);
                puthex(f, REG_VRAMCNT_A); fputc(' ', f);
                puthex(f, REG_VRAMCNT_B); fputc('\n', f);
                fclose(f);
            }
        }
    }
    return 0;
}
