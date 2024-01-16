#include<vector>
#include<string>
#include<iostream>
#include <iomanip>
#include<malloc.h>

#include "DataStruct.h"

using namespace std;

// �ⲿ����ȫ�ֱ���
extern vector<UCB*> users;  //�����û�
extern UCB* admin;  //����Ա
extern UCB* guest;	//�ο�
extern UCB* curUserPtr;    //��ǰ�û�
extern FCB* curDirPtr;    //��ǰ�û�
extern FCB* preDirPtr;		//�ϼ�Ŀ¼
extern int userID; //�û�id
extern int fileID;	//�ļ�ID
extern OFT* openFileTable;	//ϵͳ���ļ���
extern bool isSuperUserDo;	//����Ա��ʱ��¼
extern DFBM* dist_free_block_manager;   //��������������
extern MFD* master_file_directory;  //���ļ�Ŀ¼
extern int realFileNameCount;		//������ʵ���ļ���
extern int realDirNameCount;		//������ʵ��Ŀ¼��

void allocateFreeBlocks(int length, FCB& newFile);
void recycleBlocks(FCB& fileToDelete);

// �ļ��Ƿ��Ѵ�
bool isFileOpen(string fileName, vector<FCB* >::iterator& it) {
	// ����ļ��Ƿ��Ѿ���
	auto openFileIt = find_if(openFileTable->fileDescriptors.begin(),
		openFileTable->fileDescriptors.end(), EqualTargetFileName(fileName));

	if (openFileIt != openFileTable->fileDescriptors.end()) {
		it = openFileIt;
		return true;
	}

	return false;
}

// �����ļ�
void touch(UCB& fileOwner, FCB& parentDir, string fileName, int contentLength = FILE_DEFAULT_LENTH)
{
	// �ж��Ƿ��ļ�����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	if (it != curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�����\n";
		return;
	}

	// �ж��Ƿ����Լ���Ŀ¼��
	// �ж��Ƿ��ǹ���Ա����
	// �ж��Ƿ��ǹ���Ա��ʱ��¼
	if (fileOwner.userID == parentDir.ownerID || isSuperUserDo || curUserPtr->userID == 0)
	{
		// �����ļ����ƿ�
		FCB* newFile = new FCB;
		newFile->fileName = fileName;
		newFile->fileType = '-';
		newFile->ownerName = fileOwner.userName;
		newFile->ownerID = fileOwner.userID;
		newFile->contentLength = contentLength;
		newFile->start = nullptr;
		newFile->end = nullptr;
		newFile->length = newFile->contentLength / DISK_BLOCK_LENGTH;
		if (contentLength % DISK_BLOCK_LENGTH)
		{
			newFile->length++;
		}
		allocateFreeBlocks(newFile->length, *newFile);
		newFile->fileAcess = "rwxr--";
		auto currentTime = chrono::system_clock::now();
		time_t currentTimeT = chrono::system_clock::to_time_t(currentTime);
		//tm* timeinfo = localtime(&currentTimeT);	//Windows����ʹ�øð汾
		tm* timeinfo = new tm;
		localtime_s(timeinfo, &currentTimeT);
		newFile->timeinfo = timeinfo;
		newFile->isOpen = false;
		newFile->isDirectory = false;
		newFile->content;
		newFile->fd = nullptr;
		newFile->parentDir = &parentDir;	//���ø�Ŀ¼
		newFile->fileID = fileID++;		//�����ļ�ID
		newFile->realFileName = "file_" + to_string(realFileNameCount++) + ".txt";		//������ʵ���ļ���
		ofstream outfile(newFile->realFileName);		//������ʵ�ļ�
		if (outfile.is_open())
		{
			outfile.close();
		}
		else
		{
			cout << "������ʵ�ļ�ʧ��" << endl;
		}

		// �����ļ���ӵ���Ŀ¼��Ŀ¼����
		parentDir.fd->directoryEntries.push_back(newFile);
	}
	else
	{
		cout << "�����޲���Ȩ��\n";
	}
}

