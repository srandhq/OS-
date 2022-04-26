#pragma once
#define BNUM 1024     //总盘块数目
#define blocksize 64 //每个盘快大小32Byte
#define MAXUSER 10    //最大用户数
#define MAXUFILE 20   //每个用户拥有的最大文件数
#define MAXOPEN 20    //可同时打开的文件最大数目
#define buffersize 1024
/****** 引导块 *******/
typedef struct GuideBlock {
	int fsize;    //文件总块数
	int sstart;   //超级快起始块
	int istart;   //inode区起始块
	int fstart;   //一般存储区起始块
}guide;

/****** fat *******/
typedef struct FAT
{
	int nextblock;	//下一个物理块的地址
	int useflag;	//位示图 useflag＝0 物理块为空，useflag＝1 物理块不空
}FAT;

/****** inode文件控制块 *********/
typedef struct inode
{
	char name[20];  //文件夹或文件名
	int  kind;	    //0文件夹；1文件
	int  i_mode;	//0只读；1读写；2可执行
	int  i_size;    //若kind=0，则表示文件夹中的文件数目，若kind=1，则表示文件长度，以字节为单位
	int  i_addr;    //外存inode号
	int  f_addr;    //文件存储首地址
	int  f_posblock;//文件块位置指针
	int  f_posoff;  //文件块内偏移指针
}inode;

/******* ufd用户文件目录 *********/
typedef struct UFD
{
	char name[20];   //文件名
	int  iaddr;      //文件外存i节点号
}UFD;

/******** mfd用户主文件目录 **********/
typedef struct MFD
{
	char username[20];    //用户名
	char password[20];    //密码
	UFD  ufd[MAXUFILE];   //用户文件目录
	int  iaddr;           //用户文件夹inode
}MFD;

/****** 超级块 *******/
typedef struct SuperBlock
{
	FAT fatfile[BNUM];   //fat表
	MFD mfd[MAXUSER];    //主用户目录
	int currentuser;     //当前用户
	inode currentdir;    //用户文件夹inode
}surper;
