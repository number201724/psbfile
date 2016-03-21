#pragma once

/*
KRKR PSBFILE Compiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include "def.h"
#include "psb_cc_base.h"
#include "cc_stream.h"

class psb_cc;

class psb_cc_base
{
public:
	psb_cc_base(psb_value_t::type_t type);
	psb_cc_base(psb_cc* cc, psb_value_t::type_t type);
	psb_cc_base(psb_cc* cc, Json::Value &src, psb_value_t::type_t type);
	virtual ~psb_cc_base();

	unsigned char* get_data();
	uint32_t get_length();
	psb_value_t::type_t get_type();
	Json::Value get_source();
	void reset_data();
	virtual bool compile() = 0;
	virtual const char *get_class_name() = 0;
	virtual void dump();
protected:
	psb_cc *_cc;
	Json::Value _src;

	psb_value_t::type_t _type;
	unsigned char *_data;
	uint32_t _length;
};

