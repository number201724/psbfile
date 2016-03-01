#include "psb_stream.hpp"


psb_stream_t::
psb_stream_t() {
	buff = (unsigned char*)malloc(0);
	size = 0;
}

psb_stream_t::
psb_stream_t(const unsigned char* in_buff, uint32_t in_size) {
	size = in_size;
	buff = (unsigned char *)malloc(in_size);
	memcpy(buff, in_buff, in_size);
}
psb_stream_t::
~psb_stream_t() {
	free(buff);
}

uint32_t
psb_stream_t::
write(const unsigned char* data, uint32_t length) {
	uint32_t pos = size;
	buff = (unsigned char*)realloc(buff, length + size);
	memcpy(&buff[size], data, length);
	size += length;
	return pos;
}

long
psb_stream_t::
replace(uint32_t pos, uint32_t src_len, uint32_t dst_len, const unsigned char* data) {
	if (dst_len == src_len) {
		memcpy(&buff[pos], data, dst_len);
	}
	else if (src_len < dst_len) {
		uint32_t diff = dst_len - src_len;
		insert(data, diff, pos);
		memcpy(&buff[pos], data, dst_len);
		return diff;
	}
	else if (src_len > dst_len) {
		psb_stream_t psb_stream;
		long diff = dst_len - src_len;
		psb_stream.write(buff, pos);
		psb_stream.write(data, dst_len);
		psb_stream.write(&buff[pos + src_len], size - (pos + src_len));

		resize(psb_stream.get_length());

		memcpy(buff, psb_stream.get_buffer(), psb_stream.get_length());
		return diff;
	}
	return 0;
}
void
psb_stream_t::
resize(uint32_t n) {
	buff = (unsigned char*)realloc(buff, n);
	size = n;
}
void
psb_stream_t::
insert(const unsigned char* data, uint32_t length, uint32_t pos) {
	buff = (unsigned char*)realloc(buff, length + size);
	memmove(&buff[pos + length], &buff[pos], size - pos);
	memcpy(&buff[pos], data, length);

	size += length;
}

unsigned char*
psb_stream_t::
get_buffer() {
	return buff;
}

uint32_t
psb_stream_t::
get_length() {
	return size;
}

psb_array_build_t::
psb_array_build_t(vector<uint32_t> values) :
	psb_stream_t()
{
	entry_length = 0;

	for (unsigned int i = 0; i < values.size(); i++) {
		if (int_size(values[i]) > entry_length) {
			entry_length = int_size(values[i]);
		}
	}

	align = entry_length;
	arrays = values;
}

unsigned char*
psb_array_build_t::
get_buffer() {
	resize(0);

	unsigned char n = int_size(arrays.size());
	write_byte(n);
	write_int(arrays.size(), n);
	write_byte(align);

	for (unsigned int i = 0; i < arrays.size(); i++) {
		write_int(arrays[i], align);
	}

	return buff;
}
void
psb_array_build_t::
set_align(unsigned char n) {
	align = n;
}

uint32_t
psb_array_build_t::
get_length() {
	return size;
}

unsigned char
psb_array_build_t::
int_size(uint32_t value) {
	//bit mask
	if (value & 0xff000000) {
		return 4;
	}
	if (value & 0xff0000) {
		return 3;
	}
	if (value & 0xff00) {
		return 2;
	}
	return 1;
}

void
psb_array_build_t::
write_byte(unsigned char value) {
	value += 0xC;

	write(&value, sizeof(value));
}

void
psb_array_build_t::
write_int(uint32_t value, unsigned char n) {
	unsigned char* p = (unsigned char *)&value;

	if (n == 0) {
		n = int_size(value);
	}

	write(p, n);
}
psb_fixed_build_t::
psb_fixed_build_t(const unsigned char* in_buff, uint32_t in_size) :
	psb_stream_t(in_buff, in_size) {
	update_hdr();
}
psb_fixed_build_t::
psb_fixed_build_t(psb_stream_t& psb_stream) :
	psb_stream_t(psb_stream.get_buffer(), psb_stream.get_length()) {
	update_hdr();
}

void
psb_fixed_build_t::
update_hdr() {
	hdr = (PSBHDR *)get_buffer();
}

void
psb_fixed_build_t::
correct_hdr(uint32_t pos, long diff) {
	update_hdr();

	if (hdr->unknown1 >= pos)
		hdr->unknown1 += diff;
	if (hdr->offset_names >= pos)
		hdr->offset_names += diff;
	if (hdr->offset_strings >= pos)
		hdr->offset_strings += diff;
	if (hdr->offset_strings_data >= pos)
		hdr->offset_strings_data += diff;
	if (hdr->offset_chunk_offsets >= pos)
		hdr->offset_chunk_offsets += diff;
	if (hdr->offset_chunk_lengths >= pos)
		hdr->offset_chunk_lengths += diff;
	if (hdr->offset_chunk_data >= pos)
		hdr->offset_chunk_data += diff;
	if (hdr->offset_entries >= pos)
		hdr->offset_entries += diff;
}
void
psb_fixed_build_t::
insert(const unsigned char* data, uint32_t length, uint32_t pos) {
	psb_stream_t::insert(data, length, pos);
	correct_hdr(pos, length);
}
long
psb_fixed_build_t::
replace(uint32_t pos, uint32_t src_len, uint32_t dst_len, const unsigned char* data) {
	long diff = psb_stream_t::replace(pos, src_len, dst_len, data);

	correct_hdr(pos, diff);
	return diff;
}