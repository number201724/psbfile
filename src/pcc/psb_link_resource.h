#pragma once
#include "psb_cc_base.h"
#include "psb_cc_array.h"

struct psb_resource
{
	uint32_t id;
	unsigned char *data;
	uint32_t length;
};

class psb_link_resource
{
public:
	psb_link_resource();
	~psb_link_resource();


	bool load_resource(Json::Value& resource_code);
	bool load_file(uint32_t i,string filename);
	virtual bool compile();

	vector<psb_resource> resource_table;
	psb_cc_array chunk_offsets;
	psb_cc_array chunk_lengths;
	cc_stream chunk_data;
};

