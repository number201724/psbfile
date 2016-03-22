/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include "def.h"
#include <direct.h>
#include "../compress.h"

#define BMP_BIT 32

#pragma pack(1)

struct BITMAP_FILE_HDR {
	unsigned short    bfType;
	unsigned int	  bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned int	  bfOffBits;
};

struct BITMAP_INFO_HDR {
	unsigned int      biSize;
	unsigned int      biWidth;
	unsigned int      biHeight;
	unsigned short    biPlanes;
	unsigned short    biBitCount;
	unsigned int      biCompression;
	unsigned int      biSizeImage;
	unsigned int      biXPelsPerMeter;
	unsigned int      biYPelsPerMeter;
	unsigned int      biClrUsed;
	unsigned int      biClrImportant;
};
#pragma pack()

void convert_pixel(unsigned char *data, int width, int height, int align)
{
	int widthSize = width * align;

	unsigned char *temp = new unsigned char[widthSize * height];

	for (int i = 0; i < height; i++)
	{
		unsigned char *dst = &temp[((height - i) - 1) * widthSize];
		unsigned char *src = &data[i * widthSize];
		memcpy(dst, src, widthSize);
	}

	memcpy(data, temp, widthSize * height);

	delete[] temp;
}

int main(int argc,char* argv[])
{
	uint32_t length;
	unsigned char *data;
	BITMAP_FILE_HDR *hdr;
	BITMAP_INFO_HDR *info_hdr;

	if (argc < 2)
	{
		cout << "usage: rl_convert <image.bmp> <output>" << endl;
		return 0;
	}

	fstream bitmap_file(argv[1], ios::in | ios::binary);
	fstream output_file(argv[2], ios::out | ios::binary | ios::trunc);

	if (!bitmap_file.is_open())
	{
		cout << "can't open bitmap file" << endl;
		return 1;
	}

	if (!output_file.is_open())
	{
		cout << "can't open output file" << endl;
		return 1;
	}


	bitmap_file.seekg(0, ios::end);
	length = (uint32_t)bitmap_file.tellg();
	bitmap_file.seekg(0, ios::beg);
	data = new unsigned char[length];
	bitmap_file.read((char*)data, length);
	bitmap_file.close();

	hdr = (BITMAP_FILE_HDR*)data;

	if(hdr->bfType != 0x4D42)
	{
		cout << "invalid bmp file" << endl;
		return 1;
	}

	info_hdr = (BITMAP_INFO_HDR*)&data[sizeof(BITMAP_FILE_HDR)];

	uint32_t actualSize;
	unsigned char *pixel = &data[hdr->bfOffBits];
	uint32_t pixel_size = info_hdr->biHeight * info_hdr->biWidth * (info_hdr->biBitCount / 8);
	convert_pixel(pixel, info_hdr->biWidth, info_hdr->biHeight, (info_hdr->biBitCount / 8));
	unsigned char *compr_data = psb_pixel_compress(pixel, pixel_size, (info_hdr->biBitCount / 8), &actualSize);

	output_file.write((char*)compr_data, actualSize);
	output_file.close();
	cout << "compress ok" << endl;
	return 0;
}