void mkdir(UCB& fileOwner, FCB& parentDir, string dirName)
{
	// �ж��Ƿ��ļ�����
	if (curDirPtr != nullptr)
	{
		auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
			curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(dirName));

		if (it != curDirPtr->fd->directoryEntries.end()) {
			cout << "�����ļ�����\n";
			return;
		}
	}

	// �ж��Ƿ����Լ���Ŀ¼��
	// �ж��Ƿ��ڴ�����Ŀ¼
	// �ж��Ƿ��ǹ���Ա����
	if (parentDir.fileID == -1 || fileOwner.userID == parentDir.ownerID || isSuperUserDo || curUserPtr->userID == 0)
	{
		// ����Ŀ¼�ļ����ƿ�
		FCB* newDir = new FCB;
		newDir->fileName = dirName;
		newDir->fileType = 'd';
		newDir->ownerName = fileOwner.userName;
		newDir->ownerID = fileOwner.userID;
		newDir->start = nullptr;
		newDir->end = nullptr;
		newDir->contentLength = DIR_DEFAULT_LENTH;
		newDir->length = newDir->contentLength / DISK_BLOCK_LENGTH;
		if (newDir->contentLength % DISK_BLOCK_LENGTH)
		{
			newDir->length++;
		}
		allocateFreeBlocks(newDir->length, *newDir);
		newDir->fileAcess = "rwxr--";
		auto currentTime = chrono::system_clock::now();
		time_t currentTimeT = chrono::system_clock::to_time_t(currentTime);
		//tm* timeinfo = localtime(&currentTimeT);	//Windows����ʹ�øð汾
		tm* timeinfo = new tm;
		localtime_s(timeinfo, &currentTimeT);
		newDir->timeinfo = timeinfo;
		newDir->isOpen = false;
		newDir->isDirectory = true;
		newDir->content;
		newDir->fd = new FD;  // ����Ŀ¼
		newDir->parentDir = &parentDir;	//���ø�Ŀ¼
		newDir->fileID = fileID++;		//�����ļ�ID
		newDir->realFileName = "dir_" + to_string(realDirNameCount++);		//������ʵ��Ŀ¼�ļ���
		ofstream outfile(newDir->realFileName);		//������ʵ�ļ�
		if (outfile.is_open())
		{
			outfile.close();
		}
		else
		{
			cout << "������ʵ�ļ�ʧ��" << endl;
		}

		//���û�����homeĿ¼
		if (fileOwner.homeDirectory == nullptr)
		{
			fileOwner.homeDirectory = newDir;
		}

		// ��ӵ���Ŀ¼
		parentDir.fd->directoryEntries.push_back(newDir);
	}
	else
	{
		cout << "�����޲���Ȩ��\n";
	}
}

// ɾ���ļ�
void rm(FCB& parentDir, string fileName)
{
	// Ҫɾ�����ļ��Ƿ����
	for (auto it = parentDir.fd->directoryEntries.begin(); it != parentDir.fd->directoryEntries.end(); ++it)
	{
		if ((*it)->fileName == fileName)
		{
			// Ҫɾ�����ļ��Ƿ�ΪĿ¼�ļ�
			if ((*it)->fileType == 'd')
			{
				cout << "����ɾ��Ŀ¼�ļ�\n";
				return;
			}

			// Ҫɾ�����ļ��Ƿ��Ѵ�
			if (isFileOpen(fileName, it)) {
				cout << "�����ļ��Ѿ���\n";
				return;
			}

			// �ж��Ƿ��ǹ���Ա����
			// �ж��Ƿ���Ȩ��ɾ�����ļ�
			if ((*it)->fileAcess[5] == 'x' || curUserPtr->userID == (*it)->ownerID ||
				isSuperUserDo || curUserPtr->userID == 0) {
				//���������
				recycleBlocks(**it);
				//ɾ����ʵ�ļ�
				remove((**it).realFileName.c_str());
				//�ͷ��ڴ�
				delete* it;
				//ɾ���ļ�
				parentDir.fd->directoryEntries.erase(it);
				cout << "�ļ� " << fileName << " ��ɾ��\n";
				return;
			}
			else
			{
				cout << "�����޲���Ȩ��\n";
				return;
			}
		}
	}

	// ��δ�ҵ��ļ�
	cout << "����" << fileName << " ������\n";
}

