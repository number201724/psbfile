#include "psb_compiler_center.h"
#include <direct.h>
psb_compiler_center pcc;


psb_compiler_center::psb_compiler_center()
{
}


psb_compiler_center::~psb_compiler_center()
{
}
string psb_compiler_center::get_directory(string src_file)
{
	char path[512];
	strcpy_s(path,src_file.c_str());
	char* pos = strrchr(path,'\\');
	if(pos != NULL){
		pos++;
		*pos = 0;
	}else{
		_getcwd(path,sizeof(path));
		strcat_s(path,"\\");
	}
	return path;
}
string psb_compiler_center::get_res_filename(string src_file)
{
	char filename[512];
	strcpy_s(filename,src_file.c_str());

	char* p = strrchr(filename,'.');

	if(p)
	{
		*p = 0;
	}

	strcat_s(p,sizeof(filename) - (p - filename),".res.json");

	return filename;
}
bool psb_compiler_center::can_load_resource()
{
	if (!_resource.is_open())
	{
		return false;
	}
	return true;
}
bool psb_compiler_center::require_compile(string src_file, string output_file)
{
	_src_file = src_file;
	_output_file = output_file;

	_src_dir = get_directory(src_file);
	_source.open(src_file, ios::in);
	
	_output.open(output_file, ios::binary | ios::out | ios::trunc);

	if (!_source.is_open())
	{
		cout << "source code open failed" << endl;
		return false;
	}

	if (!_output.is_open())
	{
		cout << "output file create failed" << endl;
		return false;
	}

	_resource.open(get_res_filename(src_file), ios::in);

	if(can_load_resource())
	{
		if (!resource_reader.parse(_resource, resource_code))
		{
			cout << "resource script parse failed" << endl;
			return false;
		}
	}
	else{
		resource_code = Json::Value(Json::arrayValue);
	}
	if (!source_reader.parse(_source, source_code))
	{
		cout << "source script parse failed" << endl;
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
	return _link.link(_compiler, resource_code,_src_dir);
}

bool psb_compiler_center::write_file()
{
	_output.write((const char*)_link.data(), _link.length());
	_output.flush();
	_output.close();
	return true;
}
