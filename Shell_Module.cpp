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
extern UCB* curUserPtr;    //当前用户
extern FCB* curDirPtr;    //当前用户
extern FCB* preDirPtr;		//上级目录
extern int userID; //用户id
extern int fileID;	//文件ID
extern bool isSuperUserDo;	//管理员暂时登录
extern DFBM* dist_free_block_manager;   //空闲物理块管理器
extern MFD* master_file_directory;  //主文件目录

// 函数声明
void login();
void logout();
void sudo();
void addUser();
void delUser(string usernameToDelete);
void mkdir(UCB& fileOwner, FCB& parentDir, string dirName);
void touch(UCB& fileOwner, FCB& parentDir, string fileName, int contentLength = 512);
void dir(FCB& directory, string argument);
void rmdir(FCB& parentDir, string dirName, string argument);
void rm(FCB& parentDir, string fileName);
void cd(string dirName);
bool open(string fileName, const ios_base::openmode& openmode = ios_base::in);
void close(string fileName);
void read(string fileName);
void write(string fileName);
void chmod(string newAccess, string fileName);
void man();
void showDisk(string argument);
void showFileBlocks(string fileName);
void pwd();
void whoami();
void printWelcomeMessage();

// 交互函数
void runShell()
{
	cout << endl;
	printWelcomeMessage();

	string input;
	while (true)
	{

		// 输出提示符
		cout << "[" << curUserPtr->userName << "@小郭文件系统:" << curDirPtr->fileName << "] $ ";

		// 读取用户输入
		getline(cin, input);

		// 处理空格换行
		if (input.empty())
			continue;

		// 解析输入命令
		istringstream iss(input);
		string command;
		iss >> command;

		// 以下操作不能带参数
		if (command == "sudo")
		{
			sudo();
			getline(cin, input);
			continue;
		}
		else if (command == "man")
		{
			man();
		}
		else if (command == "pwd")
		{
			pwd();
		}
		else if (command == "whoami")
		{
			whoami();
		}
		else if (command == "login")
		{
			login();
		}
		else if (command == "logout")
		{
			logout();
		}
		else if (command == "adduser")
		{
			addUser();
			getline(cin, input);
		}
		else if (command == "quit")
		{
			cout << "BYE BYE!\n";
			return;
		}
		else if (command == "ls" || command == "df" || command == "mkdir" || command == "chmod" ||
			command == "du" || command == "open" || command == "close" || command == "vi" ||
			command == "cat" || command == "touch" || command == "deluser" || command == "rm" ||
			command == "cd" || command == "rmdir")
		{
			string argument;
			iss >> argument;

			// 以下命令可以带也可以不带参数
			if (command == "ls")
			{
				dir(*curDirPtr, argument);
			}
			else if (command == "df")
			{
				showDisk(argument);
			}
			else
			{
				string argument1;
				iss >> argument1;

				// 以下命令至少带一个参数
				if (argument.empty())
				{
					cout << "错误！缺少参数\n";
					continue;
				}
				else if (command == "mkdir")
				{
					mkdir(*curUserPtr, *curDirPtr, argument);
				}
				else if (command == "chmod")
				{
					chmod(argument, argument1);
				}
				else if (command == "du")
				{
					showFileBlocks(argument);
				}
				else if (command == "open")
				{
					open(argument);
				}
				else if (command == "close")
				{
					close(argument);
				}
				else if (command == "cat")
				{
					read(argument);
				}
				else if (command == "vi")
				{
					write(argument);
				}
				else if (command == "deluser")
				{
					delUser(argument);
					getline(cin, input);
				}
				else if (command == "touch")
				{
					if (argument1 != "")
					{
						touch(*curUserPtr, *curDirPtr, argument, stoi(argument1));
					}
					else
					{
						touch(*curUserPtr, *curDirPtr, argument);
					}
				}
				else if (command == "rmdir")
				{
					rmdir(*curDirPtr, argument, argument1);
					if (argument1 == "")
					{
						getline(cin, input);
					}
				}
				else if (command == "rm")
				{
					rm(*curDirPtr, argument);
				}
				else if (command == "cd")
				{
					cd(argument);
				}
				else
				{
					cout << "错误！未知命令\n";
				}
			}
		}
		else
		{
			cout << "错误！未知命令\n";
		}
		// 自动调用sudo结束
		isSuperUserDo = false;
	}
}