/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include "psb_cc_null.h"



psb_cc_null::psb_cc_null() :
	psb_cc_base(psb_value_t::TYPE_NULL)
{

}


psb_cc_null::~psb_cc_null()
{

}

bool psb_cc_null::compile()
{
	reset_data();
	_data = new unsigned char[1];
	_length = 1;

	_data[0] = (unsigned char)psb_value_t::TYPE_NULL;
	dump();
	return true;
}
const char* psb_cc_null::get_class_name()
{
	return "psb_cc_null";
}