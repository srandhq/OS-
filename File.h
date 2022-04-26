#pragma once
#define BNUM 1024     //���̿���Ŀ
#define blocksize 64 //ÿ���̿��С32Byte
#define MAXUSER 10    //����û���
#define MAXUFILE 20   //ÿ���û�ӵ�е�����ļ���
#define MAXOPEN 20    //��ͬʱ�򿪵��ļ������Ŀ
#define buffersize 1024
/****** ������ *******/
typedef struct GuideBlock {
	int fsize;    //�ļ��ܿ���
	int sstart;   //��������ʼ��
	int istart;   //inode����ʼ��
	int fstart;   //һ��洢����ʼ��
}guide;

/****** fat *******/
typedef struct FAT
{
	int nextblock;	//��һ�������ĵ�ַ
	int useflag;	//λʾͼ useflag��0 �����Ϊ�գ�useflag��1 ����鲻��
}FAT;

/****** inode�ļ����ƿ� *********/
typedef struct inode
{
	char name[20];  //�ļ��л��ļ���
	int  kind;	    //0�ļ��У�1�ļ�
	int  i_mode;	//0ֻ����1��д��2��ִ��
	int  i_size;    //��kind=0�����ʾ�ļ����е��ļ���Ŀ����kind=1�����ʾ�ļ����ȣ����ֽ�Ϊ��λ
	int  i_addr;    //���inode��
	int  f_addr;    //�ļ��洢�׵�ַ
	int  f_posblock;//�ļ���λ��ָ��
	int  f_posoff;  //�ļ�����ƫ��ָ��
}inode;

/******* ufd�û��ļ�Ŀ¼ *********/
typedef struct UFD
{
	char name[20];   //�ļ���
	int  iaddr;      //�ļ����i�ڵ��
}UFD;

/******** mfd�û����ļ�Ŀ¼ **********/
typedef struct MFD
{
	char username[20];    //�û���
	char password[20];    //����
	UFD  ufd[MAXUFILE];   //�û��ļ�Ŀ¼
	int  iaddr;           //�û��ļ���inode
}MFD;

/****** ������ *******/
typedef struct SuperBlock
{
	FAT fatfile[BNUM];   //fat��
	MFD mfd[MAXUSER];    //���û�Ŀ¼
	int currentuser;     //��ǰ�û�
	inode currentdir;    //�û��ļ���inode
}surper;
