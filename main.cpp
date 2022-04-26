#include <iostream>
#include <fstream>
#include <math.h> 
#include"File.h"
using namespace std;

//全局变量
int order=1;
guide g;  //引导块
inode fcb[MAXOPEN]; //内存inode表，即打开文件表
surper s; //超级块
fstream iofile;

int applyinodememory() {//申请1块inode空间，成功则返回空闲块在fat表中的相对块号，否则返回-1
	int i;
	for (i = g.istart; i < g.fstart; i++) {//从inode区到数据区间找1块空闲块
		if (s.fatfile[i].useflag == 0) {//第i块没有被占用
			s.fatfile[i].useflag = 1;
			return i;
		}
	}
	return -1;
}

int applyfilememory(int bnum) {//申请bnum块文件存储空间，成功则返回空闲块在fat表中的相对块号，否则返回-1
	int i, j, k = 0, b;
	for (i = g.fstart, j = 0; i < g.fsize && j < bnum; i++) {//从数据区到存储区间末尾，找bnum块空闲块
		if (s.fatfile[i].useflag == 0) {//第i块没有被占用
			j++;                   //找到一块空闲块
			s.fatfile[i].useflag = 1;
			if (j != 1) {              //如果不是找到的第一块，那么找到的上一块的nextblock指向这一块
				s.fatfile[k].nextblock = i;
			}
			else if (j == 1) b = i;       //如果是找到的第一块，记录下该块的相对块号
			k = i;
		}
	}
	if (j = bnum) return b;             //找足bnum块则返回第一块的相对块号
	else {                           //没有找足bnum块则恢复找到的空闲块为没有被使用的块，并返回-1
		for (i = 1; i <= j; i++) {
			s.fatfile[b].useflag = 0;
			b = s.fatfile[b].nextblock;
		}
		return -1;
	}
}

void initblock() {    //为各个文件块赋初值    
	char* buf;
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary); //在当前文件夹下建立一个输入输出二进制文件
	buf = (char*)malloc(blocksize * BNUM);	//申请0.25M空间 
	if (buf == NULL)					//申请不成功，返回
	{
		printf("文件系统创建不成功!\n");
		return;
	}
	// 申请成功，把其空间写入MyDisk.img，使MyDisk.img为0.25M
	iofile.seekg(0, ios::beg);
	iofile.write(buf, blocksize * BNUM);
	free(buf);

	//初始化引导块
	g.fsize = 1024;  //文件总块数
	g.sstart = 1;    //超级快起始块
	g.istart = 107;    //inode起始块
	g.fstart = 307;  //一般存储区起始块
	iofile.seekg(0, ios::beg);
	iofile.write((char*)&g, sizeof(g));

	//初始化超级块1~62盘块
	for (int i = 0; i < BNUM; i++)
	{
		s.fatfile[i].nextblock = -1;
		s.fatfile[i].useflag = 0;
	}
	s.fatfile[0].useflag = s.fatfile[1].useflag = 1;
	//目录文件初始化
	for (int i = 0; i < MAXUSER; i++)
	{
		s.mfd[i].iaddr = -1;
		s.mfd[i].username[0] = '\0';
		for (int k = 0; k < MAXUFILE; k++)
		{
			s.mfd[i].ufd[k].name[0] = '\0';
			s.mfd[i].ufd[k].iaddr = -1;
		}
	}
	for (int j = 0; j < MAXOPEN; j++) {
		fcb[j].name[0] = '\0';
	}
	s.currentuser = -1;   //当前无用户
	s.currentdir.i_addr = -1;
	s.currentdir.kind = 1;
	s.currentdir.i_size = 0;
	s.currentdir.name[0] = '\0';
	iofile.seekg(blocksize, ios::beg);
	iofile.write((char*)&s, sizeof(s));
	iofile.close();
}

