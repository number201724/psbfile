#ifndef _PSB_STREAM_H_INCLUDED_
#define _PSB_STREAM_H_INCLUDED_

#include <stdio.h>
#include <stdint.h>
#include "psb.hpp"

class psb_stream_t {
public:
	psb_stream_t();
	psb_stream_t(const unsigned char* in_buff, uint32_t in_size);
	virtual ~psb_stream_t();

	virtual void insert(const unsigned char* data, uint32_t length, uint32_t pos);
	virtual long replace(uint32_t pos, uint32_t src_len, uint32_t dst_len, const unsigned char* data);
	void resize(uint32_t n);
	uint32_t write(const unsigned char* data, uint32_t length);
	virtual unsigned char* get_buffer();
	virtual uint32_t get_length();
protected:
	unsigned char* buff;
	uint32_t size;
};

class psb_array_build_t : public psb_stream_t {
public:
	psb_array_build_t(vector<uint32_t> values);

	unsigned char int_size(uint32_t value);
	void write_byte(unsigned char value);
	void write_int(uint32_t value, unsigned char size = 0);

	unsigned char entry_length;

	virtual unsigned char* get_buffer();
	virtual uint32_t get_length();
	void set_align(unsigned char n);

protected:
	vector<uint32_t> arrays;
	unsigned char align;
};

class psb_fixed_build_t : public psb_stream_t {
public:
	psb_fixed_build_t(const unsigned char* in_buff, uint32_t in_size);
	psb_fixed_build_t(psb_stream_t& psb_stream);

	void update_hdr();
	void correct_hdr(uint32_t pos, long diff);
	virtual void insert(const unsigned char* data, uint32_t length, uint32_t pos);
	virtual long replace(uint32_t pos, uint32_t src_len, uint32_t dst_len, const unsigned char* data);


	PSBHDR *hdr;
};


#endif