#include<vector>
#include<string>
#include<iostream>
#include <iomanip>
#include<malloc.h>

#include "DataStruct.h"

using namespace std;

// 外部声明全局变量
extern vector<UCB*> users;  //所有用户
extern UCB* admin;  //管理员
extern UCB* guest;	//游客
extern UCB* curUserPtr;    //当前用户
extern FCB* curDirPtr;    //当前用户
extern FCB* preDirPtr;		//上级目录
extern int userID; //用户id
extern int fileID;	//文件ID
extern OFT* openFileTable;	//系统打开文件表
extern bool isSuperUserDo;	//管理员暂时登录
extern DFBM* dist_free_block_manager;   //空闲物理块管理器
extern MFD* master_file_directory;  //主文件目录
extern int realFileNameCount;		//生成真实的文件名
extern int realDirNameCount;		//生成真实的目录名

void allocateFreeBlocks(int length, FCB& newFile);
void recycleBlocks(FCB& fileToDelete);

// 文件是否已打开
bool isFileOpen(string fileName, vector<FCB* >::iterator& it) {
	// 检查文件是否已经打开
	auto openFileIt = find_if(openFileTable->fileDescriptors.begin(),
		openFileTable->fileDescriptors.end(), EqualTargetFileName(fileName));

	if (openFileIt != openFileTable->fileDescriptors.end()) {
		it = openFileIt;
		return true;
	}

	return false;
}

// 创建文件
void touch(UCB& fileOwner, FCB& parentDir, string fileName, int contentLength = FILE_DEFAULT_LENTH)
{
	// 判断是否文件重名
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	if (it != curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件重名\n";
		return;
	}

	// 判断是否在自己的目录中
	// 判断是否是管理员操作
	// 判断是否是管理员暂时登录
	if (fileOwner.userID == parentDir.ownerID || isSuperUserDo || curUserPtr->userID == 0)
	{
		// 创建文件控制块
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
		//tm* timeinfo = localtime(&currentTimeT);	//Windows不能使用该版本
		tm* timeinfo = new tm;
		localtime_s(timeinfo, &currentTimeT);
		newFile->timeinfo = timeinfo;
		newFile->isOpen = false;
		newFile->isDirectory = false;
		newFile->content;
		newFile->fd = nullptr;
		newFile->parentDir = &parentDir;	//设置父目录
		newFile->fileID = fileID++;		//设置文件ID
		newFile->realFileName = "file_" + to_string(realFileNameCount++) + ".txt";		//设置真实的文件名
		ofstream outfile(newFile->realFileName);		//创建真实文件
		if (outfile.is_open())
		{
			outfile.close();
		}
		else
		{
			cout << "创建真实文件失败" << endl;
		}

		// 将新文件添加到父目录的目录项中
		parentDir.fd->directoryEntries.push_back(newFile);
	}
	else
	{
		cout << "错误！无操作权限\n";
	}
}

void mkdir(UCB& fileOwner, FCB& parentDir, string dirName)
{
	// 判断是否文件重名
	if (curDirPtr != nullptr)
	{
		auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
			curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(dirName));

		if (it != curDirPtr->fd->directoryEntries.end()) {
			cout << "错误！文件重名\n";
			return;
		}
	}

	// 判断是否在自己的目录中
	// 判断是否在创建根目录
	// 判断是否是管理员操作
	if (parentDir.fileID == -1 || fileOwner.userID == parentDir.ownerID || isSuperUserDo || curUserPtr->userID == 0)
	{
		// 创建目录文件控制块
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
		//tm* timeinfo = localtime(&currentTimeT);	//Windows不能使用该版本
		tm* timeinfo = new tm;
		localtime_s(timeinfo, &currentTimeT);
		newDir->timeinfo = timeinfo;
		newDir->isOpen = false;
		newDir->isDirectory = true;
		newDir->content;
		newDir->fd = new FD;  // 创建目录
		newDir->parentDir = &parentDir;	//设置父目录
		newDir->fileID = fileID++;		//设置文件ID
		newDir->realFileName = "dir_" + to_string(realDirNameCount++);		//设置真实的目录文件名
		ofstream outfile(newDir->realFileName);		//创建真实文件
		if (outfile.is_open())
		{
			outfile.close();
		}
		else
		{
			cout << "创建真实文件失败" << endl;
		}

		//新用户创建home目录
		if (fileOwner.homeDirectory == nullptr)
		{
			fileOwner.homeDirectory = newDir;
		}

		// 添加到父目录
		parentDir.fd->directoryEntries.push_back(newDir);
	}
	else
	{
		cout << "错误！无操作权限\n";
	}
}

