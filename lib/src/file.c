#include <stdio.h>
#include <stdlib.h>
#include "sys.h"
#include "string.h"
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
	if(strcmp(pathname, "/meta.log") == 0 || strcmp(pathname, "/data.log") == 0)
	{
		printf("creating %s\n", pathname);
		int res;
		// first part
		__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(8),"b"(pathname),"c"(mode));
		if ( res >= 0 )
		{
			// checkpoint
			return res;
		}
		return -1;
	}
	printf("creating %s\n", pathname);
	int res;
	// first part
	// open meta.log
	int metafile = open("/meta.log", 0x3);
	if(metafile == -1)
	{
		creat("/meta.log", 0x1ff);
		metafile = open("/meta.log", 0x3);
	}
	struct st_inode metafile_state;
	fstat(metafile, &metafile_state);
	seek(metafile, metafile_state.st_size, 0);

	struct MetaLog metalog;
	metalog.start = START;
	// absolute path
	if(pathname[0] == '/')
		sprintf(metalog.filename1, "%s", pathname);
	// relative path
	else
	{
		getPath(metalog.filename1);
		int pwd_len = strlen(metalog.filename1);
		if(pwd_len != 1)
			metalog.filename1[pwd_len++] = '/';
		sprintf(metalog.filename1 + pwd_len, "%s", pathname);
	}
	metalog.filename2[0] = '\0';
	sprintf(metalog.operation, "creat");
	metalog.startpos = 0;
	metalog.endpos = 0;
	metalog.mode = mode;
	metalog.end = END;
	metalog.checkpoint = CHECKPOINT;

	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog) - sizeof(int)));
//	printf("DEBUG!!! 3");
	if(res < 0)
	{
		close(metafile);
		return -1;
	}

	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(8),"b"(pathname),"c"(mode));
	if ( res >= 0 )
	{
		// checkpoint
		int chk_res;
		__asm__ __volatile__ ("int $0x80":"=a"(chk_res):"a"(4),"b"(metafile),"c"((char*)(&metalog) + (sizeof(struct MetaLog) - sizeof(int))),"d"(sizeof(int)));
		close(metafile);
		return res;
	}
	close(metafile);
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
//	printf("MYGETFILENAME!!!!!!BEFORE\n");
	int res;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(49),"b"(fd),"c"(buf));
//	printf("MYGETFILENAME!!!!!!AFTER\n");
	if ( res >= 0 )
	{
//		printf("RES!!!!Buf: %s\n", buf);
		return res;
	}
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
	struct st_inode metafile_state;
	fstat(metafile, &metafile_state);
	seek(metafile, metafile_state.st_size, 0); // move file ptr to end of file

	int datafile = open("/data.log", 0x3);
	if(datafile == -1)
	{
		creat("/data.log", 0x1ff);
		datafile = open("/data.log", 0x3);
	}
	struct st_inode datafile_state;
	fstat(datafile, &datafile_state);
	seek(datafile, datafile_state.st_size, 0);

	struct MetaLog metalog;

	///////

	// sprintf(metalog.start, "STT");
	sprintf(metalog.operation, "write");
	// sprintf(metalog.end, "END");
	// sprintf(metalog.checkpoint, "CHK");
	// sprintf(metalog.startpos, "%d", datafile_state.st_size);
	// sprintf(metalog.endpos, "%d", datafile_state.st_size + nbytes); //////////
	// printf("IN WRITE, BEFORE MYGETFILENAME!!!! %s \n",metalog.filename);
	// MyGetFileName(fd, metalog.filename);
	// printf("IN WRITE, AFTER MYGETFILENAME!!!!%s \n",metalog.filename);
	///////

	int startpos, endpos;
	startpos = datafile_state.st_size;
	// write datafile
	char *buf_backup = buf;
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(datafile),"c"(buf_backup),"d"(nbytes));
//	printf("DEBUG!!! 1");
	if(res < 0)
	{
		close(metafile);
		close(datafile);
		return -1;
	}
//	printf("DEBUG!!! 2");
	endpos = startpos + res;
	// write data into datafile

	metalog.start = START;
	metalog.end = END;
	metalog.checkpoint = CHECKPOINT;
	MyGetFileName(fd, metalog.filename1);
	metalog.filename2[0] = '\0';
	metalog.startpos = startpos;
	metalog.endpos = endpos;
	metalog.mode = 0;

//	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog)));
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog) - sizeof(int)));
//	printf("DEBUG!!! 3");
	if(res < 0)
	{
		close(metafile);
		close(datafile);
		return -1;
	}
//	printf("DEBUG!!! 4");
	// checkpointing
	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(fd),"c"(buf),"d"(nbytes));
