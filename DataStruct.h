#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <ctype.h>
#include <chrono>
#include <fstream>
#include <cstdio>

#define DISK_LENGTH (1024 * 1024)
#define DISK_BLOCK_LENGTH 512
#define DISK_BLOCK_NUM (DISK_LENGTH / 512)
#define MAX_FILENAME_LENGTH 10
#define MAX_USERNAME_LENGTH 10
#define MAX_USER_NUM 10
#define DIR_DEFAULT_LENTH 512
#define FILE_DEFAULT_LENTH 512

using namespace std;

// 防止结构体相互引用
struct DistBlock;
struct FileControlBlock;
struct FileDirectory;
struct UserControlBlock;
struct DistFreeBlockManager;
struct MasterFileDirectory;
struct OpenFileTable;

// 物理块
struct DistBlock
{
	int maxLength;    // 容量
	int start;        // 起始地址
	bool useFlag;     // 是否被使用
	DistBlock* next;  // 指向下一个磁盘块的指针
};

// 空闲物理块管理器
struct DistFreeBlockManager
{
	int totalBlockNum;     // 总物理块数量
	int freeBlockNum;      // 空闲物理块数量
	int usedBlockNUm;      // 使用物理块数量
	vector<DistBlock*> distBlocks;  // 所有物理块
};

// 目录文件
struct FileDirectory
{
	vector<FileControlBlock*> directoryEntries;  // 目录项
};

// 打开文件表
struct OpenFileTable
{
	vector<FileControlBlock*> fileDescriptors;  // 文件描述符
};

// 用户控制块
struct UserControlBlock
{
	string userName;            // 用户账号
	string password;            // 用户密码
	int userID;                 // 用户id
	FileControlBlock* homeDirectory;  // 用户home目录
};

// 主文件目录
struct MasterFileDirectory
{
	FileControlBlock* rootDirectory;  // 根目录
};

// 文件控制块
struct FileControlBlock
{
	string fileName;          // 文件名
	int fileID;               // 文件ID
	char fileType;            // 文件类型
	string ownerName;         // 文件所有者姓名
	int ownerID;              // 文件所有者ID
	DistBlock* start;         // 文件在磁盘中的起始地址
	DistBlock* end;           // 文件在磁盘中的结尾地址
	int contentLength;        // 文件内容长度
	int length;               // 文件的最大长度
	string fileAcess;         // 所有者权限rwx其他用户权限rwx
	tm* timeinfo;             // 文件创建时间
	bool isOpen;              // 文件是否打开
	bool isDirectory;         // 是否为目录
	string realFileName;      // 真实文件名
	fstream* realFileStream;  // 真实文件流指针
	string content;           // 文件内容缓冲区
	FileControlBlock* parentDir;  // 父目录
	FileDirectory* fd;        // 如果是目录文件
};

// 比较名称的一元谓词
class EqualTargetFileName
{
private:
	string targetFileName;

public:
	EqualTargetFileName(string name) : targetFileName(name) {}

	bool operator()(FileControlBlock* file)
	{
		return file->fileName == targetFileName;
	}
};

class EqualTargetRealFileName
{
private:
	string targetRealFileName;

public:
	EqualTargetRealFileName(string name) : targetRealFileName(name) {}

	bool operator()(FileControlBlock* file)
	{
		return file->realFileName == targetRealFileName;
	}
};

class EqualTargetUserName
{
private:
	string targetUserName;

public:
	EqualTargetUserName(string name) : targetUserName(name) {}

	bool operator()(UserControlBlock* user)
	{
		return user->userName == targetUserName;
	}
};

typedef struct DistBlock DB;
typedef struct FileControlBlock FCB;
typedef struct FileDirectory FD;
typedef struct UserControlBlock UCB;
typedef struct DistFreeBlockManager DFBM;
typedef struct MasterFileDirectory MFD;
typedef struct OpenFileTable OFT;

#endif