#pragma once

class psb_cc_base;

#include "psb_cc_base.h"
#include "psb_cc_null.h"
#include "psb_cc_array.h"
#include "psb_cc_integer.h"
#include "psb_cc_string.h"
#include "psb_cc_decimal.h"
#include "psb_cc_resource.h"
#include "psb_cc_collection.h"
#include "psb_cc_object.h"
#include "psb_cc_boolean.h"
#include "psb_cc_btree.h"
#include "psb_link_resource.h"


class psb_link
{
public:
	psb_link();
	~psb_link();

	bool link_string_table();
	bool link_names_table();
	bool link_resource_table(Json::Value& resource_code);
	bool link(psb_cc &cc, Json::Value& resource_code);

protected:
	psb_cc_btree *names;

	psb_cc* _compiler;
	psb_cc_array *_strings_cc;
	cc_stream _string_data;
	psb_link_resource* _resource;

	unsigned char *_data;
	uint32_t _length;

public:
	unsigned char* data() const
	{
		return _data;
	}

	uint32_t length() const
	{
		return _length;
	}


};