//	printf("DEBUG!!! 5");
	if ( res >= 0 )
	{
		int chk_res;
		// �Ӽ���
		__asm__ __volatile__ ("int $0x80":"=a"(chk_res):"a"(4),"b"(metafile),"c"((char*)(&metalog) + (sizeof(struct MetaLog) - sizeof(int))),"d"(sizeof(int)));
		close(metafile);
		close(datafile);
		return res;
	}
//	printf("DEBUG!!! 6");
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
	printf("linking - %s - %s\n", pathname, newPathname);
	int res;
	int metafile = open("/meta.log", 0x3);
	if(metafile == -1)
	{
		creat("/meta.log", 0x1ff);
		metafile = open("/meta.log", 0x3);
	}
	struct st_inode metafile_state;
	fstat(metafile, &metafile_state);
	seek(metafile, metafile_state.st_size, 0);

	struct MetaLog metalog;
	metalog.start = START;
	// filename1
	if(pathname[0] == '/')
		sprintf(metalog.filename1, "%s", pathname);
	else
	{
		getPath(metalog.filename1);
		int pwd_len = strlen(metalog.filename1);
		if(pwd_len != 1)
			metalog.filename1[pwd_len++] = '/';
		sprintf(metalog.filename1 + pwd_len, "%s", pathname);
	}

	// filename2
	if(newPathname[0] == '/')
		sprintf(metalog.filename2, "%s", newPathname);
	else
	{
		getPath(metalog.filename2);
		int pwd_len = strlen(metalog.filename2);
		if(pwd_len != 1)
			metalog.filename2[pwd_len++] = '/';
		sprintf(metalog.filename2 + pwd_len, "%s", newPathname);
	}
	sprintf(metalog.operation, "link");
	metalog.startpos = 0;
	metalog.endpos = 0;
	metalog.mode = 0;
	metalog.end = END;
	metalog.checkpoint = CHECKPOINT;

	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog) - sizeof(int)));
//	printf("DEBUG!!! 3");
	if(res < 0)
	{
		close(metafile);
		return -1;
	}

	__asm__ volatile ("int $0x80":"=a"(res):"a"(9),"b"(pathname),"c"(newPathname));
	if ( res >= 0 )
	{
		// checkpoint
		int chk_res;
		__asm__ __volatile__ ("int $0x80":"=a"(chk_res):"a"(4),"b"(metafile),"c"((char*)(&metalog) + (sizeof(struct MetaLog) - sizeof(int))),"d"(sizeof(int)));
		close(metafile);
		return res;
	}
	close(metafile);
	return -1;
}
/*
����ļ�����ϵͳ����c���װ����
pathname��Ҫ����������ļ�·��
����ֵ���ɹ�����1��ʧ�ܷ���-1
*/
int unlink(char* pathname)
{
	printf("unlinking %s\n", pathname);
	int res;
	int metafile = open("/meta.log", 0x3);
	if(metafile == -1)
	{
		creat("/meta.log", 0x1ff);
		metafile = open("/meta.log", 0x3);
	}
	struct st_inode metafile_state;
	fstat(metafile, &metafile_state);
	seek(metafile, metafile_state.st_size, 0);

	struct MetaLog metalog;
	metalog.start = START;
	// filename1
	if(pathname[0] == '/')
		sprintf(metalog.filename1, "%s", pathname);
	else
	{
		getPath(metalog.filename1);
		int pwd_len = strlen(metalog.filename1);
		if(pwd_len != 1)
			metalog.filename1[pwd_len++] = '/';
		sprintf(metalog.filename1 + pwd_len, "%s", pathname);
	}

	metalog.filename2[0] = '\0';
	sprintf(metalog.operation, "unlink");
	metalog.startpos = 0;
	metalog.endpos = 0;
	metalog.mode = 0;
	metalog.end = END;
	metalog.checkpoint = CHECKPOINT;

	__asm__ __volatile__ ("int $0x80":"=a"(res):"a"(4),"b"(metafile),"c"((char*)&metalog),"d"(sizeof(struct MetaLog) - sizeof(int)));
//	printf("DEBUG!!! 3");
	if(res < 0)
	{
		close(metafile);
		return -1;
	}

	__asm__ volatile ("int $0x80":"=a"(res):"a"(10),"b"(pathname));
	if ( res >= 0 )
	{
		// checkpoint
		int chk_res;
		__asm__ __volatile__ ("int $0x80":"=a"(chk_res):"a"(4),"b"(metafile),"c"((char*)(&metalog) + (sizeof(struct MetaLog) - sizeof(int))),"d"(sizeof(int)));
		close(metafile);
		return res;
	}
	close(metafile);
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


