#include <stdio.h>
#include "../../common/service.h"

void WriteVersion()
{
#ifndef __WIN32__
	FILE* fp = fopen("VERSION.txt", "w");

	if (fp)
	{
		fprintf(fp, "__GAME_VERSION__: %s\n", __GAME_VERSION__);
		fprintf(fp, "%s@%s:%s\n", __USER__, __HOSTNAME__, __PWD__);
#ifdef __PRINT_BUILD_INFO__
		fprintf(fp, "Build Time: " __DATE__ " " __TIME__);
#endif // __PRINT_BUILD_INFO__
		fclose(fp);
	}
#endif
}