/* FastVideoDS FVDS picture-prefix metadata, independently parsed.
 * This does NOT decode 4x4 DCT, motion vectors or RGB555 pixels.
 * The first little-endian 16-bit word carries the I/P high-bit flag.
 */
#ifndef CPYMO_NDS_FVDS_PICTURE_PREFIX_H
#define CPYMO_NDS_FVDS_PICTURE_PREFIX_H
#include <stddef.h>
#include <stdint.h>
/* -1: too short; -2: indexed keyframe starts with P picture
 *  0: I-picture; 1: P-picture. */
static inline int nds_fvds_picture_type(const uint8_t *bytes,size_t length,
                                       int indexed_keyframe){
 if(!bytes||length<2)return -1;
 unsigned prefix=(unsigned)bytes[0]|((unsigned)bytes[1]<<8);
 int predicted=(prefix&0x8000u)?1:0;
 if(indexed_keyframe&&predicted)return -2;
 return predicted;
}
#endif
