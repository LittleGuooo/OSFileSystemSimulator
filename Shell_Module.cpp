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
extern UCB* curUserPtr;    //��ǰ�û�
extern FCB* curDirPtr;    //��ǰ�û�
extern FCB* preDirPtr;		//�ϼ�Ŀ¼
extern int userID; //�û�id
extern int fileID;	//�ļ�ID
extern bool isSuperUserDo;	//����Ա��ʱ��¼
extern DFBM* dist_free_block_manager;   //��������������
extern MFD* master_file_directory;  //���ļ�Ŀ¼

// ��������
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

// ��������
void runShell()
{
	cout << endl;
	printWelcomeMessage();

	string input;
	while (true)
	{

		// �����ʾ��
		cout << "[" << curUserPtr->userName << "@С���ļ�ϵͳ:" << curDirPtr->fileName << "] $ ";

		// ��ȡ�û�����
		getline(cin, input);

		// ����ո���
		if (input.empty())
			continue;

		// ������������
		istringstream iss(input);
		string command;
		iss >> command;

		// ���²������ܴ�����
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

			// ����������Դ�Ҳ���Բ�������
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

				// �����������ٴ�һ������
				if (argument.empty())
				{
					cout << "����ȱ�ٲ���\n";
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
					cout << "����δ֪����\n";
				}
			}
		}
		else
		{
			cout << "����δ֪����\n";
		}
		// �Զ�����sudo����
		isSuperUserDo = false;
	}
}