void initialize()   //初始化整个二级文件系统
{
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
	iofile.seekg(0, ios::beg);
	iofile.read((char*)&g, sizeof(g));
	iofile.seekg(blocksize, ios::beg);
	iofile.read((char*)&s, sizeof(s));
	iofile.close();
	for (int i = 0; i < MAXOPEN; i++)  //初始化打开文件表
	{
		fcb[i].name[0] = '\0';
		fcb[i].i_size = 0;
		fcb[i].i_addr = -1;
		fcb[i].kind = 1;
		fcb[i].i_mode = 0;
		fcb[i].f_addr = -1;
		fcb[i].f_posblock = -1;
		fcb[i].f_posoff = 0;
	}
	cout << "初始化成功..." << endl;

}

void refreshblock() {  //把修改后的各块写回文件
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
	iofile.seekg(0, ios::beg);
	iofile.write((char*)&g, sizeof(g));
	iofile.seekg(blocksize, ios::beg);
	iofile.write((char*)&s, sizeof(s));
	iofile.close();
}

int printuser() {          //列出所有用户
	if (s.mfd[0].username[0] == '\0') {
		cout << "没有用户，请创建！" << endl;
		return -1;
	}
	cout << "所有用户如下：" << endl;
	for (int i = 0; s.mfd[i].username[0] != '\0'; i++) {
		cout << s.mfd[i].username << "    ";
		if (i % 10 == 0) cout << endl;
	}
	cout << endl;
	return 1;
}

void login() {     //用户登录
	char name[20];
	char password[20];
	int flag = 1;
	if (printuser() == -1) return;
L:	cout << "用户名：";
	cin >> name;
	for (int i = 0; s.mfd[i].username[0] != '\0'; i++) {
		if (strcmp(s.mfd[i].username, name) == 0) {
			cout << "密码：";
			cin >> password;
			if (strcmp(s.mfd[i].password, password) == 0) {
				s.currentuser = i;
				iofile.open("MyDisk.img", ios::in | ios::out | ios::binary);
				int k = s.mfd[s.currentuser].iaddr;
				iofile.seekg(k * blocksize, ios::beg);
				iofile.read((char*)&s.currentdir, sizeof(inode));
				iofile.close();
				flag = 0;
				cout << "登陆成功！" << endl;
			}
			else {
				cout << "登录失败,密码错误！" << endl;
				goto L;
				return;
			}
		}
	}
	if (flag == 1)
		cout << "该用户不存在" << endl;

}

void newuser() {     //创建新用户
	char name[20], password[10], password2[10];
C:	cout << "创建新用户：" << endl << "用户名：";
	cin >> name;
	cout << "密码(不得超过10位)：";
	cin >> password;
	cout << "密码确认：";
	cin >> password2;
	for (int i = 0; i < MAXUSER; i++)  //检查用户名是否重复
	{
		if (strcmp(s.mfd[i].username, name) == 0)
		{
			cout << "该用户已存在！" << endl;
			return;
		}
	}
	if (strcmp(password, password2) == 0) {  //密码输入正确
		int iaddr;
		int i = 0;
		for (i = 0; s.mfd[i].username[0] != '\0' && i < MAXUSER; i++);
		if (i == MAXUSER)
		{
			cout << "系统中用户数已达上限，无法创建新用户" << endl;
			return;
		}
		iaddr = s.mfd[i].iaddr = applyinodememory();
		if (iaddr == -1) {
			cout << "inode空间不足无法创建！" << endl;
			return;
		}
		strcpy_s(s.mfd[i].username, name);
		strcpy_s(s.mfd[i].password, password);
		inode in;
		strcpy_s(in.name, name);
		in.kind = 0;
		in.i_mode = 1;
		in.i_size = 0;
		in.i_addr = iaddr;
		iofile.open("MyDisk.img", ios::in | ios::out | ios::binary);
		iofile.seekg(iaddr * blocksize, ios::beg);
		iofile.write((char*)&in, sizeof(inode));
		iofile.close();
		cout << "创建成功！" << endl;
	}
	else {
		cout << "两次输入的密码不一致！请重新输入！" << endl;
		goto C;
	}
	refreshblock();
}

