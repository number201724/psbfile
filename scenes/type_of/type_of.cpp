// type_of.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdint.h>

int main()
{
	static const uint32_t TYPE_TO_KIND[] = {
		0, 1, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 10, 11, 12
	};


	for (int i = 0; i < sizeof(TYPE_TO_KIND); i++) {
		uint32_t n = TYPE_TO_KIND[i];
		switch (n)
		{
		case 1:
			printf("%02X => BOOL:FALSE\n", i);
			break;
		case 2:
			printf("%02X => BOOL:TRUE\n", i);
			break;
		case 3:
			printf("%02X => integer\n", i);
			break;

		case 9:
			printf("%02X => float\n", i);
			break;

		case 10:
			printf("%02X => double\n", i);
			break;
		}
	}
	return 0;
}

