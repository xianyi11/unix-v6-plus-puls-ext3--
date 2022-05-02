#ifndef FILE_H
#define FILE_H

struct MetaLog
{
   int start;            // 定义一个特殊的START，1296744718 4
	// char start[4];
   char filename[128];   // 128
	// char filename[16];
    char operation[16];   // write, creat, unlink, link 10
    int startpos, endpos; // [startpos, endpos) // -1, -1 8
    // char startpos[8];
    // char endpos[8];
    int end;              // 定义一个特殊的END，1437806543 4
    int checkpoint;       // 317668571 4
    // char end[4];
    // char checkpoint[4];
}__attribute__ ((__packed__));

struct st_inode
{
	unsigned int st_mode;	/* 状态的标志位 */
	int		st_nlink;		/* 文件联结计数，即该文件在目录树中不同路径名的数量 */
	
	short	st_uid;			/* 文件所有者的用户标识数 */
	short	st_gid;			/* 文件所有者的组标识数 */
	
	int		st_size;			/* 文件大小，字节为单位 */
	int		st_addr[10];		/* 用于文件逻辑块好和物理块好转换的基本索引表 */
	
	int		st_atime;		/* 最后访问时间 */
	int		st_mtime;		/* 最后修改时间 */
};

int creat(char* pathname, unsigned int mode);

int open(char* pathname, unsigned int mode);

int close(int fd);

int write(int fd, char* buf, int nbytes);

int read(int fd, char* buf, int nbytes);

int MyGetFileName(int fd, char* buf);

int mknod(char* pathname,unsigned int mode, int dev);

int link(char* pathname,char* newPathname);

int unlink(char* pathname);

int chdir(char* pathname);

int seek(int fd,unsigned int offset,unsigned int ptrname);

int chmod(char* pathname,unsigned int mode);

int chown(char* pathname,short uid, short gid);

int pipe(int* fildes);

int dup(int fd);

int fstat(int fd,unsigned long statbuf);

int stat(char* pathname,unsigned long statbuf);

#endif