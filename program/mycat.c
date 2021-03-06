#include<file.h>
#include<stdlib.h>

void usage()
{
     printf("Usage: mycat filename number\n");
}

void main1(int argc, char **argv)
{
	if(argc != 3)
	{
		usage();
		return;
	}
	int fd;
	fd = open(argv[1], 0111);
	struct MetaLog metalog;
	int cnt = 0;
	int i;
	int k = 1;
	int end = 0;
	for(;argv[2][end]; ++end)
		;
	--end;
	for(i = end; i>=0; --i)
	{
		cnt += (argv[2][i] - '0') * k;
		k *= 10;
	}
	seek(fd, cnt * sizeof(struct MetaLog), 0);
	read(fd, (char*)&metalog, sizeof(struct MetaLog));
	printf("start: %d\nfilename1: %s\nfilename2: %s\noperation: %s\nstartpos: %d\nendpos: %d\nmode: %d\nend: %d\ncheckpoint: %d\n",
			metalog.start, metalog.filename1, metalog.filename2, metalog.operation, metalog.startpos, metalog.endpos, metalog.mode, metalog.end, metalog.checkpoint);
}
