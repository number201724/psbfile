/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/
#include "psb_cc_boolean.h"


psb_cc_boolean::psb_cc_boolean(bool value) :
	psb_cc_base(psb_value_t::TYPE_BOOLEAN), _value(value)
{
}


psb_cc_boolean::~psb_cc_boolean()
{
}

bool psb_cc_boolean::compile()
{
	cc_stream s;

	if (_value) {
		s.write_integer(psb_value_t::TYPE_TRUE);
	}
	else {
		s.write_integer(psb_value_t::TYPE_FALSE);
	}

	_data = s.copy_buffer();
	_length = s.get_length();

	dump();

	return true;
}

const char* psb_cc_boolean::get_class_name()
{
	return "psb_cc_boolean";
}