/* CPyMO 1.2 optional native lower-screen artwork.
 * A single preprocessed 256x192 RGB555LE image lives on the TF card:
 *   /pymo/nds-dual-bg.rgb555
 * Magic "CPYBG12\\0", followed by 49152 little-endian RGB555 words.
 * Source: owner-approved old concept 02-B, not fabricated C stars.
 * Loaded only when the enhanced narrative screen is first drawn.
 * No SDK headers, heap growth per frame, PNG decoder or image scaling.
 */
#ifndef CPYMO_NDS_PICTURE_BG_H
#define CPYMO_NDS_PICTURE_BG_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NDS_PICTURE_PIXELS (256u * 192u)
#define NDS_PICTURE_PATH "/pymo/nds-dual-bg.rgb555"
static uint16_t* nds_picture_pixels;
static bool nds_picture_checked;

static const uint16_t* nds_picture_get(void){
 if(nds_picture_checked)return nds_picture_pixels;
 nds_picture_checked=true;
 FILE* file=fopen(NDS_PICTURE_PATH,"rb");
 if(!file)return NULL;
 static const unsigned char signature[8]={'C','P','Y','B','G','1','2',0};
 unsigned char header[8];
 if(fread(header,1,sizeof header,file)!=sizeof header ||
    memcmp(header,signature,sizeof header)!=0){
  fclose(file);return NULL;
 }
 uint16_t* px=(uint16_t*)malloc(NDS_PICTURE_PIXELS*sizeof(uint16_t));
 if(!px){fclose(file);return NULL;}
 size_t n=fread(px,sizeof(uint16_t),NDS_PICTURE_PIXELS,file);
 int extra=fgetc(file);
 fclose(file);
 if(n!=NDS_PICTURE_PIXELS || extra!=EOF){free(px);return NULL;}
 /* RGB555 bit15 (opaque) is mandatory for every pixel in the native bitmap.
  * Any corrupt or incompatible asset fails closed to the old safe canvas. */
 for(unsigned i=0;i<NDS_PICTURE_PIXELS;i++){
  if(!(px[i]&0x8000u)){free(px);return NULL;}
 }
 nds_picture_pixels=px;
 return nds_picture_pixels;
}
#endif
