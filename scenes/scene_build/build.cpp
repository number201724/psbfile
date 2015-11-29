/*
	M2 PSB Editor
	Author:201724
	email:number201724@me.com
	qq:495159
*/
#include "../../common/psb.hpp"
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

class psb_stream_t {
public:
	psb_stream_t();
	psb_stream_t(const unsigned char* in_buff, unsigned long in_size);
	virtual ~psb_stream_t();

	virtual void insert(const unsigned char* data, unsigned long length, unsigned long pos);
	virtual long replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data);
	void resize(unsigned long n);
	unsigned long write(const unsigned char* data, unsigned long length);
	unsigned char* get_buffer();
	unsigned long get_length();
protected:
	unsigned char* buff;
	unsigned long size;
};

class psb_array_build_t : public psb_stream_t {
public:
	psb_array_build_t(vector<unsigned long> values);

	unsigned char int_size(unsigned long value);
	void write_byte(unsigned char value);
	void write_int(unsigned long value, unsigned char size = 0);

	unsigned char entry_length;
};

class psb_fixed_build_t : public psb_stream_t {
public:
	psb_fixed_build_t(const unsigned char* in_buff, unsigned long in_size);
	psb_fixed_build_t(psb_stream_t& psb_stream);

	void update_hdr();
	void correct_hdr(unsigned long pos, long diff);
	virtual void insert(const unsigned char* data, unsigned long length, unsigned long pos);
	virtual long replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data);


	PSBHDR *hdr;
};
psb_stream_t::
psb_stream_t() {
	buff = (unsigned char*)malloc(0);
	size = 0;
}

psb_stream_t::
psb_stream_t(const unsigned char* in_buff, unsigned long in_size) {
	size = in_size;
	buff = (unsigned char *)malloc(in_size);
	memcpy(buff, in_buff, in_size);
}
psb_stream_t::
~psb_stream_t() {
	free(buff);
}

unsigned long
psb_stream_t::
write(const unsigned char* data, unsigned long length) {
	unsigned long pos = size;
	buff = (unsigned char*)realloc(buff, length + size);
	memcpy(&buff[size], data, length);
	size += length;
	return pos;
}

long
psb_stream_t::
replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data) {
	if (dst_len == src_len) {
		memcpy(&buff[pos], data, dst_len);
	}
	else if (src_len < dst_len) {
		unsigned long diff = dst_len - src_len;
		insert(data, diff, pos);
		memcpy(&buff[pos], data, dst_len);
		return diff;
	}
	else if (src_len > dst_len) {
		psb_stream_t psb_stream;
		long diff = dst_len - src_len;
		psb_stream.write(buff, pos);
		psb_stream.write(data, dst_len);
		psb_stream.write(&buff[pos + src_len], size - (pos + src_len));

		resize(psb_stream.get_length());

		memcpy(buff, psb_stream.get_buffer(), psb_stream.get_length());
		return diff;
	}
	return 0;
}
void
psb_stream_t::
resize(unsigned long n) {
	buff = (unsigned char*)realloc(buff, n);
	size = n;
}
void
psb_stream_t::
insert(const unsigned char* data, unsigned long length, unsigned long pos) {
	buff = (unsigned char*)realloc(buff, length + size);
	memmove(&buff[pos + length], &buff[pos], size - pos);
	memcpy(&buff[pos], data, length);

	size += length;
}

unsigned char*
psb_stream_t::
get_buffer() {
	return buff;
}

unsigned long
psb_stream_t::
get_length() {
	return size;
}

psb_array_build_t::
psb_array_build_t(vector<unsigned long> values) :
	psb_stream_t()
{
	entry_length = 0;

	for (unsigned int i = 0; i < values.size(); i++) {
		if (int_size(values[i]) > entry_length) {
			entry_length = int_size(values[i]);
		}
	}

	unsigned char n = int_size(values.size());
	write_byte(n);
	write_int(values.size(), n);
	write_byte(entry_length);

	for (unsigned int i = 0; i < values.size(); i++) {
		write_int(values[i], entry_length);
	}
}

unsigned char
psb_array_build_t::
int_size(unsigned long value) {
	if (value <= 0xFF) {
		return 1;
	}
	if (value <= 0xFFFF) {
		return 2;
	}
	if (value <= 0xFFFFFF) {
		return 3;
	}
	return 4;
}

void
psb_array_build_t::
write_byte(unsigned char value) {
	value += 0xC;

	write(&value, sizeof(value));
}

void
psb_array_build_t::
write_int(unsigned long value, unsigned char n) {
	unsigned char* p = (unsigned char *)&value;

	if (n == 0) {
		n = int_size(value);
	}

	write(p, n);
}
psb_fixed_build_t::
psb_fixed_build_t(const unsigned char* in_buff, unsigned long in_size) :
	psb_stream_t(in_buff, in_size) {
	update_hdr();
}
psb_fixed_build_t::
psb_fixed_build_t(psb_stream_t& psb_stream) :
	psb_stream_t(psb_stream.get_buffer(), psb_stream.get_length()) {
	update_hdr();
}

void
psb_fixed_build_t::
update_hdr() {
	hdr = (PSBHDR *)get_buffer();
}

void
psb_fixed_build_t::
correct_hdr(unsigned long pos, long diff) {
	update_hdr();

	if (hdr->unknown1 >= pos)
		hdr->unknown1 += diff;
	if (hdr->offset_names >= pos)
		hdr->offset_names += diff;
	if (hdr->offset_strings >= pos)
		hdr->offset_strings += diff;
	if (hdr->offset_strings_data >= pos)
		hdr->offset_strings_data += diff;
	if (hdr->offset_chunk_offsets >= pos)
		hdr->offset_chunk_offsets += diff;
	if (hdr->offset_chunk_lengths >= pos)
		hdr->offset_chunk_lengths += diff;
	if (hdr->offset_chunk_data >= pos)
		hdr->offset_chunk_data += diff;
	if (hdr->offset_entries >= pos)
		hdr->offset_entries += diff;
}
void
psb_fixed_build_t::
insert(const unsigned char* data, unsigned long length, unsigned long pos) {
	psb_stream_t::insert(data, length, pos);
	correct_hdr(pos, length);
}
long
psb_fixed_build_t::
replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data){
	long diff = psb_stream_t::replace(pos, src_len, dst_len, data);

	correct_hdr(pos, diff);
	return diff;
}
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
	char solid[] = "\xE2\x97\x8F\x00";		//¡ñ => UTF-8
	char hollow[] = "\xE2\x97\x8B\x00";		//¡ð => UTF-8

	fp = fopen(merge_filename.c_str(), "rb");

	if (fp == NULL) {
		printf("open merge file failed\n");
		exit(1);
	}

	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, hollow, 3) == 0 && strncmp(&line[11], hollow, 3) == 0) {

			sscanf(&line[3], "%08u", &index);

			//ignore ¡ñ00000000¡ñ partten
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