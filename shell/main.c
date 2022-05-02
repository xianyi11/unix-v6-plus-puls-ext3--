#include "stdio.h"
#include "stdlib.h"
#include "sys.h"

#include "CommandTree.h"
#include "ExecuteCommand.h"
#include "globe.h"
#include "PreExecute.h"
#include "string.h"
#include"file.h"

char line[1000];
char* args[50];
int argsCnt;
void CheckAndRecovery()
{
	int metalogfd=open("/meta.log",0x3);
	if(metalogfd<0){
		return;
	}
	struct st_inode st;
	fstat(metalogfd,&st);
	int remainder=st.st_size% sizeof(struct MetaLog);
	//日志有问题
	if(remainder!=0){
		if(remainder<sizeof(struct MetaLog)-sizeof(int)){
			//日志未写完
			return;
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

			}
			//creat
			else if(strcmp( "creat", metalog.operation )==0){
				
			}
			//link
			else if(strcmp( "link", metalog.operation )==0){

			}
			//unlink
			else if(strcmp( "unlink", metalog.operation )==0){
				
			}
			else{
				printf("unknown operation %s\n",metalog.operation);
			}
		}
	}
}
int main1()
{
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