// ɾ��Ŀ¼����
void rmdir(FCB& parentDir, string dirName, string argument = "")
{
	// Ҫɾ����Ŀ¼�Ƿ����
	for (auto it = parentDir.fd->directoryEntries.begin(); it != parentDir.fd->directoryEntries.end(); ++it)
	{
		if ((*it)->fileName == dirName && (*it)->isDirectory)
		{
			// ��ʾ�û��Ƿ�ȷ��ɾ��
			char confirmation;
			if (argument != "-rf")
			{
				cout << "ȷ��ɾ��Ŀ¼ " << dirName << " ��(y/n): ";
				cin >> confirmation;
			}
			else
			{
				confirmation = 'y';
			}
			if (confirmation == 'y' || confirmation == 'Y') {
				// �ж��Ƿ��ǹ���Ա����
				// �ж��Ƿ���Ȩ��ɾ�����ļ�
				if ((*it)->fileAcess[5] == 'x' || curUserPtr->userID == (*it)->ownerID ||
					isSuperUserDo || curUserPtr->userID == 0) {
						{
							// ����ɾ��Ŀ¼����������
							for (auto& content : (*it)->fd->directoryEntries)
							{
								if (content->isDirectory)
								{
									rmdir(**it, content->fileName, argument);
								}
								else
								{
									rm(**it, content->fileName);
								}
							}
							//���������
							recycleBlocks(**it);
							//ɾ����ʵ�ļ�
							remove((**it).realFileName.c_str());
							//�ͷ��ڴ�
							delete* it;
							//ɾ��Ŀ¼
							parentDir.fd->directoryEntries.erase(it);
							cout << "Ŀ¼ " << dirName << " ��ɾ��\n";
						}
						return;
				}
				else
				{
					cout << "�����޲���Ȩ��\n";
					return;
				}
			}
		}
	}

	// ��δ�ҵ�Ŀ¼
	cout << "����" << dirName << " ������\n";
}

// ��תĿ¼
void cd(string dirName)
{

	//�����ϼ�Ŀ¼����
	if (dirName == "..")
	{
		if (curDirPtr->fileID == 0)
		{
			cout << "���ڸ�Ŀ¼��\n";
			return;
		}

		// ���ĵ�ǰĿ¼
		curDirPtr = preDirPtr;

		// �����ϼ�Ŀ¼
		preDirPtr = curDirPtr->parentDir;

		return;
	}

	// ������ǰĿ¼��Ŀ¼��
	for (auto entry : curDirPtr->fd->directoryEntries)
	{
		if (entry->fileName == dirName && entry->isDirectory)
		{
			// �����ϼ�Ŀ¼
			preDirPtr = entry->parentDir;

			// ���ĵ�ǰĿ¼
			curDirPtr = entry;

			return;
		}
	}

	// ��δ�ҵ�Ŀ¼
	cout << "Ŀ¼ " << dirName << " ������\n";
}

// ���ļ�
bool open(string fileName, const ios_base::openmode& openmode = ios_base::in) {
	// �����ļ��Ƿ����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// ����ļ�������
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�������\n";
		return false;
	}

	FCB* fileToOpen = *it;

	// ����Ƿ���Ŀ¼�ļ�
	if (fileToOpen->isDirectory) {
		cout << "�����޷���Ŀ¼�ļ�\n";
		return false;
	}

	// ����û�Ȩ��
	if (curUserPtr->userID != fileToOpen->ownerID
		&& fileToOpen->fileAcess[5] != 'x' && !isSuperUserDo) {
		cout << "�����޲���Ȩ��\n";
		return false;
	}

	if (isFileOpen((**it).fileName, it)) {
		cout << "�����ļ��Ѿ���\n";
		return false;
	}

	// ����ʵ�ļ�
	(*it)->realFileStream = new fstream((*it)->realFileName, openmode);
	if (!((*it)->realFileStream->is_open()))
	{
		cout << "������ʵ�ļ���ʧ��\n";
		return false;
	}

	// ���ļ����뵽���ļ���
	openFileTable->fileDescriptors.push_back(fileToOpen);

	return true;
}

