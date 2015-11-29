#include "../../common/psb.hpp"
/*
M2 PSB Editor
Author:201724
email:number201724@me.com
qq:495159

original by asmodean['expimg']
*/

struct scene_text_pack
{
	unsigned long index;
	string texts;
};

string script_filename;
map<unsigned long, string> scene_texts_map;
vector<scene_text_pack> scene_texts;

void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer = "");

void
parse_commands(int argc,
	char *argv[])
{
	if (argc == 1) {
		printf("scene_parse <filepath>\n");
		exit(0);
	}

	script_filename = argv[1];
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

string
format_layer(string prev_layer,
	string entry_name) {
	string layer_name = prev_layer;
	if (layer_name.empty()) {
		layer_name = entry_name;
	}
	else {
		layer_name += "." + entry_name;
	}
	return layer_name;
}

void
parse_texts(string layer_name,
	string entry_name,
	const psb_string_t* str) {
	scene_text_pack packs;

	if (layer_name == "scenes.texts" || layer_name == "scenes.title" ||layer_name == "scenes.selects.text" || layer_name == "scenes.selectInfo.select.text") {
		
		packs.index = str->get_index();
		packs.texts = str->get_string();

		if (scene_texts_map.find(packs.index) == scene_texts_map.end()) {
			scene_texts_map[packs.index] = packs.texts;
			scene_texts.push_back(packs);
		}
	}
}

//It is actually a connecting structure, object and data node separately
void
traversal_offsets_tree(psb_t& psb,
	const psb_offsets_t *offsets,
	string layer_name,
	string entry_name) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < offsets->size(); i++) {
		unsigned char* entry_buff = offsets->get(i);
		
		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name);
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name);
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				parse_texts(layer_name, entry_name, (const psb_string_t *)value);
			}
		}
	}
}
void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < objects->size(); i++) {
		string entry_name = objects->get_name(i);
		string layer_name = format_layer(prev_layer, entry_name);
		unsigned char* entry_buff = objects->get_data(i);
		
		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name);
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name);
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				parse_texts(layer_name, entry_name, (const psb_string_t *)value);
			}
		}
	}
}

void
format_to_file(string scene_filename) {
	FILE* fp;
	char solid[] = "\xE2\x97\x8F\x00";		//● => UTF-8
	char hollow[] = "\xE2\x97\x8B\x00";		//○ => UTF-8

	fp = fopen(scene_filename.c_str(), "wb");
	if (!fp) {
		printf("create scene file failed\n");
		exit(1);
	}

	for (size_t i = 0; i < scene_texts.size(); i++) {
		fprintf(fp, "%s%08u%s%s\n", solid, scene_texts[i].index, solid, scene_texts[i].texts.c_str());
		fprintf(fp, "%s%08u%s%s\n", hollow, scene_texts[i].index, hollow, scene_texts[i].texts.c_str());
		fprintf(fp, "\n");
	}

	fflush(fp);
	fclose(fp);
}

int main(int argc,
	char *argv[])
{
	unsigned char* buff;
	size_t size;
	string text_filename;

	parse_commands(argc, argv);

	if (!get_file_buffers(buff, size)) {
		printf("open script file failed\n");
		return 1;
	}

	if (strncmp((const char *)buff, "PSB", 3) != 0) {
		printf("invalid psb format\n");
		exit(0);
	}

	psb_t psb(buff);
	const psb_objects_t* psb_objects = psb.get_objects();

	printf("parse script scenes and selects\n");
	traversal_object_tree(psb, psb_objects);
	printf("parse texts to file\n");
	format_to_file(script_filename + ".txt");
	printf("done\n");

	return 0;
}