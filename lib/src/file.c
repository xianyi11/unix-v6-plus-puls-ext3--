#include <stdio.h>
#include <stdlib.h>
#include "file.h"

#define START 1296744718
#define END 1437806543
#define CHECKPOINT 317668571

/*
�����ļ�ϵͳ����c���װ����
name������·���ĵ�ַ
mode������ģʽ����ָ���ļ�����ͬ���û��������û��Ĺ�����ʽ
����ֵ���ɹ��򷵻ؽ��̴��ļ��ţ�ʧ�ܷ���-1
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
���ļ�ϵͳ����c���װ����
name�����ļ�·���ĵ�ַ
mode�����ļ�ģʽ�����ö���д���Ƕ�д�ķ�ʽ
����ֵ���ɹ��򷵻ؽ��̴��ļ��ţ�ʧ�ܷ���-1
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
���ļ�ϵͳ����c���װ����
fd���򿪽��̴��ļ���
ubuf��Ŀ�����׵�ַ
nbytes��Ҫ��������ֽ���
����ֵ����ȡ��ʵ����Ŀ���ֽڣ�
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
д�ļ�ϵͳ����c���װ����
fd���򿪽��̴��ļ���
ubuf����ϢԴ�׵�ַ
nbytes��д���ֽ���
����ֵ���ɹ�����д���ʵ����Ŀ���ֽڣ�
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
	// д����־����־�ύ
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
		// �Ӽ���
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
�����ļ�λ��ϵͳ����c���װ����
fd:���ļ���
���ptrname == 0�����дλ������Ϊoffset
���ptrname == 1�����дλ�ü�offset�������ɸ���
���ptrname == 2�����дλ�õ���Ϊ�ļ����ȼ�offset
���ptrname > 2��Ϊ3��5������ͬ0~2�������ȵ�λ��һ���ֽڱ�Ϊ512���ֽ�
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
����fileָ���ڽ��̴��ļ�����ϵͳ����c���װ����
fd�����̴򿪴��ļ���
����ֵ�����ƵĽ��̴��ļ���
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
�õ����̴��ļ�inode��Ϣϵͳ����
fd�����ļ���
statbuf: Ŀ�ĵ�ַ
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
�õ����̴��ļ�inode��Ϣϵͳ����
pathname��ָ���ļ�·��
des: Ŀ�ĵ�ַ
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
�ı��ļ�����ģʽϵͳ����c���װ����
pathname���ļ�·��
mode���޸ĵ�ģʽ
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
�ı��ļ��ļ����ź��ļ�ͬ���ϵͳ����c���װ����
pathname���ļ�·��
mode���޸ĵ�ģʽ
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
�����ļ��ķ���·��ϵͳ����c���װ����
pathname���ļ�·��ָ��
newPathname���µ��ļ�·��ָ��
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
����ļ�����ϵͳ����c���װ����
pathname��Ҫ����������ļ�·��
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
�ı䵱ǰĿ¼ϵͳ����c���װ����
pathname��Ҫ�ı䵽��·��ָ��
����ֵ���ɹ�����1��ʧ�ܷ���-1
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
���������ļ�ϵͳ����c���װ����
pathname��·����ָ��
mode������ģʽ
dev���豸��
����ֵ���ɹ�����1��ʧ�ܷ���-1
*/
int mknod(char* pathname,unsigned int mode, int dev)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(14),"b"(pathname),"c"(mode),"d"(dev));
	if ( res >= 0 )
		return res;
	return -1;
}


