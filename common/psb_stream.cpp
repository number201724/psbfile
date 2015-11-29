#include "psb_stream.hpp"


psb_stream_t::
psb_stream_t() {
	buff = (unsigned char*)malloc(0);
	size = 0;
}

psb_stream_t::
psb_stream_t(const unsigned char* in_buff, unsigned long in_size) {
	size = in_size;
	buff = (unsigned char *)malloc(in_size);
	memcpy(buff, in_buff, in_size);
}
psb_stream_t::
~psb_stream_t() {
	free(buff);
}

unsigned long
psb_stream_t::
write(const unsigned char* data, unsigned long length) {
	unsigned long pos = size;
	buff = (unsigned char*)realloc(buff, length + size);
	memcpy(&buff[size], data, length);
	size += length;
	return pos;
}

long
psb_stream_t::
replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data) {
	if (dst_len == src_len) {
		memcpy(&buff[pos], data, dst_len);
	}
	else if (src_len < dst_len) {
		unsigned long diff = dst_len - src_len;
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
resize(unsigned long n) {
	buff = (unsigned char*)realloc(buff, n);
	size = n;
}
void
psb_stream_t::
insert(const unsigned char* data, unsigned long length, unsigned long pos) {
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

unsigned long
psb_stream_t::
get_length() {
	return size;
}

psb_array_build_t::
psb_array_build_t(vector<unsigned long> values) :
	psb_stream_t()
{
	entry_length = 0;

	for (unsigned int i = 0; i < values.size(); i++) {
		if (int_size(values[i]) > entry_length) {
			entry_length = int_size(values[i]);
		}
	}

	unsigned char n = int_size(values.size());
	write_byte(n);
	write_int(values.size(), n);
	write_byte(entry_length);

	for (unsigned int i = 0; i < values.size(); i++) {
		write_int(values[i], entry_length);
	}
}

unsigned char
psb_array_build_t::
int_size(unsigned long value) {
	if (value <= 0xFF) {
		return 1;
	}
	if (value <= 0xFFFF) {
		return 2;
	}
	if (value <= 0xFFFFFF) {
		return 3;
	}
	return 4;
}

void
psb_array_build_t::
write_byte(unsigned char value) {
	value += 0xC;

	write(&value, sizeof(value));
}

void
psb_array_build_t::
write_int(unsigned long value, unsigned char n) {
	unsigned char* p = (unsigned char *)&value;

	if (n == 0) {
		n = int_size(value);
	}

	write(p, n);
}
psb_fixed_build_t::
psb_fixed_build_t(const unsigned char* in_buff, unsigned long in_size) :
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
correct_hdr(unsigned long pos, long diff) {
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
insert(const unsigned char* data, unsigned long length, unsigned long pos) {
	psb_stream_t::insert(data, length, pos);
	correct_hdr(pos, length);
}
long
psb_fixed_build_t::
replace(unsigned long pos, unsigned long src_len, unsigned long dst_len, const unsigned char* data) {
	long diff = psb_stream_t::replace(pos, src_len, dst_len, data);

	correct_hdr(pos, diff);
	return diff;
}