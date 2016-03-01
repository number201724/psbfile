#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "compress.h"

void lzss_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align)
{
	uint32_t i;
	int count;
	uint32_t totalBytes = 0;
	int cmdByte = 0;
	unsigned char *max,*p;
	
	for (; actualSize != totalBytes; )
	{
		cmdByte = *pInput++; totalBytes++;

		if (cmdByte & PSB_LZSS_LOOKAHEAD)
		{
			count = (cmdByte ^ PSB_LZSS_LOOKAHEAD) + 3;
			
			for(i = 0; i < count; i++)
			{
				memcpy(pOutput, pInput, align);
				pOutput += align;
			}
			
			pInput += align; totalBytes += align;
		}
		else {
			count = (cmdByte + 1) * align;

			for (int i = 0; i < count; i++) {
				*pOutput++ = *pInput++;
			}

			totalBytes += count;
		}
	}
}

uint32_t lzss_compress_np_count(const unsigned char *start, const unsigned char *end, const unsigned char* find, uint32_t align)
{
	uint32_t c = 0;
	
	return c;
}


unsigned char lzss_compress_bound(const unsigned char *start, const unsigned char *end, const unsigned char* find, uint32_t *count, uint32_t align)
{
	unsigned char type = 0;
	
	
	
	return type;
}
unsigned char* lzss_compress(const unsigned char *data, uint32_t length, uint32_t align, uint32_t *actualSize)
{
	unsigned char *result;
/*	const unsigned char *end, *p;
	uint32_t count;
	uint32_t position = 0;
	
	result = (unsigned char*)malloc( length );
	p = data;
	end = data + length;
	
	while(end != p)
	{
		count = match_count(p, end, p, align);
		
		printf("%08x    %d\n",p, count);
		p += count * align;
		
		//break;
	}
	
*/
	return result;
}