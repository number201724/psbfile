#include "psb.hpp"
/*
M2 PSB Editor
Author:201724
email:number201724@me.com
qq:495159

original by asmodean['expimg']
*/

/***************************************************************************
* Implementation of lots of junk!
***************************************************************************/

/***************************************************************************
* psb_value_t
*/
psb_value_t::
psb_value_t(const psb_t&    psb,
	type_t          type,
	unsigned char*& p)
	: psb(psb),
	type(type)
{}

psb_value_t::
psb_value_t(const psb_t&    psb,
	unsigned char*& p)
	: psb(psb)
{}

psb_value_t::
~psb_value_t(void) {
}

psb_value_t::type_t
psb_value_t::
get_type(void) const {
	return type;
}

/***************************************************************************
* psb_number_t
*/
psb_number_t::
psb_number_t(const psb_t&    psb,
	unsigned char*& p) 
	: psb_value_t(psb, p) 
{
	type = (psb_value_t::type_t)*p++;
	number = 0;

	if (type == psb_value_t::TYPE_N1) {
		number = *p++;
	}
	if (type == psb_value_t::TYPE_N2) {
		for (unsigned long i = 0; i < 2; i++) number |= *p++ << (i * 8);
	}
	if (type == psb_value_t::TYPE_N3) {
		for (unsigned long i = 0; i < 3; i++) number |= *p++ << (i * 8);
	}
	if (type == psb_value_t::TYPE_N4) {
		for (unsigned long i = 0; i < 4; i++) number |= *p++ << (i * 8);
	}
}

unsigned long
psb_number_t::
get_number() const
{
	return number;
}

bool
psb_number_t::
is_number(psb_value_t *value)
{
	if (value->get_type() == psb_value_t::TYPE_N0 || value->get_type() == psb_value_t::TYPE_N1 ||
		value->get_type() == psb_value_t::TYPE_N2 || value->get_type() == psb_value_t::TYPE_N3 ||
		value->get_type() == psb_value_t::TYPE_N4) {
		return true;
	}
	return false;
}

/***************************************************************************
* psb_array_t
*/
psb_array_t::
psb_array_t(const psb_t&    psb,
	unsigned char*& p)
	: psb_value_t(psb, TYPE_ARRAY, p),
	data_length(0)
{

	unsigned long n = *p++ - 0xC;
	data_length += sizeof(unsigned char);
	unsigned long v = 0;

	for (unsigned long i = 0; i < n; i++) {
		v |= *p++ << (i * 8);
		data_length += sizeof(unsigned char);
	}

	entry_count = v;
	entry_length = *p++ - 0xC;
	data_length += sizeof(unsigned char);
	buff = p;

	data_length += (entry_count * entry_length);
	p += entry_count * entry_length;
}

unsigned long
psb_array_t::
size(void) const {
	return entry_count;
}

unsigned long
psb_array_t::
get(unsigned long index) const {
	unsigned long v = 0;

	unsigned char* p = buff + index * entry_length;

	for (unsigned long i = 0; i < entry_length; i++) {
		v |= *p++ << (i * 8);
	}

	return v;
}

/***************************************************************************
* psb_string_t
*/
psb_string_t::
psb_string_t(const psb_t&    psb,
	unsigned char*& p)
	: psb_value_t(psb, TYPE_STRING, p),
	buff(--p)
{

}

unsigned long
psb_string_t::
get_index() const {
	return psb.get_string_index(buff);
}

string
psb_string_t::
get_string() const {
	return psb.get_string(buff);
}

/***************************************************************************
* psb_objects_t
*/
psb_objects_t::
psb_objects_t(const psb_t&    psb,
	unsigned char*& p)
	: psb_value_t(psb, TYPE_OBJECTS, p),
	buff(p)
{
	names = new psb_array_t(psb, buff);
	offsets = new psb_array_t(psb, buff);
}

psb_objects_t::
~psb_objects_t(void) {
	delete offsets;
	delete names;
}

unsigned long
psb_objects_t::
size(void) const {
	return names->size();
}

string
psb_objects_t::
get_name(unsigned long index) const {
	return psb.get_name(names->get(index));
}

unsigned char*
psb_objects_t::
get_data(unsigned long index) const {
	return buff + offsets->get(index);
}

unsigned char*
psb_objects_t::
get_data(const string& name) const {
	for (unsigned long i = 0; i < names->size(); i++) {
		if (get_name(i) == name) {
			return get_data(i);
		}
	}

	return NULL;
}



/***************************************************************************
* psb_offsets_t
*/
psb_list_t::
psb_list_t(const psb_t&    psb,
	unsigned char*& p)
	: psb_value_t(psb, TYPE_LIST, p)
{
	offsets = new psb_array_t(psb, p);
	buff = p;
}

psb_list_t::
~psb_list_t(void) {
	delete offsets;
}

unsigned long
psb_list_t::
size(void) const {
	return offsets->size();
}

