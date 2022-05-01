#include <stdio.h>
#include <file.h>

char* message = "Stuff this in your pipe and smoke it";

int main1(int argc, char* argv[])
{
	int fdSrc = -1;
	
	fdSrc = open(argv[1], 0x1FF);
	if ( -1 == fdSrc )
		printf("Cannot Open File: %s\n", argv[1]);
    char buf[128];
    MyGetFileName(fdSrc,buf);
    printf("GetFileName: %s\n", buf); 
	return 1;
}
