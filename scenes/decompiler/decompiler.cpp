#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include "../../common/psb.hpp"
using namespace std;

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

bool save_bmp(string filename, int width, int height, void *data)
{
	int nAlignWidth = width * 4;
	BITMAP_FILE_HDR Header;
	BITMAP_INFO_HDR HeaderInfo;
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = (sizeof(BITMAP_FILE_HDR) + sizeof(BITMAP_INFO_HDR));
	Header.bfSize = (sizeof(BITMAP_FILE_HDR) + sizeof(BITMAP_INFO_HDR) + nAlignWidth* height * 4);
	HeaderInfo.biSize = sizeof(BITMAP_INFO_HDR);
	HeaderInfo.biWidth = width;
	HeaderInfo.biHeight = height;
	HeaderInfo.biPlanes = 1;
	HeaderInfo.biBitCount = BMP_BIT;
	HeaderInfo.biCompression = 0;
	HeaderInfo.biSizeImage = nAlignWidth * height;
	HeaderInfo.biXPelsPerMeter = 0;
	HeaderInfo.biYPelsPerMeter = 0;
	HeaderInfo.biClrUsed = 0;
	HeaderInfo.biClrImportant = 0;
	fstream output(filename + ".bmp", ios::binary | ios::out);

	output.write((const char *)&Header, sizeof(Header));
	output.write((const char *)&HeaderInfo, sizeof(HeaderInfo));
	output.write((const char *)data, HeaderInfo.biSizeImage);
	output.flush();
	output.close();
	return true;
}

struct file_object
{
	string name;
	psb_resource_t *resource;
};

#define PSB_LZSS_LOOKSHIFT		7
#define PSB_LZSS_LOOKAHEAD		( 1 << PSB_LZSS_LOOKSHIFT )
uint32_t lzss_uncompress(const unsigned char* pInput, unsigned char* pOutput, uint32_t actualSize, uint32_t align)
{
	uint32_t totalBytes = 0;
	int cmdByte = 0;


	for (; actualSize != totalBytes; )
	{
		cmdByte = *pInput++; totalBytes++;

		if (!(cmdByte & PSB_LZSS_LOOKAHEAD))
		{
			int count = (cmdByte + 1) * align;

			for (int i = 0; i < count; i++) {
				*pOutput++ = *pInput++;
			}

			totalBytes += count;
		}
		else {
			int count = (cmdByte & (~PSB_LZSS_LOOKAHEAD)) + 3;

			for (uint32_t i = 0; i < align; i++) {
				pOutput[i] = *pInput++;
				totalBytes++;
			}

			unsigned char *max = &pOutput[align * (count - 1)];
			for (unsigned char * p = pOutput; p != max; p++)
				p[align] = p[0];

			pOutput += align * count;
		}
	}

	return totalBytes;
}

void convert_pixel(unsigned char *data, int width, int height, int align)
{
	int widthSize = width * align; //对齐宽度大小
	unsigned char *temp = new unsigned char[widthSize];

	for (int i = 0; i < height; i++) //用高度*(宽度*对齐)寻址
	{
		unsigned char *dst = &data[((height - i) - 1) * widthSize];
		unsigned char *src = &data[i * widthSize];
		memcpy(temp, dst, widthSize);
		memcpy(dst, src, widthSize);
		memcpy(src, temp, widthSize);
	}

	delete[] temp;
}

void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects, string object_name, Json::Value &root);

