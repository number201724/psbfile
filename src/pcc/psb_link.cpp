/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/
#include "psb_cc_base.h"
#include "cc_stream.h"
#include "psb_link.h"
#include "psb_link_resource.h"



psb_link::psb_link() :
	_data(nullptr), _length(0)
{
}


psb_link::~psb_link()
{
}

bool psb_link::link(psb_cc &cc, Json::Value& resource_code,string res_path)
{
	cc_stream complete_data;
	PSBHDR hdr;
	_compiler = &cc;
	_res_path = res_path;

	psb_cc_base* entries = cc.get_entries();
	
	if(!link_string_table())
	{
		cout << "link string table failed" << endl;
		return false;
	}

	if(!link_names_table())
	{
		cout << "link names table failed" << endl;
	}

	if(!link_resource_table(resource_code))
	{
		cout << "link resource table failed" << endl;
		return false;
	}

	memcpy(&hdr.signature, "PSB", 4);
	hdr.type = 2;
	hdr.unknown1 = 0;
	complete_data.append((unsigned char*)&hdr, sizeof(hdr));

	//it's stream ,don't modify
	hdr.offset_names = complete_data.get_length();
	complete_data.append(names->get_data(), names->get_length());

	hdr.offset_entries = complete_data.get_length();
	complete_data.append(entries->get_data(), entries->get_length());

	hdr.offset_strings = complete_data.get_length();
	complete_data.append(_strings_cc->get_data(), _strings_cc->get_length());

	hdr.offset_strings_data = complete_data.get_length();
	complete_data.append(_string_data.get_data(), _string_data.get_length());

	hdr.offset_chunk_offsets = complete_data.get_length();
	complete_data.append(_resource->chunk_offsets.get_data(), _resource->chunk_offsets.get_length());
	hdr.offset_chunk_lengths = complete_data.get_length();
	complete_data.append(_resource->chunk_lengths.get_data(), _resource->chunk_lengths.get_length());
	hdr.offset_chunk_data = complete_data.get_length();
	complete_data.append(_resource->chunk_data.get_data(), _resource->chunk_data.get_length());

	complete_data.replace(0, (const unsigned char*)&hdr, sizeof(hdr));

	_data = complete_data.copy_buffer();
	_length = complete_data.get_length();

	return true;
}

bool psb_link::link_string_table()
{
	_strings_cc = new psb_cc_array();
	_strings_cc->resize(_compiler->_string_table.size());

	for (uint32_t i = 0; i < _compiler->_string_table.size(); i++)
	{
		_strings_cc->operator[](i) = _string_data.get_length();
		_string_data.append((unsigned char*)_compiler->_string_table[i].c_str(), _compiler->_string_table[i].length() + 1);
	}

	return _strings_cc->compile();
}

bool psb_link::link_names_table()
{
	names = new psb_cc_btree();
	
	for (uint32_t i = 0; i < _compiler->_name_table.size(); i++)
	{
		names->insert(_compiler->_name_table[i]);
	}

	return names->compile();

}

bool psb_link::link_resource_table(Json::Value& resource_code)
{
	_resource = new psb_link_resource();

	if (!_resource->load_resource(resource_code,_res_path))
	{
		return false;
	}
	if (!_resource->compile())
	{
		return false;
	}

	return true;
}