int ls() {   //列出该文件夹下的所有文件
	if (s.mfd[s.currentuser].username[0] == '\0') {
		cout << "请先登录！" << endl;
		return -1;
	}
	cout << "文件夹：" << s.mfd[s.currentuser].username << endl;
	if (s.mfd[s.currentuser].ufd[0].name[0] == '\0') {
		cout << "没有文件，请创建！" << endl;
		return -1;
	}
	for (int i = 0; i < MAXUFILE; i++)
	{
		if (s.mfd[s.currentuser].ufd[i].name[0] != '\0')
			cout << s.mfd[s.currentuser].ufd[i].name << endl;
	}
	return 1;
}

void flseek(int fd, int position)
{
	int block, off;
	block = fcb[fd].f_addr;
	off = position;
	if (fcb[fd].i_size == 0)
	{
		cout << "文件为空无法定位" << endl;
		return;
	}
	if (fcb[fd].i_size < position)
	{
		cout << "文件长度不足，定位失败" << endl;
		return;
	}
	while (off > blocksize)
	{
		block = s.fatfile[block].nextblock;
		off -= blocksize;
	}
	fcb[fd].f_posblock = block; //定位
	fcb[fd].f_posoff = off;
	cout << "定位成功" << endl;
}

int fcreat(char* name, int mode) {  //创建一个文件
	int i, j;
	int iaddr = applyinodememory();
	if (iaddr == -1) {
		cout << "创建文件i节点失败！" << endl;
		return -1;
	}
	i = 0;
	while (s.mfd[s.currentuser].ufd[i].name[0] != '\0')
		i++;
	if (i == MAXUFILE)
	{
		cout << "该用户文件数已达上限，无法创建！" << endl;
		return -1;
	}
	for (j = 0; j < i; j++)
	{
		if (strcmp(s.mfd[s.currentuser].ufd[j].name, name) == 0)
		{
			cout << "文件名重复 请重新建立！" << endl;
			return -1;
		}

	}
	s.mfd[s.currentuser].ufd[i].iaddr = iaddr;
	strcpy_s(s.mfd[s.currentuser].ufd[i].name, name);
	s.currentdir.i_size++;
	refreshblock();
	inode t;
	t.i_mode = mode;
	t.i_size = 0;
	t.kind = 1;
	t.i_addr = iaddr;
	t.f_addr = -1;
	t.f_posblock = -1;
	t.f_posoff = 0;
	strcpy_s(t.name, name);
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
	iofile.seekg(iaddr * blocksize, ios::beg);
	iofile.write((char*)&t, sizeof(inode));
	iofile.close();
	return iaddr;
}

