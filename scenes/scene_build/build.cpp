/*
	M2 PSB Editor
	Author:201724
	email:number201724@me.com
	qq:495159
*/
#include "../../common/psb.hpp"
#include "../../common/psb_stream.hpp"
#include <direct.h>

struct scene_text_pack
{
	unsigned long index;
	string texts;
};

string script_filename;
string merge_filename;
vector<scene_text_pack> scene_texts;
vector<unsigned long> strings;


void
parse_commands(int argc,
	char *argv[])
{
	if (argc <= 1 || argc != 3) {
		printf("usage:\n");
		printf("\tscene_build <psbfile> <merge.txt>\n");
		exit(0);
	}

	script_filename = argv[1];
	merge_filename = argv[2];
}

bool
get_file_buffers(unsigned char*& buff,
	size_t& size) {
	FILE* fp;


	fp = fopen(script_filename.c_str(), "rb");
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
	string filename;
	char dir[256];
	FILE* fp;
	_getcwd(dir, sizeof(dir));
	strcat(dir, "/NewScn/");
	_mkdir(dir);
	if (script_filename.find("\\") == string::npos || script_filename.find("/") == string::npos) {
		filename = dir + script_filename;
	}
	else {
		const char* pos = strrchr(script_filename.c_str(), '\\');
		if (pos) pos++;
		if (pos) {
			filename = dir + script_filename;
		}
		else {
			return false;
		}
	}
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		return false;
	}

	fwrite(psb_stream.get_buffer(), 1, psb_stream.get_length(), fp);
	fclose(fp);

	return true;
}

void load_str_index(psb_t& psb) {
	for (unsigned long i = 0; i < psb.strings->size(); i++) {
		strings.push_back(psb.strings->get(i));
	}
	printf("load strings index count:%u\n", strings.size());
}

void load_merge_files() {
	FILE* fp;
	unsigned long index;
	char* text;
	int text_len;
	char line[4096];
	char solid[] = "\xE2\x97\x8F\x00";		//● => UTF-8
	char hollow[] = "\xE2\x97\x8B\x00";		//○ => UTF-8

	fp = fopen(merge_filename.c_str(), "rb");

	if (fp == NULL) {
		printf("open merge file failed\n");
		exit(1);
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, hollow, 3) == 0 && strncmp(&line[11], hollow, 3) == 0) {

			sscanf(&line[3], "%08u", &index);

			//ignore ●00000000● partten
			text = &line[14];
			text_len = strlen(text);


			if (text_len > 0 && text[text_len - 1] == '\n') {
				text[text_len - 1] = 0;
			}

			scene_text_pack packs;
			packs.index = index;
			packs.texts = text;
			scene_texts.push_back(packs);
		}
	}

	fclose(fp);

	printf("load merge text count:%d\n", scene_texts.size());
}


unsigned long
fixed_write_string(psb_fixed_build_t& psb_build, unsigned long& pos, string new_str) {
	unsigned long result;
	const char* buf = new_str.c_str();
	unsigned long buf_len = new_str.length() + 1;

	psb_build.insert((const unsigned char*)buf, buf_len, pos);

	result = pos - psb_build.hdr->offset_strings_data;
	pos += buf_len;

	return result;
}

void
fixed_script_scenes(psb_t& psb, psb_fixed_build_t& psb_build) {

	unsigned long section_size = 0;
	unsigned long pos;

	//calc strings_data section size
	for (unsigned long i = 0; i < strings.size(); i++) {
		if (strings[i] > section_size) {
			section_size = strings[i];
		}
	}

	string s = (psb.strings_data + section_size);
	section_size += s.length() + 1;

	pos = psb.hdr->offset_strings_data + section_size;

	printf("fixed strings offset and add strings\n");
	for (unsigned long i = 0; i < scene_texts.size(); i++) {
		unsigned long index = scene_texts[i].index;

		if (index > strings.size()) {
			printf("to long index:%u\n", scene_texts[i].index);
			continue;
		}

		string texts = (psb.strings_data + strings[index]);

		if (texts == scene_texts[i].texts) {
			continue;
		}

		printf("%s\n", scene_texts[i].texts.c_str());
		strings[index] = fixed_write_string(psb_build, pos, scene_texts[i].texts);
		printf("%d\n", strings[index]);
	}

	printf("ok...\n");

	printf("build string index\n");
	psb_array_build_t array_table(strings);
	printf("replace string index\n");
	psb_build.replace(psb_build.hdr->offset_strings, psb.strings->data_length, array_table.get_length(), array_table.get_buffer());

	printf("string index process ok\n");
}

int main(int argc,
	char *argv[])
{
	unsigned char* buff;
	size_t size;
	string text_filename;

	parse_commands(argc, argv);

	printf("open psbfile....\n");
	if (!get_file_buffers(buff, size)) {
		printf("open script file failed\n");
		return 1;
	}

	printf("ok!\n");

	if (strncmp((const char *)buff, "PSB", 3) != 0) {
		printf("invalid psb format\n");
		return 1;
	}
	printf("file is psb header!\n");

	printf("load psb buffer...\n");
	psb_t psb(buff);

	printf("ok...\n");

	printf("load merge script...\n");
	load_merge_files();
	printf("ok...\n");

	printf("load psb string index...\n");
	load_str_index(psb);
	printf("ok...\n");

	psb_fixed_build_t psb_build(buff, size);
	printf("fixed script scenes...\n");
	fixed_script_scenes(psb, psb_build);
	printf("done...\n");
	if (set_file_buffer(psb_build)) {
		printf("save psb file success\n");
	}
	else {
		printf("save psb file failed\n");
	}

	return 0;
}