// �ر��ļ�
void close(string fileName) {
	vector<FCB*>::iterator it;

	if (!isFileOpen(fileName, it)) {
		cout << "�����ļ�δ��\n";
		return;
	}

	// ����û�Ȩ��
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[5] != 'x' && !isSuperUserDo) {
		cout << "�����޲���Ȩ��\n";
		return;
	}

	// �ر���ʵ�ļ�
	(*it)->realFileStream->close();

	// �Ӵ��ļ�����ɾ��
	openFileTable->fileDescriptors.erase(it);
}

// ���ļ�
void read(string fileName) {
	// �����ļ��Ƿ����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// ����ļ�������
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�������\n";
		return;
	}

	// ����Ƿ���Ŀ¼�ļ�
	if ((*it)->isDirectory) {
		cout << "�����޷���ȡĿ¼�ļ�\n";
		return;
	}

	// ���ļ�
	bool isAlreadyOpen = isFileOpen(fileName, it);
	if (!isAlreadyOpen)
	{
		open((*it)->fileName, ios_base::in);
	}

	// ����û�Ȩ��
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[3] != 'r' && !isSuperUserDo) {
		cout << "�����޶�Ȩ��\n";
		return;
	}

	// ��ʾ��ʵ�ļ�����
	string output;
	while (getline(*((*it)->realFileStream), output)) {
		cout << output << endl;
	}

	// �ر��ļ�
	if (!isAlreadyOpen)
	{
		close((*it)->fileName);
	}
}

// д�ļ�
void write(string fileName) {
	// �����ļ��Ƿ����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// ����ļ�������
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�������\n";
		return;
	}

	// ����Ƿ���Ŀ¼�ļ�
	if ((*it)->isDirectory) {
		cout << "�����޷�д��Ŀ¼�ļ�\n";
		return;
	}

	// ����û�Ȩ��
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[4] != 'w' && !isSuperUserDo) {
		cout << "������дȨ��\n";
		return;
	}

	// ���ļ�
	bool isAlreadyOpen = isFileOpen(fileName, it);
	if (!isAlreadyOpen)
	{
		if (!open((*it)->fileName, ios_base::out | ios_base::app))
		{
			return;
		}
	}

	cout << "����:wq����\n";
	string input;
	char c;

	while (true) {
		cout << ">> ";

		while (cin.get(c) && c != '\n') {
			input += c;
		}

		if (input == ":wq") {
			break;
		}

		// д����ʵ�ļ�
		*((*it)->realFileStream) << input << endl;
		input.clear();
	}

	// �ر��ļ�
	if (!isAlreadyOpen)
	{
		close((*it)->fileName);
	}
}

// ��������Ȩ��
string PermissionDigitToBinary(char digit) {
	switch (digit) {
	case '0':
		return "---";
	case '1':
		return "--x";
	case '2':
		return "-w-";
	case '3':
		return "-wx";
	case '4':
		return "r--";
	case '5':
		return "r-x";
	case '6':
		return "rw-";
	case '7':
		return "rwx";
	default:
		return "";
	}
}

// �޸��ļ�Ȩ��
void chmod(string fileName, string newPermission) {
	// ��������ʽ
	if (newPermission.length() != 2 || !isdigit(newPermission[0]) || !isdigit(newPermission[1])) {
		cout << "���󣡲�������\n";
		return;
	}

	// ���������ַ�
	string ownerPermission = PermissionDigitToBinary(newPermission[0]);
	string otherPermission = PermissionDigitToBinary(newPermission[1]);

	// ������ַ�Χ
	if (ownerPermission == "" || otherPermission == "") {
		cout << "���󣡲�������\n";
		return;
	}

	// �����ļ��Ƿ����
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// ����ļ�������
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "�����ļ�������\n";
		return;
	}

	// ��鵱ǰ�û�Ȩ��
	if (!isSuperUserDo && curUserPtr->userName != fileName && curUserPtr->userID != admin->userID) {
		cout << "�����޲���Ȩ��\n";
		return;
	}

	// �޸��ļ�Ȩ��
	FCB* fileToModify = *it;
	fileToModify->fileAcess = ownerPermission + otherPermission;
}
