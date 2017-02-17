#include "metadata.h"

int main(int argc, char const *argv[])
{
	metadata * md;
	struct stat sample;

	dirInfo * dInfo;

	md_create(&md,200);

	md_add_dinode(md, sample, 1);
	md_add_dinode(md, sample, 2);
	md_create_dirInfo(md, &dInfo);
	md_add_dirEntry(md,&dInfo,".",1);
	md_add_dirEntry(md,&dInfo,"..",2);
	md_add_dirEntry(md,&dInfo,"fileA",3);
	md_create_dirInfo(md,&dInfo);
	md_add_dirEntry(md,&dInfo,".",2);
	md_add_dirEntry(md,&dInfo,"..",3);

	md_printall(md);
	md_free(&md);

	return 0;
}