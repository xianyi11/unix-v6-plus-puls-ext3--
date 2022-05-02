#include <stdio.h>
#include <stdlib.h>
#include "sys.h"
#include "string.h"
#include "../lib/include/file.h"

void usage()
{
     printf("Usage: recover write/creat/unlink/link 1/2\n");
}

void recover_write(int arg)
{
	// /test_write.txt
	creat("/test_write.txt", 0x1ff);
	int testfd = open("/test_write.txt", 0x3);
	int metafile = open("/meta.log", 0x3);
	if(metafile == -1)
	{
		metafile = creat("/meta.log", 0x1ff);
	}
	struct st_inode metafile_state;
	fstat(metafile, &metafile_state);
	seek(metafile, metafile_state.st_size, 0);
	int datafile = open("/data.log", 0x3);
	if(datafile == -1)
	{
		datafile = creat("/data.log", 0x1ff);
	}
	struct st_inode datafile_state;
	fstat(datafile, &datafile_state);
	seek(datafile, datafile_state.st_size, 0);
	struct MetaLog metalog;
	metalog.start = START;
	sprintf(metalog.filename1, "/test_write.txt");
	metalog.filename2[0] = '\0';
	sprintf(metalog.operation, "write");
	metalog.startpos = datafile_state.st_size;
	metalog.endpos = datafile_state.st_size + 10;
	metalog.mode = 0;
	metalog.end = END;
	metalog.checkpoint = CHECKPOINT;
	char buf[12] = "1234567890";
	int res;
	if(arg == 1) // not enough, <
	{
		printf("arg1\n");
		__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(148));
	}
	else if(arg == 2)
	{
		__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(metalog) - sizeof(int)));
		__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(datafile),"c"(buf),"d"(10));
	}
}

void recover_creat(int arg)
{

}

void recover_link(int arg)
{

}

void recover_unlink(int arg)
{

}

void main1(int argc, char **argv)
{
	// recover write/creat/unlink/link 1/2
	if(argc != 3 || (argv[2][0] != '1' && argv[2][0] != '2'))
	{
		usage();
		return;
	}
	if(strcmp(argv[1], "write") == 0)
		recover_write(argv[2][0] - '0');
	else if(strcmp(argv[1], "creat") == 0)
		recover_creat(argv[2][0] - '0');
	else if(strcmp(argv[1], "link") == 0)
		recover_link(argv[2][0] - '0');
	else if(strcmp(argv[1], "unlink") == 0)
		recover_unlink(argv[2][0] - '0');
}
