#include<file.h>

void main1(int argc, char **argv)
{
	int fd;
	fd = open(argv[1], 0111);
	struct MetaLog metalog;
	read(fd, (char*)&metalog, 60);
	printf("start: %s\nfilename: %s\noperation: %s\nstartpos: %s\nendpos: %s\nend: %s\ncheckpoint: %s\n",
			metalog.start, metalog.filename, metalog.operation, metalog.startpos, metalog.endpos, metalog.end, metalog.checkpoint);
}
