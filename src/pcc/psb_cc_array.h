#pragma once
#include "psb_cc_base.h"

class psb_cc_array : public psb_cc_base, public vector<int64_t>
{
public:
	psb_cc_array();
	virtual ~psb_cc_array();
	virtual bool compile();
	void set_value(uint32_t index, int64_t value);
	const char *get_class_name();
protected:
	uint8_t get_data_align_bytes();
	void cc_create_hdr(cc_stream& s,uint8_t bytes);
};

