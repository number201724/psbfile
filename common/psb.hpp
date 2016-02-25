#ifndef _PSB_H_INCLUDED_
#define _PSB_H_INCLUDED_

/*
M2 PSB Editor
Author:201724
email:number201724@me.com
qq:495159

original by asmodean['expimg']
*/
#include <stdio.h>
#include <tchar.h>

#include <string>
#include <map>
#include <vector>

using namespace std;

struct PSBHDR {
	unsigned char signature[4];
	unsigned long type;
	unsigned long unknown1;
	unsigned long offset_names;
	unsigned long offset_strings;
	unsigned long offset_strings_data;
	unsigned long offset_chunk_offsets;
	unsigned long offset_chunk_lengths;
	unsigned long offset_chunk_data;
	unsigned long offset_entries;
};


/***************************************************************************
* A whole bunch of packed data junk...
***************************************************************************/

class psb_t;

/***************************************************************************
* psb_value_t
*/
class psb_value_t {
public:
	// Probably this should actually be kind as in get_number below. Don't care.
	enum type_t {
		TYPE_ARRAY = 0x31337, // fake

		//number
		TYPE_N0 = 0x4,
		TYPE_N1 = 0x5,
		TYPE_N2 = 0x6,
		TYPE_N3 = 0x7,
		TYPE_N4 = 0x8,

		TYPE_STRING = 0x15,
		TYPE_STRING2 = 0x16,
		TYPE_LIST = 0x20,
		TYPE_OBJECTS = 0x21,
	};

	psb_value_t(const psb_t&    psb,
		type_t          type,
		unsigned char*& p);
	psb_value_t(const psb_t&    psb,
		unsigned char*& p);
	

	virtual ~psb_value_t(void);

	type_t get_type(void) const;
	virtual string get_type_string() = 0;

protected:
	const psb_t& psb;
	type_t type;
};
/***************************************************************************
* psb_number_t
*/
class psb_number_t : public psb_value_t {
public:
	psb_number_t(const psb_t&    psb,
		unsigned char*& p);

	unsigned long get_number() const;

	static bool is_number(psb_value_t *value);

	virtual string get_type_string() {
		return "psb_number_t";
	}
private:
	unsigned long number;
};
/***************************************************************************
* psb_array_t
*/
class psb_array_t : public psb_value_t {
public:
	psb_array_t(const psb_t&    psb,
		unsigned char*& p);

	unsigned long size(void) const;

	unsigned long get(unsigned long index) const;

	unsigned long  data_length;
	unsigned long  entry_count;
	unsigned long  entry_length;
	unsigned char* buff;

	virtual string get_type_string() {
		return "psb_array_t";
	}
};
/***************************************************************************
* psb_string_t
*/
class psb_string_t : public psb_value_t {
public:
	psb_string_t(const psb_t&    psb,
		unsigned char*& p);

	unsigned long get_index() const;

	string get_string() const;

	unsigned char* buff;

	virtual string get_type_string() {
		return "psb_string_t";
	}
};
/***************************************************************************
* psb_objects_t
*/
class psb_objects_t : public psb_value_t {
public:
	psb_objects_t(const psb_t&    psb,
		unsigned char*& p);

	~psb_objects_t(void);

	unsigned long size(void) const;

	string get_name(unsigned long index) const;

	unsigned char* get_data(unsigned long index) const;

	unsigned char* get_data(const string& name) const;

	template<class T> void unpack(T*& out, const string& name) const;

	virtual string get_type_string() {
		return "psb_objects_t";
	}

public:
	psb_array_t*   names;
	psb_array_t*   offsets;
	unsigned char* buff;
};

/***************************************************************************
* psb_list_t
*/
class psb_list_t : public psb_value_t {
public:
	psb_list_t(const psb_t&    psb,
		unsigned char*& p);

	~psb_list_t(void);

	unsigned long size(void) const;

	unsigned char* get(unsigned long index) const;

	template<class T> void unpack(T*& out, unsigned long index) const;

	virtual string get_type_string() {
		return "psb_list_t";
	}

public:
	psb_array_t*   offsets;
	unsigned char* buff;
};

/***************************************************************************
* psb_t
*/
class psb_t {
public:
	psb_t(unsigned char* buff);

	~psb_t(void);

	string get_name(unsigned long index) const;

	unsigned long get_number(unsigned char* p) const;

	string get_string(unsigned char* p) const;
	unsigned long get_string_index(unsigned char* p) const;

	const psb_objects_t* get_objects(void) const;

	unsigned char* get_chunk(unsigned char* p) const;

	unsigned long get_chunk_length(unsigned char* p) const;

	//string make_filename(const string& name) const;

	psb_value_t* unpack(unsigned char*& p) const;

	template<class T> void unpack(T*& out, unsigned char*& p) const;

public:
	unsigned long get_chunk_index(unsigned char* p) const;

	unsigned char* buff;
	PSBHDR*        hdr;
	psb_array_t*   str1;
	psb_array_t*   str2;
	psb_array_t*   str3;
	psb_array_t*   strings;
	char*          strings_data;
	psb_array_t*   chunk_offsets;
	psb_array_t*   chunk_lengths;
	unsigned char* chunk_data;

	psb_objects_t* objects;
	psb_list_t* expire_suffix_list;

	string         extension;
};

template<class T>
void
psb_objects_t::
unpack(T*& out, const string& name) const {
	out = NULL;

	unsigned char* temp = get_data(name);

	if (temp) {
		psb.unpack(out, temp);
	}
}

template<class T>
void
psb_list_t::
unpack(T*& out, unsigned long index) const {
	unsigned char* temp = get(index);

	psb.unpack(out, temp);
}

template<class T>
void
psb_t::
unpack(T*& out, unsigned char*& p) const {
	out = dynamic_cast<T*>(unpack(p));
}


#endif