#include<vector>
#include<string>
#include<iostream>
#include <iomanip>
#include <sstream>

#include "DataStruct.h"

using namespace std;

// �ⲿ����ȫ�ֱ���
extern vector<UCB*> users;  //�����û�
extern UCB* admin;  //����Ա
extern UCB* guest;	//�ο�
extern UCB* curUserPtr;	//��ǰ�û�
extern FCB* curDirPtr;	//��ǰ�û�
extern FCB* preDirPtr;	//�ϼ�Ŀ¼
extern int userID; //�û�id
extern int fileID;	//�ļ�ID
extern bool isSuperUserDo;	//����Ա��ʱ��¼
extern DFBM* dist_free_block_manager;   //��������������
extern MFD* master_file_directory;  //���ļ�Ŀ¼

// ���������
void allocateFreeBlocks(int length, FCB& newFile) {
	// ���д��̿鲻��
	if (dist_free_block_manager->freeBlockNum < length)
	{
		cout << "���󣡿��д��̿鲻��\n";
		return;
	}

	int n = length;
	for (DB* block : dist_free_block_manager->distBlocks) {
		// ��������δ��ʹ��
		if (!block->useFlag) {
			block->useFlag = true;
			n--;

			// ����ͷ�巨
			if (newFile.start == nullptr) {
				newFile.start = block;
				newFile.end = block;
			}
			else {
				newFile.end->next = block;
				newFile.end = block;
			}

			// �Ƿ�������
			if (n == 0) {
				break;
			}
		}
	}

	// ���¿��д��̿���Ϣ
	dist_free_block_manager->freeBlockNum -= length;
	dist_free_block_manager->usedBlockNUm += length;
}

// ���������
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

	// ���¿��д��̿���Ϣ
	dist_free_block_manager->freeBlockNum += n;
	dist_free_block_manager->usedBlockNUm -= n;
}