// 删除文件
void rm(FCB& parentDir, string fileName)
{
	// 要删除的文件是否存在
	for (auto it = parentDir.fd->directoryEntries.begin(); it != parentDir.fd->directoryEntries.end(); ++it)
	{
		if ((*it)->fileName == fileName)
		{
			// 要删除的文件是否为目录文件
			if ((*it)->fileType == 'd')
			{
				cout << "错误！删除目录文件\n";
				return;
			}

			// 要删除的文件是否已打开
			if (isFileOpen(fileName, it)) {
				cout << "错误！文件已经打开\n";
				return;
			}

			// 判断是否是管理员操作
			// 判断是否有权限删除该文件
			if ((*it)->fileAcess[5] == 'x' || curUserPtr->userID == (*it)->ownerID ||
				isSuperUserDo || curUserPtr->userID == 0) {
				//回收物理块
				recycleBlocks(**it);
				//删除真实文件
				remove((**it).realFileName.c_str());
				//释放内存
				delete* it;
				//删除文件
				parentDir.fd->directoryEntries.erase(it);
				cout << "文件 " << fileName << " 已删除\n";
				return;
			}
			else
			{
				cout << "错误！无操作权限\n";
				return;
			}
		}
	}

	// 若未找到文件
	cout << "错误！" << fileName << " 不存在\n";
}

// 删除目录函数
void rmdir(FCB& parentDir, string dirName, string argument = "")
{
	// 要删除的目录是否存在
	for (auto it = parentDir.fd->directoryEntries.begin(); it != parentDir.fd->directoryEntries.end(); ++it)
	{
		if ((*it)->fileName == dirName && (*it)->isDirectory)
		{
			// 提示用户是否确认删除
			char confirmation;
			if (argument != "-rf")
			{
				cout << "确认删除目录 " << dirName << " ？(y/n): ";
				cin >> confirmation;
			}
			else
			{
				confirmation = 'y';
			}
			if (confirmation == 'y' || confirmation == 'Y') {
				// 判断是否是管理员操作
				// 判断是否有权限删除该文件
				if ((*it)->fileAcess[5] == 'x' || curUserPtr->userID == (*it)->ownerID ||
					isSuperUserDo || curUserPtr->userID == 0) {
						{
							// 迭代删除目录中所有内容
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
							//回收物理块
							recycleBlocks(**it);
							//删除真实文件
							remove((**it).realFileName.c_str());
							//释放内存
							delete* it;
							//删除目录
							parentDir.fd->directoryEntries.erase(it);
							cout << "目录 " << dirName << " 已删除\n";
						}
						return;
				}
				else
				{
					cout << "错误！无操作权限\n";
					return;
				}
			}
		}
	}

	// 若未找到目录
	cout << "错误！" << dirName << " 不存在\n";
}

// 跳转目录
void cd(string dirName)
{

	//返回上级目录操作
	if (dirName == "..")
	{
		if (curDirPtr->fileID == 0)
		{
			cout << "已在根目录下\n";
			return;
		}

		// 更改当前目录
		curDirPtr = preDirPtr;

		// 更新上级目录
		preDirPtr = curDirPtr->parentDir;

		return;
	}

	// 遍历当前目录的目录项
	for (auto entry : curDirPtr->fd->directoryEntries)
	{
		if (entry->fileName == dirName && entry->isDirectory)
		{
			// 更新上级目录
			preDirPtr = entry->parentDir;

			// 更改当前目录
			curDirPtr = entry;

			return;
		}
	}

	// 若未找到目录
	cout << "目录 " << dirName << " 不存在\n";
}

