#include <stdio.h>
#include <stdlib.h>
#include "dilists.h"

int main()
{
	dirInfoHead * dirList;

	dirInfo_create(&dirList,2,1);

	dirInfo_add(dirList,"fileA",3);
	dirInfo_add(dirList,"dirA",4);
	dirInfo_add(dirList,"dirB",5);

	dirInfo_printall(dirList);

	dirInfo_delete(dirList,"dirA");

	dirInfo_printall(dirList);

	dirInfo_free(&dirList);

	return 0;
}