int fdelete(char* name) //删除一个文件
{
	int k;
	for (int i = 0; i < MAXUFILE; i++)
	{
		if (strcmp(s.mfd[s.currentuser].ufd[i].name, name) == 0)  //找到要删除的文件
		{
			int j = s.mfd[s.currentuser].ufd[i].iaddr;
			inode in;                                     //通过文件i节点找到文件存储位置
			iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
			iofile.seekg(j * blocksize, ios::beg);
			iofile.read((char*)&in, sizeof(inode));
			iofile.close();
			for (k = in.f_addr; k != -1 && k < g.fsize;) {
				s.fatfile[k].useflag = 0;
				int e = s.fatfile[k].nextblock;
				s.fatfile[k].nextblock = -1;
				k = e;
			}
			for (k = j; k != -1;) {                          //释放外存inode
				s.fatfile[k].useflag = 0;
				int t = s.fatfile[k].nextblock;
				s.fatfile[k].nextblock = -1;
				k = t;
			}
			for (k = i; k < (MAXUFILE - 1) && s.mfd[s.currentuser].ufd[k + 1].name[0] != '\0';) {//删除对应的ufd
				s.mfd[s.currentuser].ufd[k].iaddr = s.mfd[s.currentuser].ufd[k + 1].iaddr;
				strcpy_s(s.mfd[s.currentuser].ufd[k].name, s.mfd[s.currentuser].ufd[k + 1].name);
			}
			s.mfd[s.currentuser].ufd[k].iaddr = -1;
			s.mfd[s.currentuser].ufd[k].name[0] = '\0';
			s.currentdir.i_size--;
			int n = 0;
			while (n < MAXOPEN) //判断文件是否已打开
			{
				if (strcmp(fcb[n].name, name) != 0)
					n++;
				else break;
			}
			if (n < MAXOPEN)   //从已打开列表中删除
			{
				for (k = n; k < MAXOPEN - 1 && fcb[k + 1].name[0] != '\0'; k++) {
					strcpy_s(fcb[k].name, fcb[k + 1].name);
					fcb[k].i_addr = fcb[k + 1].i_addr;
					fcb[k].i_mode = fcb[k + 1].i_mode;
					fcb[k].i_size = fcb[k + 1].i_size;
					fcb[k].kind = fcb[k + 1].kind;
					fcb[k].f_addr = fcb[k + 1].f_addr;
					fcb[k].f_posblock = fcb[k + 1].f_posblock;
					fcb[k].f_posoff = fcb[k + 1].f_posoff;
				}
				fcb[k].name[0] = '\0';
				fcb[k].i_size = 0;
				fcb[k].i_addr = -1;
				fcb[k].kind = 1;
				fcb[k].i_mode = 0;
				fcb[k].f_addr = -1;
				fcb[k].f_posblock = -1;
				fcb[k].f_posoff = 0;
			}
			for (k = i; k < MAXUFILE - 1 && s.mfd[s.currentuser].ufd[k + 1].iaddr != -1; k++) { //整理目录文件
				strcpy_s(s.mfd[s.currentuser].ufd[k].name, s.mfd[s.currentuser].ufd[k + 1].name);
				s.mfd[s.currentuser].ufd[k].iaddr = s.mfd[s.currentuser].ufd[k + 1].iaddr;
			}
			s.mfd[s.currentuser].ufd[k].name[0] = '\0';
			s.mfd[s.currentuser].ufd[k].iaddr = -1;
			refreshblock();
			cout << "删除成功！" << endl;
			return 1;
		}

	}
	cout << "没有找到该文件，删除失败!" << endl;
	return -1;
}

int fwrite(int fd, char* buffer, int length)  //fd打开文件号，buf为信息源区首地址，length为写字节数
{
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
	int block, blockoff, k, r;
	if (fcb[fd].i_mode == 0)
	{
		cout << "文件为只读，不可修改" << endl;
		return -1;
	}
	if (fcb[fd].i_size == 0)   //文件为空
	{
		fcb[fd].f_addr = applyfilememory(1);
		fcb[fd].f_posblock = fcb[fd].f_addr;
		fcb[fd].f_posoff = 0;
		if (fcb[fd].f_addr == -1)
			return -1;
	}
	block = fcb[fd].f_posblock;   //读写指针所在盘块
	blockoff = fcb[fd].f_posoff;  //读写指针在盘块内偏移量
	int rest = length;
	int i = fcb[fd].i_addr;
	if (length <= blocksize - blockoff)  //要写内容仅在一个盘块的可写范围内
	{
		iofile.seekg(block * blocksize + blockoff, ios::beg);
		iofile.write(buffer, length);
		fcb[fd].i_size += length;
		k = fcb[fd].i_addr;
		iofile.seekg(k * blocksize, ios::beg);
		iofile.write((char*)&fcb[fd], sizeof(inode));
		iofile.close();
		return length;
	}
	else {
		iofile.seekg(block * blocksize + blockoff, ios::beg);
		r = blocksize - blockoff;
		iofile.write(buffer, r);
		rest -= r;
	}
	int blockneed = rest / blocksize;
	if (rest % blocksize > 0) blockneed++;
	while (blockneed > 0 && s.fatfile[block].nextblock != -1) blockneed--;
	if (blockneed > 0) {
		s.fatfile[block].nextblock = applyfilememory(blockneed);
		if (s.fatfile[block].nextblock == -1) {
			iofile.close();
			return r;
		}
	}
	int rb;
	k = s.fatfile[block].nextblock;
	for (rb = 0; rb < rest / blocksize; rb++) {
		iofile.seekg(k * blocksize, ios::beg);
		iofile.write((char*)&buffer[r + rb * blocksize], blocksize);
		rest -= blocksize;
		k = s.fatfile[k].nextblock;
	}
	if (rest > 0 && k != -1) {
		iofile.seekg(k * blocksize, ios::beg);
		iofile.write((char*)&buffer[r + rb * blocksize], rest);
	}
	rest = 0;
	fcb[fd].i_size += length;
	k = fcb[fd].i_addr;
	iofile.seekg(k * blocksize, ios::beg);
	iofile.write((char*)&fcb[fd], sizeof(inode));
	iofile.close();
	refreshblock();
	return length;
}

