#include "psb_compiler_center.h"

psb_compiler_center pcc;


psb_compiler_center::psb_compiler_center()
{
}


psb_compiler_center::~psb_compiler_center()
{
}

bool psb_compiler_center::require_compile(string src_file, string res_file, string output_file)
{
	_src_file = src_file;
	_res_file = res_file;
	_output_file = output_file;

	_source.open(src_file, ios::in);
	_resource.open(res_file, ios::in);
	_output.open(output_file, ios::binary | ios::out | ios::trunc);

	if (!_source.is_open())
	{
		cout << "source code open failed" << endl;
		return false;
	}

	if (!_resource.is_open())
	{
		cout << "resource script open failed" << endl;
		return false;
	}

	if (!_output.is_open())
	{
		cout << "output file create failed" << endl;
		return false;
	}

	if (!source_reader.parse(_source, source_code))
	{
		cout << "source script parse failed" << endl;
		return false;
	}

	if (!resource_reader.parse(_resource, resource_code))
	{
		cout << "resource script parse failed" << endl;
		return false;
	}

	return true;
}

bool psb_compiler_center::compile()
{
	return _compiler.cc(source_code);
}

bool psb_compiler_center::link()
{
	return _link.link(_compiler, resource_code);
}

bool psb_compiler_center::write_file()
{
	_output.write((const char*)_link.data(), _link.length());
	_output.flush();
	_output.close();
	return true;
}
