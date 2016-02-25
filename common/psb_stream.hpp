#ifndef _PSB_STREAM_H_INCLUDED_
#define _PSB_STREAM_H_INCLUDED_

#include "psb.hpp"

class psb_stream_t {
public:
	psb_stream_t();
	psb_stream_t(const unsigned char* in_buff, unsigned long in_size);
	virtual ~psb_stream_t();

	virtual void insert(const unsigned char* data, unsigned long length, unsigned long pos);
	virtual long replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data);
	void resize(unsigned long n);
	unsigned long write(const unsigned char* data, unsigned long length);
	virtual unsigned char* get_buffer();
	virtual unsigned long get_length();
protected:
	unsigned char* buff;
	unsigned long size;
};

class psb_array_build_t : public psb_stream_t {
public:
	psb_array_build_t(vector<unsigned long> values);

	unsigned char int_size(unsigned long value);
	void write_byte(unsigned char value);
	void write_int(unsigned long value, unsigned char size = 0);

	unsigned char entry_length;

	virtual unsigned char* get_buffer();
	virtual unsigned long get_length();
	void set_align(unsigned char n);

protected:
	vector<unsigned long> arrays;
	unsigned char align;
};

class psb_fixed_build_t : public psb_stream_t {
public:
	psb_fixed_build_t(const unsigned char* in_buff, unsigned long in_size);
	psb_fixed_build_t(psb_stream_t& psb_stream);

	void update_hdr();
	void correct_hdr(unsigned long pos, long diff);
	virtual void insert(const unsigned char* data, unsigned long length, unsigned long pos);
	virtual long replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data);


	PSBHDR *hdr;
};


#endif