int fread(int fd, char* buffer, int length)   //fd打开文件号(文件inode号)，buf为读得信息应送向的目标区首地址，length为读字节数
{
	for (int count = 0; count < buffersize; count++) {
		buffer[count] = '\0';
	}
	int block, blockoff;
	int rest = length;
	block = fcb[fd].f_posblock;   //读写指针所在盘块
	blockoff = fcb[fd].f_posoff;  //读写指针在盘块内偏移量
	if (fcb[fd].i_size == 0) {//文件长度为0
		return 0;
	}
	if (length > (fcb[fd].i_size - blockoff)) {
		length = fcb[fd].i_size - blockoff;
		return fread(fd, buffer, length);
	}
	else
	{
		iofile.open("MyDisk.img", ios::in | ios::out | ios::binary);
		int r, rb, k;
		if (length <= blocksize - blockoff) {  //要写内容仅在一个盘块的可写范围内
			iofile.seekg(block * blocksize + blockoff, ios::beg);
			iofile.read(buffer, length);
			buffer[length] = '\0';
			iofile.close();
			return length;
		}
		else {
			r = blocksize - blockoff;
			iofile.seekg(block * blocksize + blockoff, ios::beg);
			iofile.read(buffer, r);
			rest -= r;
		}
		k = s.fatfile[block].nextblock;
		for (rb = 0; rb < rest / blocksize; rb++) {
			iofile.seekg(k * blocksize, ios::beg);
			iofile.read((char*)&buffer[r + rb * blocksize], blocksize);
			rest -= blocksize;
			k = s.fatfile[k].nextblock;
		}
		if (rest > 0 && k != -1) {
			iofile.seekg(k * blocksize, ios::beg);
			iofile.read((char*)&buffer[r + rb * blocksize], rest);
		}
		rest = 0;
		iofile.close();
		buffer[length] = '\0';
		return length;
	}
}

int  fopen(char* name, int mode)
{
	if (fcb[MAXOPEN - 1].name[0] != '\0') {
		cout << "用户打开文件数已满！" << endl;
		return -1;
	}
	int i = 0;
	while (i < MAXOPEN && fcb[i].name[0] != '\0')  //是否已打开
	{
		if (strcmp(fcb[i].name, name) == 0)  //如果已经在打开表中
		{
			cout << "文件已经打开!" << endl;
			return i;
		}
		i++;
	}
	for (int j = 0; j < s.currentdir.i_size; j++)
	{
		if (strcmp(s.mfd[s.currentuser].ufd[j].name, name) == 0)   //文件存在
		{
			iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
			iofile.seekg(s.mfd[s.currentuser].ufd[j].iaddr * blocksize, ios::beg);
			iofile.read((char*)&fcb[i], sizeof(inode)); //把外存inode读入
			iofile.close();
			cout << "打开成功！" << endl;
			return i;
		}
	}
	//文件不存在
	cout << "没有该文件！" << endl;
	return -1;
}

