#include <sys.h>
void main1(int argc, char **argv)
{
    char path[32];
    getPath(path);
    printf("pwd:%s\n",path);
}
