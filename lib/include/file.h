#ifndef FILE_H
#define FILE_H

struct MetaLog
{
   int start;            // ����һ�������START��1296744718 4
	// char start[4];
   char filename[128];   // 128
	// char filename[16];
    char operation[16];   // write, creat, unlink, link 10
    int startpos, endpos; // [startpos, endpos) // -1, -1 8
    // char startpos[8];
    // char endpos[8];
    int end;              // ����һ�������END��1437806543 4
    int checkpoint;       // 317668571 4
    // char end[4];
    // char checkpoint[4];
}__attribute__ ((__packed__));

struct st_inode
{
	unsigned int st_mode;	/* ״̬�ı�־λ */
	int		st_nlink;		/* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */
	
	short	st_uid;			/* �ļ������ߵ��û���ʶ�� */
	short	st_gid;			/* �ļ������ߵ����ʶ�� */
	
	int		st_size;			/* �ļ���С���ֽ�Ϊ��λ */
	int		st_addr[10];		/* �����ļ��߼���ú�������ת���Ļ��������� */
	
	int		st_atime;		/* ������ʱ�� */
	int		st_mtime;		/* ����޸�ʱ�� */
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