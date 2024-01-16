#include<vector>
#include<string>
#include<iostream>
#include <iomanip>

#include "DataStruct.h"

using namespace std;

// 外部声明全局变量
extern vector<UCB*> users;  //所有用户
extern UCB* admin;  //管理员
extern UCB* guest;	//游客
extern UCB* curUserPtr;    //当前用户
extern FCB* curDirPtr;    //当前用户
extern FCB* preDirPtr;		//上级目录
extern int ID; //用户id
extern int fileID;	//文件ID
extern OFT* openFileTable;	//系统打开文件表
extern bool isSuperUserDo;	//管理员暂时登录
extern DFBM* dist_free_block_manager;   //空闲物理块管理器
extern MFD* master_file_directory;  //主文件目录

void printWelcomeMessage() {
	cout <<
		R"(

   _____           __   __       ______ _ _       _____           _                 
  / ____|          \ \ / /      |  ____(_| |     / ____|         | |                
 | |  __ _   _  ___ \ V /_   _  | |__   _| | ___| (___  _   _ ___| |_ ___ _ __ ___  
 | | |_ | | | |/ _ \ > <| | | | |  __| | | |/ _ \\___ \| | | / __| __/ _ | '_ ` _ \ 
 | |__| | |_| | (_) / . | |_| | | |    | | |  __/____) | |_| \__ | ||  __| | | | | |
  \_____|\__,_|\___/_/ \_\__,_| |_|    |_|_|\___|_____/ \__, |___/\__\___|_| |_| |_|
                                                         __/ |                      
                                                        |___/                       

)"
<< endl;
	cout << setiosflags(ios_base::internal) << setw(60) << "输 入“ man ”查 看 命 令 手 册 \n";
	cout << endl;
	cout << endl;
}

// 用户手册
void man()
{
	cout <<
		R"(
sudo        		暂时切换管理员操作	login       			用户登录
logout      		用户注销		addUser     			添加用户
delUser[用户名]    	删除用户		ls[-l / -of]     		显示目录
mkdir[目录名]  		创建目录            	touch[文件名]  			创建文件
rmdir[目录名]  		删除目录                rm[文件名]   			删除文件
man         		用户手册                open[文件名]  			打开文件
close[文件名]  		关闭文件                cat[文件名]  			读文件
vi[文件名]   		写文件			chmod[文件名][新权限]		修改文件权限
pwd         		显示当前路径		whoami      			显示当前用户
df[-a]       		显示磁盘使用情况	du[文件名]   			显示文件磁盘使用情况
quit			退出程序)"
<< endl;
}


// 显示空闲物理块
void showDisk(string argument) {
	cout << setw(15) << "总物理块数：" << dist_free_block_manager->totalBlockNum <<
		" 空闲物理块数：" << dist_free_block_manager->freeBlockNum <<
		" 使用物理块数：" << dist_free_block_manager->usedBlockNUm << endl;

	int displayNum = 300;
	if (argument == "-a")
	{
		displayNum = dist_free_block_manager->totalBlockNum;
	}

	int blockCount = 0;
	for (int i = 0; i < displayNum; ++i) {
		cout << dist_free_block_manager->distBlocks[i]->useFlag << " ";
		++blockCount;
		if (blockCount % 30 == 0) {
			cout << "   " << i - 29 << " ~ " << i << endl;
		}
	}
	cout << endl;
}

// 显示文件使用的物理块
void showFileBlocks(string fileName) {
	// 查找文件是否存在
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// 如果文件不存在
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件不存在\n";
		return;
	}

	// 遍历文件物理块并显示
	cout << "文件 " << (*it)->fileName << " 物理块使用：" << endl;
	int blockCount = 0;
	DB* currentBlock = (*it)->start;
	while (currentBlock != nullptr) {
		cout << " -> " << currentBlock->start;
		++blockCount;

		if (blockCount % 10 == 0) {
			cout << endl;
		}
		currentBlock = currentBlock->next;
	}
}


// 显示当前路径
void pwd() {
	FileControlBlock* currentDir = curDirPtr;
	string path = "";

	while (currentDir != nullptr) {
		path = "/" + currentDir->fileName + path;
		currentDir = currentDir->parentDir;
	}

	cout << path << "\n";
}

void whoami() {
	// 显示当前用户
	cout << curUserPtr->userName << "\n";
}

// 显示目录项细节
void dirDetails(vector<FCB*>& entries) {
	cout << "类型"
		<< "权限" << " "
		<< setiosflags(ios::left) << setw(10) << "所有者" << " "
		<< setiosflags(ios::left) << setw(7) << "文件大小" << " "
		<< setiosflags(ios::left) << setw(12) << "物理地址" << " "
		<< setiosflags(ios::left) << setw(18) << "创建时间" << " "
		<< setiosflags(ios::left) << setw(11) << "文件名" << endl;
	for (FCB* entry : entries) {
		cout << entry->fileType << " "
			<< entry->fileAcess << " "
			<< setiosflags(ios::left) << setw(11) << entry->ownerName << " "
			<< setiosflags(ios::left) << setw(7) << entry->contentLength << " "
			<< setiosflags(ios::left) << setw(7) << entry->start->start * 512 << " "
			<< setiosflags(ios::left) << setw(7) << put_time(entry->timeinfo, "%Y-%m-%d %H:%M:%S") << " "
			<< setiosflags(ios::left) << setw(11) << entry->fileName << endl;
	}
}

// 显示目录项
void dir(FCB& directory, string argument) {

	if (argument == "") {
		int count = 0;
		for (FCB* entry : directory.fd->directoryEntries) {
			cout << setw(11) << setiosflags(ios_base::left) << entry->fileName ;
			count++;
			if (count >= 7) {
				cout << endl;
				count = 0;
			}
		}
		cout << endl;
	}
	else if (argument == "-l")
	{
		dirDetails(directory.fd->directoryEntries);
	}
	else if (argument == "-of")
	{
		dirDetails(openFileTable->fileDescriptors);
	}
	else
	{
		cout << "错误！参数错误\n";
	}
}
