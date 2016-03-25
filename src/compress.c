#include "compress.h"
//解压缩
void psb_pixel_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align)
{
	int i;
	int count;
	uint32_t totalBytes = 0;
	int cmdByte = 0;

	while (actualSize != totalBytes)
	{
		cmdByte = *pInput++; totalBytes++;

		if (cmdByte & PSB_LZSS_LOOKAHEAD)
		{
			count = (cmdByte ^ PSB_LZSS_LOOKAHEAD) + 3;

			for (i = 0; i < count; i++)
			{
				memcpy(pOutput, pInput, align);
				pOutput += align;
			}

			pInput += align; totalBytes += align;
		}
		else {
			count = (cmdByte + 1) * align;

			for (i = 0; i < count; i++) {
				*pOutput++ = *pInput++;
			}

			totalBytes += count;
		}
	}
}
//查找相等的数据数量
int psb_pixel_compress_bound(unsigned char *data, const unsigned char *end, uint32_t align, unsigned char *result)
{
	uint32_t count = 0;
	unsigned char *p;
	unsigned char temp[4];

	int i;


	memcpy(temp, data, align);

	for (i = 0; i < (PSB_LZSS_LOOKAHEAD + 2); i++)
	{
		p = &data[i * align];		//下一个位置

		if (p >= end) {		//是否读取到数据尾
			break;
		}

		if (memcmp(temp, &data[i * align], align) == 0) {	//比较数据相等
			count++;
		}
		else {
			break;
		}
	}

	//相等数据最少3个起步
	if (count >= 3) {
		*result = (count - 3) | PSB_LZSS_LOOKAHEAD;	//设置数据相等标记位
		return count;
	}

	return 0;
}
//查找不相等的数据数量
int psb_pixel_compress_bound_np(unsigned char *data, const unsigned char *end, uint32_t align, unsigned char *result)
{
	uint32_t count = 1;
	unsigned char *p;
	unsigned char temp[4];

	int i;

	memcpy(temp, data, align);

	for (i = 1; i < PSB_LZSS_LOOKAHEAD; i++)
	{
		p = &data[i * align];//下一个位置
		if(p >= end) break;
		if (psb_pixel_compress_bound(p, end, align, result) == 0) {	//查找不相等的数据
			count++;
		}
		else {
			break;
		}
	}
	*result = (count - 1);	//不相等数据从1开始
	return count;
}

//使用PSB的pixel算法压缩一段图片数据
unsigned char* psb_pixel_compress(unsigned char *data, uint32_t length, uint32_t align, uint32_t *actualSize)
{
	unsigned char *result, *p, *end;
	unsigned char cmdByte;
	uint32_t pos, blockSize;
	int count;

	p = data;
	end = data + length;
	pos = 0;

	result = malloc(0);

	while (p != end)
	{
		count = psb_pixel_compress_bound(p, end, align, &cmdByte);

		if (count > 0) {
			//写入相同的数据展开
			blockSize = align + sizeof(cmdByte);
			result = realloc(result, pos + blockSize);
			result[pos] = cmdByte;
			memcpy(&result[pos + sizeof(cmdByte)], p, align);
		}
		else {
			count = psb_pixel_compress_bound_np(p, end, align, &cmdByte);
			//写入不同的数据展开
			blockSize = count * align + sizeof(cmdByte);
			result = realloc(result, pos + blockSize);
			result[pos] = cmdByte;
			memcpy(&result[pos + sizeof(cmdByte)], p, count * align);
		}

		pos += blockSize;
		p += count * align;
	}

	*actualSize = pos;
	return result;
}