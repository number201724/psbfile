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
Json::Value root_source;
Json::Reader parser_source;
Json::Value root_resource;
Json::Reader parser_resource;
string top_directory;

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

bool is_rl_image_entries(Json::Value &j_object)
{
	return j_object.isMember("pixel") && j_object.isMember("compress");
}

bool load_chunk_data(string res_identifier,unsigned char*& data, uint32_t &length)
{
	if (res_identifier.find("#resource#") == -1) {
		return false;
	}

	uint32_t resource_index = 0;
	sscanf_s(res_identifier.c_str(), "#resource#%u", &resource_index);

	string file_name = root_resource[resource_index].asString();

	fstream resource_data(file_name, ios::in | ios::binary);
	if (!resource_data.is_open()) { return false; }

	resource_data.seekg(0, ios::end);
	length = (uint32_t)resource_data.tellg();
	resource_data.seekg(0, ios::beg);
	data = new unsigned char[length];
	resource_data.read((char*)data, length);
	resource_data.close();

	return true;
}

void split(std::string& s, std::string delim, std::vector< std::string >& ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret.push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last > 0)
	{
		ret.push_back(s.substr(last, index - last));
	}
}

void make_directorys(string image_name)
{
	vector<string> dir_tree;
	split(image_name, "/", dir_tree);

	string now_dir = top_directory;

	for (uint32_t i = 0; i < dir_tree.size();i++)
	{
		now_dir += "/" + dir_tree[i];
		_mkdir(now_dir.c_str());
	}

}
void extra_image_data(Json::Value &image_entries, string image_dir,string image_name)
{
	unsigned char *data;
	uint32_t length;
	unsigned char *uncompr_data;
	uint32_t uncompr_length;

	if(image_entries["compress"] != "RL") {
		return;
	}

	int height = image_entries["height"].asInt();
	int width = image_entries["width"].asInt();
	int align = 4;
	if(image_entries.isMember("pal")){
		align = image_entries["pal"].asInt();
	}

	if(!load_chunk_data(image_entries["pixel"].asString(), data, length))
	{
		cout << "can not load chunk data" << endl;
		return;
	}


	uncompr_length = height * width * align;
	uncompr_data = new unsigned char[uncompr_length];

	psb_pixel_uncompress(data, uncompr_data, length, align);
	convert_pixel(uncompr_data, width, height, align);

	make_directorys(image_dir);
	save_bmp(top_directory + "/" + image_name, width, height, uncompr_data);

	delete[] uncompr_data;
	delete[] data;
}
void enum_files(string parent, Json::Value &node)
{
	std::vector<std::string> members = node.getMemberNames();
	
	for (uint32_t i = 0; i < members.size(); i++)
	{
		string name = members[i];
		Json::Value &nexts = node[name];
		if (nexts.type() != Json::objectValue) {
			continue;
		}

		if (is_rl_image_entries(nexts))
		{
			string image_file_name = parent + name;
			extra_image_data(nexts, parent, image_file_name);
			continue;
		}

		enum_files(parent + name + "/", nexts);
	}
}

int main(int argc,char* argv[])
{
	if (argc < 3)
	{
		cout << "usage: rl_image <json_source> <json_resource> <output_dir>" << endl;
		return 0;
	}

	fstream json_source(argv[1], ios::in);
	fstream json_resource(argv[2], ios::in);
	top_directory = argv[3];
	_mkdir(argv[3]);

	if (!json_source.is_open()) {
		cout << "open json_source file failed" << endl;
		return 1;
	}
	if (!json_resource.is_open()) {
		cout << "open json_resource file failed" << endl;
		return 1;
	}

	if(!parser_source.parse(json_source, root_source))
	{
		cout << "parse json_source failed" << endl;
		return 1;
	}

	if(!parser_resource.parse(json_resource, root_resource))
	{
		cout << "parse json_resource failed" << endl;
		return 1;
	}

	if(!root_source.isMember("source"))
	{
		cout << "this file not rl_image file" << endl;
		return 1;
	}

	cout << "uncompress rl image...." << endl;
	Json::Value &source_node = root_source["source"];
	enum_files("", source_node);

	cout << "uncompress rl image ok" << endl;
	return 0;
}