unsigned char*
psb_list_t::
get(unsigned long index) const {
	return buff + offsets->get(index);
}

/***************************************************************************
* psb_t
*/
psb_t::
psb_t(unsigned char* buff) {
	this->buff = buff;
	hdr = (PSBHDR*)buff;

	unsigned char* p = buff + hdr->offset_names;
	str1 = new psb_array_t(*this, p);
	str2 = new psb_array_t(*this, p);
	str3 = new psb_array_t(*this, p);

	p = buff + hdr->offset_strings;
	strings = new psb_array_t(*this, p);

	strings_data = (char*)(buff + hdr->offset_strings_data);

	p = buff + hdr->offset_chunk_offsets;
	chunk_offsets = new psb_array_t(*this, p);

	p = buff + hdr->offset_chunk_lengths;
	chunk_lengths = new psb_array_t(*this, p);

	chunk_data = buff + hdr->offset_chunk_data;

	p = buff + hdr->offset_entries;
	unpack(objects, p);

	expire_suffix_list = NULL;

	if (objects) {
		objects->unpack(expire_suffix_list, "expire_suffix_list");

		if (expire_suffix_list) {
			// Hrm ... ever more than one?
			extension = get_string(expire_suffix_list->get(0));
		}
	}
}

psb_t::
~psb_t(void) {
	delete expire_suffix_list;
	delete objects;
	delete chunk_lengths;
	delete chunk_offsets;
	delete strings;
	delete str3;
	delete str2;
	delete str1;
}

string
psb_t::
get_name(unsigned long index) const {
	string accum;

	unsigned long a = str3->get(index);
	unsigned long b = str2->get(a);

	while (true) {
		unsigned long c = str2->get(b);
		unsigned long d = str1->get(c);
		unsigned long e = b - d;

		b = c;

		accum = (char)e + accum;

		if (!b) {
			break;
		}
	}

	return accum;
}

unsigned long
psb_t::
get_number(unsigned char* p) const {
	static const unsigned long TYPE_TO_KIND[] = {
		0, 1, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 0xA, 0xB, 0xC
	};

	unsigned char  type = *p++;
	unsigned long  kind = TYPE_TO_KIND[type];
	unsigned long  v = 0;

	switch (kind) {
	case 1:
		v = 0;
		break;

	case 2:
		v = 1;
		break;

	case 3:
	{
		unsigned long n = type - 4;

		for (unsigned long i = 0; i < n; i++) {
			v |= *p++ << (i * 8);
		}
	}
	break;

	case 9:
		if (type == 0x1E) {
			v = (unsigned long)*(float*)p;
		}
		break;

	case 10:
		if (type == 0x1F) {
			v = (unsigned long)*(double*)p;
			p += 8;
		}
		break;

	default:
		printf("warning: unsupported packed number type (%d)\n", kind);
		break;
	}

	return v;
}

string
psb_t::
get_string(unsigned char* p) const {
	unsigned long n = *p++ - 0x14;
	unsigned long v = 0;

	for (unsigned long i = 0; i < n; i++) {
		v |= *p++ << (i * 8);
	}

	return strings_data + strings->get(v);
}

unsigned long
psb_t::
get_string_index(unsigned char* p) const {
	unsigned long n = *p++ - 0x14;
	unsigned long v = 0;

	for (unsigned long i = 0; i < n; i++) {
		v |= *p++ << (i * 8);
	}

	return v;
}

const psb_objects_t*
psb_t::
get_objects(void) const {
	return objects;
}

unsigned char*
psb_t::
get_chunk(unsigned char* p) const {
	return chunk_data + chunk_offsets->get(get_chunk_index(p));
}

unsigned long
psb_t::
get_chunk_length(unsigned char* p) const {
	return chunk_lengths->get(get_chunk_index(p));
}

psb_value_t*
psb_t::
unpack(unsigned char*& p) const {
	unsigned char type = *p++;

	switch (type) {
	case psb_value_t::TYPE_N0:
	case psb_value_t::TYPE_N1:
	case psb_value_t::TYPE_N2:
	case psb_value_t::TYPE_N3:
	case psb_value_t::TYPE_N4:
		return new psb_number_t(*this, --p);

	case psb_value_t::TYPE_ARRAY:
		return new psb_array_t(*this, p);

	case psb_value_t::TYPE_LIST:
		return new psb_list_t(*this, p);

	case psb_value_t::TYPE_OBJECTS:
		return new psb_objects_t(*this, p);

	case psb_value_t::TYPE_STRING:
	case psb_value_t::TYPE_STRING2:
		return new psb_string_t(*this, p);

	default:
		p--;
		return NULL;
	}

	return NULL;
}

unsigned long
psb_t::
get_chunk_index(unsigned char* p) const {
	unsigned long n = *p++ - 0x18;
	unsigned long v = 0;

	for (unsigned long i = 0; i < n; i++) {
		v |= *p++ << (i * 8);
	}

	return v;
}