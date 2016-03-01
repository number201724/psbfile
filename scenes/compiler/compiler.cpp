#include <iostream>
#include <fstream>
#include <string>
#include <json/json.h>
#include "../../common/psb.hpp"
using namespace std;



int main(int argc, char* argv[])
{
	Json::Reader reader;
	Json::Value root;
	fstream input("title.json",ios::in);
		
	reader.parse(input,root);
	
	
	cout << root.toStyledString() << endl;

	return 0;
}
