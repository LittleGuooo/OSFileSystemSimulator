#include<vector>
#include<string>
#include<iostream>
#include <iomanip>

#include "DataStruct.h"

using namespace std;

// �ⲿ����ȫ�ֱ���
extern vector<UCB*> users;  //�����û�
extern UCB* admin;  //����Ա
extern UCB* guest;	//�ο�
extern UCB* curUserPtr;    //��ǰ�û�
extern FCB* curDirPtr;    //��ǰ�û�
extern FCB* preDirPtr;		//�ϼ�Ŀ¼
extern int ID; //�û�id
extern int fileID;	//�ļ�ID
extern OFT* openFileTable;	//ϵͳ���ļ���
extern bool isSuperUserDo;	//����Ա��ʱ��¼
extern DFBM* dist_free_block_manager;   //��������������
extern MFD* master_file_directory;  //���ļ�Ŀ¼

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
	cout << setiosflags(ios_base::internal) << setw(60) << "�� �롰 man ���� �� �� �� �� �� \n";
	cout << endl;
	cout << endl;
}

// �û��ֲ�
void man()
{
	cout <<
		R"(
sudo        		��ʱ�л�����Ա����	login       			�û���¼
logout      		�û�ע��		addUser     			����û�
delUser[�û���]    	ɾ���û�		ls[-l / -of]     		��ʾĿ¼
mkdir[Ŀ¼��]  		����Ŀ¼            	touch[�ļ���]  			�����ļ�
rmdir[Ŀ¼��]  		ɾ��Ŀ¼                rm[�ļ���]   			ɾ���ļ�
man         		�û��ֲ�                open[�ļ���]  			���ļ�
close[�ļ���]  		�ر��ļ�                cat[�ļ���]  			���ļ�
vi[�ļ���]   		д�ļ�			chmod[�ļ���][��Ȩ��]		�޸��ļ�Ȩ��
pwd         		��ʾ��ǰ·��		whoami      			��ʾ��ǰ�û�
df[-a]       		��ʾ����ʹ�����	du[�ļ���]   			��ʾ�ļ�����ʹ�����
quit			�˳�����)"
<< endl;
}


// ��ʾ���������
void showDisk(string argument) {
	cout << setw(15) << "�����������" << dist_free_block_manager->totalBlockNum <<
		" �������������" << dist_free_block_manager->freeBlockNum <<
		" ʹ�����������" << dist_free_block_manager->usedBlockNUm << endl;

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

// ��ʾ�ļ�ʹ�õ������
void showFileBlocks(string fileName) {
	// �����ļ��Ƿ����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// ����ļ�������
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�������\n";
		return;
	}

	// �����ļ�����鲢��ʾ
	cout << "�ļ� " << (*it)->fileName << " �����ʹ�ã�" << endl;
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


// ��ʾ��ǰ·��
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
	// ��ʾ��ǰ�û�
	cout << curUserPtr->userName << "\n";
}

// ��ʾĿ¼��ϸ��
void dirDetails(vector<FCB*>& entries) {
	cout << "����"
		<< "Ȩ��" << " "
		<< setiosflags(ios::left) << setw(10) << "������" << " "
		<< setiosflags(ios::left) << setw(7) << "�ļ���С" << " "
		<< setiosflags(ios::left) << setw(12) << "�����ַ" << " "
		<< setiosflags(ios::left) << setw(18) << "����ʱ��" << " "
		<< setiosflags(ios::left) << setw(11) << "�ļ���" << endl;
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

// ��ʾĿ¼��
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
		cout << "���󣡲�������\n";
	}
}
