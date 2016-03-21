#pragma once

#include "psb_cc_base.h"

class psb_cc_integer : public psb_cc_base
{
public:
	psb_cc_integer(uint64_t number);
	psb_cc_integer();
	~psb_cc_integer();

	virtual bool compile();
	virtual const char *get_class_name();

	uint8_t cc_create_hdr(cc_stream& s);

	uint64_t get_number();
	void set_number(uint64_t number);
protected:

	uint64_t _number;
};