void fclose(int fd)
{
	int k;
	fcb[fd].f_posblock = fcb[fd].f_addr;
	fcb[fd].f_posoff = 0;
	k = fcb[fd].i_addr;
	iofile.open("D:\\操作系统课设\\MyDisk.img", ios::in | ios::out | ios::binary);
	iofile.seekg(k * blocksize, ios::beg);
	iofile.write((char*)&fcb[fd], sizeof(inode)); //把外存inode读入
	iofile.close();
	fcb[fd].name[0] = '\0';
	int i = fd;
	for (; i < (MAXOPEN - 1) && fcb[i + 1].name[0] != '\0'; i++) {
		fcb[i].i_addr = fcb[i + 1].i_addr;
		fcb[i].i_mode = fcb[i + 1].i_mode;
		fcb[i].i_size = fcb[i + 1].i_size;
		fcb[i].kind = fcb[i + 1].kind;
		fcb[i].f_addr = fcb[i + 1].f_addr;
		fcb[i].f_posblock = fcb[i + 1].f_posblock;
		fcb[i].f_posoff = fcb[i + 1].f_posoff;
		strcpy_s(fcb[i].name, fcb[i + 1].name);
	}
	fcb[i].name[0] = '\0';
	cout << "关闭成功！" << endl;
}

int chooseopenfile() {
	char name[20];
	int i;
	if (fcb[0].name[0] == '\0') {
		cout << "没有已打开文件！" << endl;
		return -1;
	}
	cout << "已打开文件如下：" << endl;
	for (i = 0; i < MAXOPEN && fcb[i].name[0] != '\0'; i++) {
		cout << fcb[i].name << "   ";
	}
	cout << endl << "请输入要操作的文件名：";
	cin >> name;
	for (i = 0; i < MAXOPEN && fcb[i].name[0] != '\0'; i++) {
		if (strcmp(fcb[i].name, name) == 0)
			return i;
	}
	cout << "输入文件名不存在！" << endl;
	return -1;
}
void help() {
	cout << "***************************************************" << endl;
	cout << "请选择功能：" << endl;
	cout << "1.创建新用户(createuser)   2.列出所有用户(showuser)  3.用户登录(login)   4.创建文件(create)" << endl;
	cout << "5.打开文件(open)     6.删除文件(delete)      7.写文件(write)     8.读文件(read)  " << endl;
	cout << "9.列出所有文件(dir) 10.修改文件指针(update) 11.关闭文件(colsedfile)   12.帮助界面(help)    0.退出(exit)" << endl;
	cout << "****************************************************" << endl;
}

void testMyFileSystem() {
	initialize();
	string n ;
	int i = 0;
	char name[20], buffer[buffersize];
	int mode, fd, length, position;
	while (n.size()!=0) {		
			cin >> n;
			if (n == "create") {
				newuser();
			}
			if (n=="showuser") { 
				printuser(); 
			}
			if (n == "login") { 
				login();
			} 
			if (n == "create") {
				cout << "请输入文件名字和模式（0只读；1读写；2可执行）：" << endl;
				cin >> name >> mode;
				fcreat(name, mode);
			}
		case(5):if (ls() == -1) break;
			cout << "请输入文件名字和打开模式（0只读；1读写；2可执行）：" << endl;
			cin >> name >> mode;
			fopen(name, mode);
			break;
		case(6):if (ls() == -1) break;
			cout << "请输入文件名字：" << endl;
			cin >> name;
			fdelete(name);
			break;
		case(7):fd = chooseopenfile();
			if (fd == -1) break;
			cout << "请为buffer赋值！" << endl;
			cin >> buffer;
			cout << "请输入length值：" << endl;
			cin >> length;
			fwrite(fd, buffer, length);
			break;
		case(8):fd = chooseopenfile();
			if (fd == -1) break;
			cout << "请输入length值：" << endl;
			cin >> length;
			fread(fd, buffer, length);
			cout << buffer;
			cout << endl;
			break;
		case(9):ls(); break;
		case(10):fd = chooseopenfile();
			if (fd == -1) break;
			cout << "请输入position：";
			cin >> position;
			flseek(fd, position);
			break;
		case(11):fd = chooseopenfile();
			if (fd == -1) break;
			fclose(fd);
			break;
		case(0):return;
		}
	}
}

void main()
{
	initblock();
	testMyFileSystem();
}