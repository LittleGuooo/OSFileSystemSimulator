#include<vector>
#include<string>
#include<iostream>
#include <iomanip>
#include <sstream>

#include "DataStruct.h"

using namespace std;

// 外部声明全局变量
extern vector<UCB*> users;  //所有用户
extern UCB* admin;  //管理员
extern UCB* guest;	//游客
extern UCB* curUserPtr;	//当前用户
extern FCB* curDirPtr;	//当前用户
extern FCB* preDirPtr;	//上级目录
extern int userID; //用户id
extern int fileID;	//文件ID
extern bool isSuperUserDo;	//管理员暂时登录
extern DFBM* dist_free_block_manager;   //空闲物理块管理器
extern MFD* master_file_directory;  //主文件目录

// 分配物理块
void allocateFreeBlocks(int length, FCB& newFile) {
	// 空闲磁盘块不足
	if (dist_free_block_manager->freeBlockNum < length)
	{
		cout << "错误！空闲磁盘块不足\n";
		return;
	}

	int n = length;
	for (DB* block : dist_free_block_manager->distBlocks) {
		// 如果物理块未被使用
		if (!block->useFlag) {
			block->useFlag = true;
			n--;

			// 链表头插法
			if (newFile.start == nullptr) {
				newFile.start = block;
				newFile.end = block;
			}
			else {
				newFile.end->next = block;
				newFile.end = block;
			}

			// 是否分配完毕
			if (n == 0) {
				break;
			}
		}
	}

	// 更新空闲磁盘块信息
	dist_free_block_manager->freeBlockNum -= length;
	dist_free_block_manager->usedBlockNUm += length;
}

// 回收物理块
void recycleBlocks(FCB& fileToDelete) {
	DB* tmpDiskBlock;
	DB* curDiskBlock = fileToDelete.start;
	int n = 0;

	while (curDiskBlock != fileToDelete.end) {
		tmpDiskBlock = curDiskBlock;
		curDiskBlock = curDiskBlock->next;
		tmpDiskBlock->useFlag = 0;

		n++;
	}

	// 更新空闲磁盘块信息
	dist_free_block_manager->freeBlockNum += n;
	dist_free_block_manager->usedBlockNUm -= n;
}
