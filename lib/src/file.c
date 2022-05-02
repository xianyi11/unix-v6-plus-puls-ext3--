#include <stdio.h>
#include <stdlib.h>
#include "file.h"

#define START 1296744718
#define END 1437806543
#define CHECKPOINT 317668571

/*
创建文件系统调用c库封装函数
name：创建路径的地址
mode：创建模式，需指定文件主，同组用户，其他用户的工作方式
返回值：成功则返回进程打开文件号，失败返回-1
*/
int creat(char* pathname, unsigned int mode)
{
	int res;
	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(8),"b"(pathname),"c"(mode));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
打开文件系统调用c库封装函数
name：打开文件路径的地址
mode：打开文件模式，采用读、写还是读写的方式
返回值：成功则返回进程打开文件号，失败返回-1
*/
int open(char* pathname, unsigned int mode)
{
	int res;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(5),"b"(pathname),"c"(mode));
	if ( res >= 0 )
	{
		return res;
	}
	return -1;
}

int close(int fd)
{
	int res;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(6),"b"(fd));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
读文件系统调用c库封装函数
fd：打开进程打开文件号
ubuf：目的区首地址
nbytes：要求读出的字节数
返回值：读取的实际数目（字节）
*/
int read(int fd, char* buf, int nbytes)
{
	int res;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(3),"b"(fd),"c"(buf),"d"(nbytes));
	if ( res >= 0 )
		return res;
	return -1;
}
int MyGetFileName(int fd, char* buf)
{
	int res;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(49),"b"(fd),"c"(buf));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
写文件系统调用c库封装函数
fd：打开进程打开文件号
ubuf：信息源首地址
nbytes：写入字节数
返回值：成功返回写入的实际数目（字节）
*/
int write(int fd, char* buf, int nbytes)
{
	if(fd == 0 || fd == 1)
	{
		int res;
		__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(fd),"c"(buf),"d"(nbytes));
		if ( res >= 0 )
			return res;
		return -1;
	}
	int res;
	// 写入日志、日志提交
	// open log
	int metafile = open("/meta.log", 0x3);
	if(metafile == -1)
	{
		creat("/meta.log", 0x1ff);
		metafile = open("/meta.log", 0x3);
	}
	// seek metafile
	seek(metafile, 0, 2); // move file ptr to end of file
	int datafile = open("/data.log", 0x3);
	if(datafile == -1)
	{
		creat("/data.log", 0x1ff);
		datafile = open("/data.log", 0x3);
	}
	// seek datafile
	seek(datafile, 0, 2);

	// write datafile
	struct st_inode datafile_state;
	fstat(datafile, &datafile_state);
	int startpos = datafile_state.st_size;
//	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(datafile),"c"(buf),"d"(nbytes));
//	if(res < 0)
//	{
//		close(metafile);
//		close(datafile);
//		return -1;
//	}
	int endpos = startpos + res;
	// write data into datafile

	struct MetaLog metalog;
//	metalog.start = START;
//	metalog.end = END;
//	metalog.checkpoint = CHECKPOINT;
//	MyGetFileName(fd, metalog.filename);
	sprintf(metalog.operation, "write");
//	metalog.startpos = startpos;
//	metalog.endpos = endpos;

	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog)));
//	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog) - sizeof(int)));
	if(res < 0)
	{
		close(metafile);
		close(datafile);
		return -1;
	}

	// checkpointing
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(fd),"c"(buf),"d"(nbytes));
	if ( res >= 0 )
	{
		// 加检查点
//		__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)(&metalog) + (sizeof(struct MetaLog) - sizeof(int))),"d"(sizeof(int)));
		close(metafile);
		close(datafile);
		return res;
	}
	close(metafile);
	close(datafile);
	return -1;
}

int pipe(int* fildes)
{
	int res;
	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(42),"b"(fildes));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
搜索文件位置系统调用c库封装函数
fd:打开文件号
如果ptrname == 0，则读写位置设置为offset
如果ptrname == 1，则读写位置加offset（可正可负）
如果ptrname == 2，则读写位置调整为文件长度加offset
如果ptrname > 2，为3～5，意义同0~2，但长度单位从一个字节变为512个字节
返回值：成功返回1，失败返回-1
*/
int seek(int fd,unsigned int offset,unsigned int ptrname)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(19),"b"(fd),"c"(offset),"d"(ptrname));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
复制file指针于进程打开文件表中系统调用c库封装函数
fd：进程打开打开文件号
返回值：复制的进程打开文件号
*/
int dup(int fd)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(41),"b"(fd));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
得到进程打开文件inode信息系统调用
fd：打开文件号
statbuf: 目的地址
返回值：成功返回1，失败返回-1
*/
int fstat(int fd,unsigned long statbuf)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(28),"b"(fd),"c"(statbuf));
	if ( res >= 0 )
		return res;
	return -1;
}
/*
得到进程打开文件inode信息系统调用
pathname：指定文件路径
des: 目的地址
返回值：成功返回1，失败返回-1
*/
int stat(char* pathname,unsigned long statbuf)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(18),"b"(pathname),"c"(statbuf));
	if ( res >= 0 )
		return res;
	return -1;
}
/*
改变文件访问模式系统调用c库封装函数
pathname：文件路径
mode：修改的模式
返回值：成功返回1，失败返回-1
*/
int chmod(char* pathname,unsigned int mode)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(15),"b"(pathname),"c"(mode));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
改变文件文件主号和文件同组号系统调用c库封装函数
pathname：文件路径
mode：修改的模式
返回值：成功返回1，失败返回-1
*/
int chown(char* pathname,short uid, short gid)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(16),"b"(pathname),"c"(uid),"d"(gid));
	if ( res >= 0 )
		return res;
	return -1;
}

/*
增加文件的访问路径系统调用c库封装函数
pathname：文件路径指针
newPathname：新的文件路径指针
返回值：成功返回1，失败返回-1
*/
int link(char* pathname,char* newPathname)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(9),"b"(pathname),"c"(newPathname));
	if ( res >= 0 )
		return res;
	return -1;
}
/*
解除文件索引系统调用c库封装函数
pathname：要解除索引的文件路径
返回值：成功返回1，失败返回-1
*/
int unlink(char* pathname)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(10),"b"(pathname));
	if ( res >= 0 )
		return res;
	return -1;
}
/*
改变当前目录系统调用c库封装函数
pathname：要改变到的路径指针
返回值：成功返回1，失败返回-1
*/
int chdir(char* pathname)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(12),"b"(pathname));
	if ( res >= 0 )
		return res;
	return -1;
}
/*
建立特殊文件系统调用c库封装函数
pathname：路径的指针
mode：创建模式
dev：设备号
返回值：成功返回1，失败返回-1
*/
int mknod(char* pathname,unsigned int mode, int dev)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(14),"b"(pathname),"c"(mode),"d"(dev));
	if ( res >= 0 )
		return res;
	return -1;
}