void
traversal_offsets_tree(psb_t& psb,
	const psb_collection_t *offsets, string entry_name, Json::Value &root) {
	psb_value_t *value = NULL;

	for (uint32_t i = 0; i < offsets->size(); i++) {
		unsigned char* entry_buff = offsets->get(i);
		psb.unpack(value, entry_buff);


		if (value != NULL) {
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_COLLECTION) {

				Json::Value node(Json::ValueType::arrayValue);
				traversal_offsets_tree(psb, (const psb_collection_t *)value, entry_name, node);
				root.append(node);

			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {

				Json::Value node(Json::ValueType::objectValue);
				traversal_object_tree(psb, (const psb_objects_t *)value, entry_name, node);
				root.append(node);

			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_STRING) {

				Json::Value node(Json::ValueType::stringValue);
				psb_string_t *psb_string = (psb_string_t*)value;
				node = psb_string->get_string();
				root.append(node);

			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_N0 || value->get_type() == psb_value_t::TYPE_N1 ||
				value->get_type() == psb_value_t::TYPE_N2 || value->get_type() == psb_value_t::TYPE_N3 ||
				value->get_type() == psb_value_t::TYPE_N4) {

				Json::Value node(Json::ValueType::uintValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root.append(node);
			}


			if (value->get_type() == psb_value_t::TYPE_FLOAT0 || value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_DOUBLE) {

				Json::Value node(Json::ValueType::realValue);

				psb_number_t *number = (psb_number_t*)value;
				if (value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_FLOAT0) {
					node = number->get_float();
				}
				if (value->get_type() == psb_value_t::TYPE_DOUBLE) {
					node = number->get_double();
				}

				root.append(node);
			}

			if (value->get_type() == psb_value_t::TYPE_N5 || value->get_type() == psb_value_t::TYPE_N6 ||
				value->get_type() == psb_value_t::TYPE_N7 || value->get_type() == psb_value_t::TYPE_N8) {

				Json::Value node(Json::ValueType::uintValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer64();
				root.append(node);
			}

			if (value->get_type() == psb_value_t::TYPE_NULL) {
				Json::Value node(Json::ValueType::nullValue);
				root.append(node);
			}
			if (value->get_type() == psb_value_t::TYPE_RESOURCE) {
				Json::Value node(Json::ValueType::stringValue);
				node = "#META_DATA";
				root.append(node);
			}
		}

		else {
			printf("invalid_type:%s,%02X\n", entry_name.c_str(), entry_buff[0]);
		}


	}
}
void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects, string object_name, Json::Value &root) {
	psb_value_t *value = NULL;
	vector<file_object> files;
	for (uint32_t i = 0; i < objects->size(); i++) {
		string entry_name = objects->get_name(i);
		unsigned char* entry_buff = objects->get_data(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_COLLECTION) {

				Json::Value node(Json::ValueType::arrayValue);
				traversal_offsets_tree(psb, (const psb_collection_t *)value, entry_name, node);
				root[entry_name] = node;
			}
			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {

				Json::Value node(Json::ValueType::objectValue);
				traversal_object_tree(psb, (const psb_objects_t *)value, entry_name, node);
				root[entry_name] = node;
			}
			//=======================================================

			if (value->get_type() == psb_value_t::TYPE_STRING) {

				Json::Value node(Json::ValueType::stringValue);
				psb_string_t *psb_string = (psb_string_t*)value;
				node = psb_string->get_string();
				root[entry_name] = node;
			}
			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_N0 || value->get_type() == psb_value_t::TYPE_N1 ||
				value->get_type() == psb_value_t::TYPE_N2 || value->get_type() == psb_value_t::TYPE_N3 ||
				value->get_type() == psb_value_t::TYPE_N4) {

				Json::Value node(Json::ValueType::uintValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer();
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_FLOAT0 || value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_DOUBLE) {

				Json::Value node(Json::ValueType::realValue);

				psb_number_t *number = (psb_number_t*)value;
				if (value->get_type() == psb_value_t::TYPE_FLOAT || value->get_type() == psb_value_t::TYPE_FLOAT0) {
					node = number->get_float();
				}
				if (value->get_type() == psb_value_t::TYPE_DOUBLE) {
					node = number->get_double();
				}

				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_N5 || value->get_type() == psb_value_t::TYPE_N6 ||
				value->get_type() == psb_value_t::TYPE_N7 || value->get_type() == psb_value_t::TYPE_N8) {

				Json::Value node(Json::ValueType::uintValue);
				psb_number_t *number = (psb_number_t*)value;
				node = number->get_integer64();
				root[entry_name] = node;
			}

			//=======================================================
			if (value->get_type() == psb_value_t::TYPE_NULL) {
				Json::Value node(Json::ValueType::nullValue);
				root[entry_name] = node;
			}

			if (value->get_type() == psb_value_t::TYPE_RESOURCE) {
				psb_resource_t *resource = (psb_resource_t *)value;

				file_object file;
				file.name = object_name;
				file.resource = resource;
				files.push_back(file);

				Json::Value node(Json::ValueType::stringValue);
				node = "#META_DATA";
				root[entry_name] = node;
			}
		}
		else {
			printf("invalid_type:%s:%02X\n", entry_name.c_str(), entry_buff[0]);
		}
	}

	for (size_t i = 0; i < files.size(); i++)
	{
		int height = root["height"].asInt();
		int width = root["width"].asInt();

		int align = root.isMember("pal") ? 1 : 4;
		int image_size = height *  width * align;
		unsigned char *img = new unsigned char[image_size];

		lzss_uncompress(files[i].resource->get_buff(), img, files[i].resource->get_length(), align);
		convert_pixel(img, width, height, align);
		save_bmp(files[i].name, width, height, img);


		delete[] img;
	}

}

int main(int argc, char* argv[])
{
	Json::Value root;
	std::streampos pos;
	char* data;
	fstream reader("title.psb", ios::binary | ios::in);
	fstream writer("title.json", ios::out);

	reader.seekg(0, ios::end);
	pos = reader.tellp();
	reader.seekg(0, ios::beg);
	data = new char[(unsigned int)pos.seekpos()];

	reader.read(data, pos.seekpos());

	psb_t psb((unsigned char*)data);
	const psb_objects_t *objects = psb.get_objects();

	traversal_object_tree(psb, objects, "", root);

	writer << root.toStyledString() << endl;

	return 0;
}