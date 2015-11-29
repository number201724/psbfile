#include "../../common/psb.hpp"
#include "../../common/psb_stream.hpp"
#include <direct.h>
#include "zconf.h"
#include "zlib.h"
/*
M2 PSB Editor
Author:201724
email:number201724@me.com
qq:495159

original by asmodean['expimg']
*/

struct MDFHDR {
	unsigned char signature[4];
	unsigned long size;
};

string filename;


void
parse_commands(int argc,
	char *argv[])
{
	if (argc == 1) {
		printf("scene_parse <filepath>\n");
		exit(0);
	}

	filename = argv[1];
}


bool
get_file_buffers(unsigned char*& buff,
	size_t& size) {
	FILE* fp;


	fp = fopen(filename.c_str(), "rb");
	if (fp == NULL) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buff = new unsigned char[size];
	fread(buff, 1, size, fp);

	fclose(fp);

	return true;
}

bool set_file_buffer(psb_stream_t& psb_stream) {

	FILE* fp;
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		return false;
	}

	fwrite(psb_stream.get_buffer(), 1, psb_stream.get_length(), fp);
	fclose(fp);

	return true;
}

bool process_package(unsigned char* buff, unsigned long length, psb_stream_t& stream, bool is_compress) {
	unsigned char* temp_buf;
	unsigned long temp_len;
	MDFHDR hdr;
	if (is_compress) {
		temp_len = compressBound(length);
		temp_buf = new unsigned char[temp_len];
		if (compress(temp_buf, &temp_len, buff, length) != Z_OK) {
			delete[] temp_buf;
			return false;
		}

		hdr.signature[0] = 'm'; hdr.signature[1] = 'd'; hdr.signature[2] = 'f'; hdr.signature[3] = '\x00';
		hdr.size = length;
		stream.write((unsigned char*)&hdr, sizeof(hdr));
		stream.write((unsigned char*)temp_buf, temp_len);
		delete[] temp_buf;
	}
	else {
		memcpy(&hdr, buff, sizeof(hdr));
		temp_buf = new unsigned char[hdr.size];
		temp_len = hdr.size;

		if (uncompress(temp_buf, &temp_len, &buff[sizeof(hdr)], length - sizeof(hdr)) != Z_OK) {
			delete[] temp_buf;
			return false;
		}

		stream.write(temp_buf, temp_len);

		delete[] temp_buf;
	}

	return true;
}

int main(int argc, char* argv[]) {
	unsigned char* buff;
	size_t size;
	string text_filename;
	psb_stream_t stream;

	parse_commands(argc, argv);

	if (!get_file_buffers(buff, size)) {
		printf("open script file failed\n");
		return 1;
	}

	if (strncmp((const char *)buff, "PSB", 3) == 0) {
		printf("execute compress\n");
		if (process_package(buff, size, stream, true)) {
			printf("compress success\n");
			if (!set_file_buffer(stream)) {
				printf("save file failed\n");
				return 1;
			}
			printf("save file ok\n");
		}
		else {
			printf("compress failed\n");
		}
		return 0;
	}

	if (strncmp((const char *)buff, "mdf", 3) == 0) {
		printf("execute uncompress\n");
		if (!process_package(buff, size, stream, false)) {
			printf("uncompress failed\n");
			return 1;
		}
		if (!set_file_buffer(stream)) {
			printf("save file failed\n");
			return 1;
		}
		printf("save file ok\n");
		return 0;
	}

	printf("invalid file format\n");
	return 1;
}