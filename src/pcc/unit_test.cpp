/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include "psb_cc_base.h"
#include "psb_cc_array.h"
#include "psb_cc_integer.h"
#include "psb_cc_string.h"
#include "psb_cc_resource.h"
#include "psb_cc_decimal.h"
#include "psb_cc_btree.h"
//
//void ut_array()
//{
//
//	psb_cc_array* debug = new psb_cc_array();
//	debug->push_back(0xff);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff00);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff0000);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff000000);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff00000000);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff0000000000);
//	debug->compile();
//	debug->dump();
//
//	debug = new psb_cc_array();
//	debug->push_back(0xff000000000000);
//	debug->compile();
//	debug->dump();
//	debug = new psb_cc_array();
//	debug->push_back(0xff00000000000000);
//	debug->compile();
//	debug->dump();
//}
//
//
//void ut_integer()
//{
//	psb_cc_integer* int_cc = new psb_cc_integer(0x0);
//	int_cc->compile();
//	int_cc->dump();
//
//	int_cc = new psb_cc_integer(0xff);
//	int_cc->compile();
//	int_cc->dump();
//
//	int_cc = new psb_cc_integer(0xff00);
//	int_cc->compile();
//	int_cc->dump();
//
//	int_cc = new psb_cc_integer(0xff0000);
//	int_cc->compile();
//	int_cc->dump();
//	int_cc = new psb_cc_integer(0xff000000);
//	int_cc->compile();
//	int_cc->dump();
//	int_cc = new psb_cc_integer(0xff00000000);
//	int_cc->compile();
//	int_cc->dump();
//	int_cc = new psb_cc_integer(0xff0000000000);
//	int_cc->compile();
//	int_cc->dump();
//	int_cc = new psb_cc_integer(0xff000000000000);
//	int_cc->compile();
//	int_cc->dump();
//	int_cc = new psb_cc_integer(0xff00000000000000);
//	int_cc->compile();
//	int_cc->dump();
//}
//
//void ut_string()
//{
//	psb_cc ccc;
//
//
//	for (int i = 0; i < 1000; i++)
//	{
//		char s[32];
//		sprintf_s(s, "%d", i);
//		psb_cc_string* str = new psb_cc_string(&ccc, s);
//		str->compile();
//		str->dump();
//	}
//	
//
//}
//
//void ut_resource()
//{
//	psb_cc_resource *res = new psb_cc_resource(0);
//	res->compile();
//	res->dump();
//	res = new psb_cc_resource(0xff00);
//	res->compile();
//	res->dump();
//	res = new psb_cc_resource(0xff0000);
//	res->compile();
//	res->dump();
//	res = new psb_cc_resource(0xff000000);
//	res->compile();
//	res->dump();
//}
//
//void ut_decimal()
//{
//	psb_cc_decimal *oo = new psb_cc_decimal();
//	oo->compile();
//	oo->dump();
//
//	oo = new psb_cc_decimal(1.0f);
//	oo->compile();
//	oo->dump();
//
//	oo = new psb_cc_decimal(1.0);
//	oo->compile();
//	oo->dump();
//
//
//}
//void unit_test()
//{
//// 	string sarr[] = { "hash",
//// 		"name",
//// 		"scenes",
//// 		"firstLine",
//// 		"label",
//// 		"lines",
//// 		"meswinload_switch",
//// 		"ses",
//// 		"volume",
//// 		"characters",
//// 		"defaultAfy",
//// 		"absolute",
//// 		"clipx",
//// 		"diff",
//// 		"dress",
//// 		"face",
//// 		"level",
//// 		"posName",
//// 		"pose",
//// 		"xpos",
//// 		"layers",
//// 		"imageFile",
//// 		"clip",
//// 		"clipy",
//// 		"zoomx",
//// 		"zoomy",
//// 		"stage",
//// 		"blurx",
//// 		"blury",
//// 		"time",
//// 		"ypos",
//// 		"nexts",
//// 		"eval",
//// 		"exp",
//// 		"storage",
//// 		"target",
//// 		"type",
//// 		"postevals",
//// 		"preevals",
//// 		"selects",
//// 		"spCount",
//// 		"texts",
//// 		"bgm",
//// 		"filename",
//// 		"disp",
//// 		"height",
//// 		"imageColor",
//// 		"width",
//// 		"opacity",
//// 		"loop",
//// 		"title" };
//// 
//// 	psb_cc_btree *tree = new psb_cc_btree();
//// 	for each(string i in sarr)
//// 	{
//// 		tree->insert(i);
//// 	}
//	//tree->insert("absolute");
//	//tree->insert("bgm");
//	//tree->insert("blurx");
//	//tree->insert("harhaaa");
//	//tree->insert("hashaaa");
//	//tree->insert("compute");
//	//tree->insert("fuckyou");
//	//tree->compile();
//	//exit(0);
//	//ut_array();
//	//ut_integer();
//	//ut_string();
//	//ut_resource();
//	//ut_decimal();
//
//	//exit(0);
//}