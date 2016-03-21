#pragma once

#include "def.h"
#include "psb_link.h"

class psb_compiler_center
{
public:
	psb_compiler_center();
	~psb_compiler_center();

	bool require_compile(string src_file, string res_file, string output_file);

	bool compile();
	bool link();
	bool write_file();

protected:
	string _src_file;
	string _res_file;
	string _output_file;


	fstream _source;
	fstream _resource;
	fstream _output;

	Json::Value source_code;
	Json::Value resource_code;
	Json::Reader source_reader;
	Json::Reader resource_reader;

	psb_cc _compiler;
	psb_link _link;

public:
	string src_file() const
	{
		return _src_file;
	}

	string res_file() const
	{
		return _res_file;
	}

	string output_file() const
	{
		return _output_file;
	}
};

extern psb_compiler_center pcc;