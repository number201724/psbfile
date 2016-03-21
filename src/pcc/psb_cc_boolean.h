#pragma once
#include "psb_cc_base.h"
class psb_cc_boolean : public psb_cc_base
{
public:
	psb_cc_boolean(bool value);
	~psb_cc_boolean();

	virtual bool compile();
	virtual const char* get_class_name();

protected:
	bool _value;
};

