// script_rebuild.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

class psb_node
{
public:
	enum node_type
	{
		TYPE_OFFSET,
		TYPE_OBJECT,
		TYPE_STRING
	};

	psb_node(node_type type) {
		_type = type;
	}
	node_type get_type() {
		return _type;
	}
private:
	node_type _type;
};
class psb_offset : public psb_node
{
public:
	psb_offset(psb_offsets_t *in) : psb_node(psb_node::TYPE_OFFSET)
	{
		buff = in->buff;
		for (unsigned long i = 0; i < in->offsets->size(); i++) {
			offsets.push_back(in->offsets->get(i));
		}
	}

	void set_index(unsigned long i)
	{
		index = i;
	}

public:
	int index;
	unsigned char *buff;
	vector<unsigned long> offsets;
	vector<psb_node*> entries;
};
class psb_object : public psb_node
{
public:
	psb_object(const psb_objects_t *objects) : psb_node(psb_node::TYPE_OBJECT) {
		buff = objects->buff;

		for (unsigned long i = 0; i < objects->names->size(); i++) {
			names.push_back(objects->names->get(i));
		}

		for (unsigned long i = 0; i < objects->offsets->size(); i++) {
			offsets.push_back(objects->offsets->get(i));
		}
	}
	void set_index(unsigned long i)
	{
		index = i;
	}
public:
	int index;
	unsigned char *buff;
	vector<unsigned long> names;
	vector<unsigned long> offsets;
	vector<psb_node*> entries;
};

class psb_string : public psb_node
{
public:
	psb_string(const psb_string_t *string) : psb_node(psb_node::TYPE_STRING) {
		index = string->get_index();
		str = string->get_string();
	}

public:
	unsigned long index;
	string str;
};

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
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer, psb_object *prev);

//It is actually a connecting structure, object and data node separately
void
traversal_offsets_tree(psb_t& psb,
	const psb_offsets_t *offsets,
	string layer_name,
	string entry_name, psb_offset *node) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < offsets->size(); i++) {
		unsigned char* entry_buff = offsets->get(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				psb_offset *entries = new psb_offset((psb_offsets_t*)value);
				entries->set_index(i);
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name, entries);
				node->entries.push_back(entries);
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				psb_object *entries = new psb_object((psb_objects_t*)value);
				entries->set_index(i);
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name, entries);
				node->entries.push_back(entries);
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				psb_string * entries = new psb_string((psb_string_t*)value);
				node->entries.push_back(entries);
				//parse_texts(layer_name, entry_name, (const psb_string_t *)value);
			}
		}
	}
}
void
traversal_object_tree(psb_t& psb,
	const psb_objects_t *objects,
	string prev_layer, psb_object *node) {
	psb_value_t *value = NULL;

	for (unsigned long i = 0; i < objects->size(); i++) {
		string entry_name = objects->get_name(i);
		string layer_name = format_layer(prev_layer, entry_name);
		unsigned char* entry_buff = objects->get_data(i);

		psb.unpack(value, entry_buff);

		if (value != NULL) {
			if (value->get_type() == psb_value_t::TYPE_OFFSETS) {
				psb_offset *entries = new psb_offset((psb_offsets_t*)value);
				entries->set_index(i);
				traversal_offsets_tree(psb, (const psb_offsets_t *)value, layer_name, entry_name, entries);
				node->entries.push_back(entries);
			}
			if (value->get_type() == psb_value_t::TYPE_OBJECTS) {
				psb_object *entries = new psb_object((psb_objects_t*)value);
				entries->set_index(i);
				traversal_object_tree(psb, (const psb_objects_t *)value, layer_name, entries);
				node->entries.push_back(entries);
			}
			if (value->get_type() == psb_value_t::TYPE_STRING) {
				psb_string * entries = new psb_string((psb_string_t*)value);
				node->entries.push_back(entries);

				//parse_texts(layer_name, entry_name, (const psb_string_t *)value);
			}
		}
	}
}

void compile(psb_object *object) {

}

void debug(psb_node *node, vector<string> stacks) {
	union {
		psb_string *str;
		psb_object *obj;
		psb_offset *off;
		psb_node *node;
	}a;

	a.node = node;
	
	if (node->get_type() == psb_node::TYPE_OBJECT) {
		stacks.push_back("objects.");
		for (unsigned long i = 0; i < a.obj->entries.size(); i++) {
			debug(a.obj->entries[i], stacks);
		}
	}
	if (node->get_type() == psb_node::TYPE_OFFSET) {
		stacks.push_back("offsets.");
		for (unsigned long i = 0; i < a.off->entries.size(); i++) {
			debug(a.off->entries[i], stacks);
		}
	}
	if (node->get_type() == psb_node::TYPE_STRING) {
		stacks.push_back("string");
		for (unsigned i = 0; i < stacks.size(); i++) {
			cout << stacks[i];
		}
		//
		
		cout << ":" <<a.str->str << endl;
	}
}

int main()
{
	fstream input("01_com_001_01.ks.scn", ios::in | ios::binary);

	input.seekg(0, ios::end);
	size_t i = input.tellg();
	input.seekg(0, ios::beg);

	unsigned char *data = new unsigned char[i];
	input.read((char *)data, i);

	psb_t psb(data);

	const psb_objects_t *objects = psb.get_objects();
	psb_object entries(objects);
	vector<string> stacks;


	traversal_object_tree(psb, objects, "entries", &entries);

	debug(&entries, stacks);
	compile(&entries);

	return 0;
}

