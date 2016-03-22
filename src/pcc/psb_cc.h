#pragma once

class psb_cc_base;

class psb_cc
{
	friend psb_cc_base;
public:
	psb_cc();
	~psb_cc();

	bool cc(Json::Value &src);

	uint32_t add_string(string value);
	uint32_t add_names(string value);
	string get_names(uint32_t index);
	psb_cc_base* pack(Json::Value& source_code);

	psb_cc_base* get_entries();

public:
	vector<string> _string_table;
	vector<string> _name_table;
	psb_cc_base *_entries;
};