// 打开文件
bool open(string fileName, const ios_base::openmode& openmode = ios_base::in) {
	// 查找文件是否存在
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// 如果文件不存在
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件不存在\n";
		return false;
	}

	FCB* fileToOpen = *it;

	// 检查是否是目录文件
	if (fileToOpen->isDirectory) {
		cout << "错误！无法打开目录文件\n";
		return false;
	}

	// 检查用户权限
	if (curUserPtr->userID != fileToOpen->ownerID
		&& fileToOpen->fileAcess[5] != 'x' && !isSuperUserDo) {
		cout << "错误！无操作权限\n";
		return false;
	}

	if (isFileOpen((**it).fileName, it)) {
		cout << "错误！文件已经打开\n";
		return false;
	}

	// 打开真实文件
	(*it)->realFileStream = new fstream((*it)->realFileName, openmode);
	if (!((*it)->realFileStream->is_open()))
	{
		cout << "错误！真实文件打开失败\n";
		return false;
	}

	// 将文件加入到打开文件表
	openFileTable->fileDescriptors.push_back(fileToOpen);

	return true;
}

// 关闭文件
void close(string fileName) {
	vector<FCB*>::iterator it;

	if (!isFileOpen(fileName, it)) {
		cout << "错误！文件未打开\n";
		return;
	}

	// 检查用户权限
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[5] != 'x' && !isSuperUserDo) {
		cout << "错误！无操作权限\n";
		return;
	}

	// 关闭真实文件
	(*it)->realFileStream->close();

	// 从打开文件表中删除
	openFileTable->fileDescriptors.erase(it);
}

// 读文件
void read(string fileName) {
	// 查找文件是否存在
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// 如果文件不存在
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件不存在\n";
		return;
	}

	// 检查是否是目录文件
	if ((*it)->isDirectory) {
		cout << "错误！无法读取目录文件\n";
		return;
	}

	// 打开文件
	bool isAlreadyOpen = isFileOpen(fileName, it);
	if (!isAlreadyOpen)
	{
		open((*it)->fileName, ios_base::in);
	}

	// 检查用户权限
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[3] != 'r' && !isSuperUserDo) {
		cout << "错误！无读权限\n";
		return;
	}

	// 显示真实文件内容
	string output;
	while (getline(*((*it)->realFileStream), output)) {
		cout << output << endl;
	}

	// 关闭文件
	if (!isAlreadyOpen)
	{
		close((*it)->fileName);
	}
}

// 写文件
void write(string fileName) {
	// 查找文件是否存在
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// 如果文件不存在
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件不存在\n";
		return;
	}

	// 检查是否是目录文件
	if ((*it)->isDirectory) {
		cout << "错误无法写入目录文件\n";
		return;
	}

	// 检查用户权限
	if (curUserPtr->userID != (*it)->ownerID
		&& (*it)->fileAcess[4] != 'w' && !isSuperUserDo) {
		cout << "错误！无写权限\n";
		return;
	}

	// 打开文件
	bool isAlreadyOpen = isFileOpen(fileName, it);
	if (!isAlreadyOpen)
	{
		if (!open((*it)->fileName, ios_base::out | ios_base::app))
		{
			return;
		}
	}

	cout << "输入:wq结束\n";
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

		// 写入真实文件
		*((*it)->realFileStream) << input << endl;
		input.clear();
	}

	// 关闭文件
	if (!isAlreadyOpen)
	{
		close((*it)->fileName);
	}
}

// 解析数字权限
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

// 修改文件权限
void chmod(string fileName, string newPermission) {
	// 检查参数格式
	if (newPermission.length() != 2 || !isdigit(newPermission[0]) || !isdigit(newPermission[1])) {
		cout << "错误！参数错误\n";
		return;
	}

	// 解析数字字符
	string ownerPermission = PermissionDigitToBinary(newPermission[0]);
	string otherPermission = PermissionDigitToBinary(newPermission[1]);

	// 检查数字范围
	if (ownerPermission == "" || otherPermission == "") {
		cout << "错误！参数错误\n";
		return;
	}

	// 查找文件是否存在
	auto it = find_if(curDirPtr->fd->directoryEntries.begin(),
		curDirPtr->fd->directoryEntries.end(), EqualTargetFileName(fileName));

	// 如果文件不存在
	if (it == curDirPtr->fd->directoryEntries.end()) {
		cout << "错误！文件不存在\n";
		return;
	}

	// 检查当前用户权限
	if (!isSuperUserDo && curUserPtr->userName != fileName && curUserPtr->userID != admin->userID) {
		cout << "错误！无操作权限\n";
		return;
	}

	// 修改文件权限
	FCB* fileToModify = *it;
	fileToModify->fileAcess = ownerPermission + otherPermission;
}
