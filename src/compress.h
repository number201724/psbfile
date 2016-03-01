#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PSB_LZSS_LOOKSHIFT		7
#define PSB_LZSS_LOOKAHEAD		( 1 << PSB_LZSS_LOOKSHIFT )


void lzss_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align);
unsigned char* lzss_compress(const unsigned char *data, uint32_t length, uint32_t align, uint32_t *actualSize);


#ifdef __cplusplus
};
#endif

#endif