/*
KRKR PSBFILE Compiler/Decompiler

Author:201724
QQ:495159
EMail:number201724@me.com
*/

#include "def.h"
#include "psb_compiler_center.h"
using namespace std;


void unit_test();

int main(int argc, char* argv[])
{
	cout << "KRKR PSB Package Compiler " << endl << "Author:201724" << endl;
	if(argc < 3)
	{
		cout << "usage: pcc <source_code_file> <output_psb_file>" << endl;
		return 0;
	}

	cout << "run interpreter...." << endl;
	if(!pcc.require_compile(argv[1], argv[2]))
	{
		cout << "interpreter failed" << endl;
		return 1;
	}
	cout << "interpreter ok" << endl;

	cout << "compile...." << endl;
	if(!pcc.compile())
	{
		cout << "psb file compile failed" << endl;
		return 1;
	}
	cout << "compile ok" << endl;

	cout << "linking...." << endl;
	if(!pcc.link())
	{
		cout << "psb file link failed" << endl;
		return 1;
	}
	cout << "link ok" << endl;
	if (!pcc.write_file()) {
		cout << "write complete data failed" << endl;
	}

	cout << "compile success ^_^" << endl;

	return 0;
}
