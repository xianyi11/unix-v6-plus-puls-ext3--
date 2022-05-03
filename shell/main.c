#include "stdio.h"
#include "stdlib.h"
#include "sys.h"

#include "CommandTree.h"
#include "ExecuteCommand.h"
#include "globe.h"
#include "PreExecute.h"
#include "string.h"
#include "file.h"
#include "../lib/include/malloc.h"

char line[1000];
char* args[50];
int argsCnt;
void CheckAndRecovery()
{
	printf("entering CheckAndRecovery......\n");
	int metalogfd=open("/meta.log",0x3);
	if(metalogfd<0){
		printf("/meta.log does not exsit ! leaving CheckAndRecovery......\n");
		return;
	}
	struct st_inode st;
	fstat(metalogfd,&st);
	int remainder=st.st_size% sizeof(struct MetaLog);
	//日志有问题
	if(remainder!=0){
		if(remainder<sizeof(struct MetaLog)-sizeof(int)){
			//日志未写完
			printf("the log in /meta.log is not enough to recover ......\n");
			;//pass
		}
		else if(remainder>=sizeof(struct MetaLog)-sizeof(int)){
			//已提交但是未加checkpoint
			//redo
			int pos=st.st_size/sizeof(struct MetaLog)*sizeof(struct MetaLog);
			seek(metalogfd,pos,0);
			struct MetaLog metalog;
			read(metalogfd, (char*)&metalog, remainder);
			//write
			if(strcmp( "write", metalog.operation )==0){
				int DataLen = metalog.endpos - metalog.startpos;//获得长度
//				char *buf = (char *)malloc(DataLen + 5);//? ///////////////
				char buf[512];
				int datalogfd = open("/data.log",0x3);
				seek(datalogfd,metalog.startpos,0);
				read(datalogfd,buf,DataLen);
				int fd = open(metalog.filename1,0x3);
				int res_all = 0;
				int res = 0;
				while(res_all < DataLen)
				{
					__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(fd),"c"(buf+res_all),"d"(DataLen-res_all));///////
					res_all+=res;
				}
				//判断返回值
				printf("recover write %s finish!!!!\n",metalog.filename1);
			}
			//creat
			else if(strcmp( "creat", metalog.operation )==0){
				int res;
				__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(8),"b"(metalog.filename1),"c"(metalog.mode));
				printf("recover creat %s finish!!!!\n",metalog.filename1);
			}
			//link
			else if(strcmp( "link", metalog.operation )==0){
				int res;
				__asm__ volatile ("int $0x80":"=a"(res):"a"(9),"b"(metalog.filename1),"c"(metalog.filename2));
				printf("recover link %s to %s finish!!!!\n",metalog.filename2,metalog.filename1);
			}
			//unlink
			else if(strcmp( "unlink", metalog.operation )==0){
				int res;
				__asm__ volatile ("int $0x80":"=a"(res):"a"(10),"b"(metalog.filename1));
				printf("recover unlink %s finish!!!!\n",metalog.filename1);
			}
			else{
				printf("unknown operation %s\n",metalog.operation);
			}
		}
	}
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(10),"b"("/meta.log"));
	__asm__ volatile ("int $0x80":"=a"(res):"a"(10),"b"("/data.log"));
	printf("finish CheckAndRecovery!!!!!\n");
}
int main1()
{
	CheckAndRecovery();
	char lineInput[512];
	getPath( curPath );	
	int root;
	while( 1 )
	{
		root = -1;
		argsCnt = 0;
		InitCommandTree();
		printf("[%s]#", curPath);
		gets( lineInput );		
		if ( strcmp( "shutdown", lineInput ) == 0 )
		{
			syncFileSystem();
			printf("You can safely turn down the computer now!\n");
			break;
		}
		argsCnt = SpiltCommand(lineInput);
		root = AnalizeCommand(0, argsCnt - 1, 0);
		if ( root >= 0 )
		{
			ExecuteCommand( &commandNodes[root], 0, 0 );
		}		
	}

	return 0;
}
