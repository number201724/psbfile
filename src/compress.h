#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>

#define PSB_LZSS_LOOKSHIFT		7
#define PSB_LZSS_LOOKAHEAD		( 1 << PSB_LZSS_LOOKSHIFT )


void psb_pixel_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align);
unsigned char* psb_pixel_compress(unsigned char *data, uint32_t length, uint32_t align, uint32_t *actualSize);


#ifdef __cplusplus
};
#endif

#endif