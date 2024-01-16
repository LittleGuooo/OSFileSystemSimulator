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
extern FCB* curDirPtr;    //当前目录
extern int userID; //用户id
extern bool isSuperUserDo;	//管理员暂时登录
extern DFBM* dist_free_block_manager;   //空闲物理块管理器
extern MFD* master_file_directory;  //主文件目录

// 函数声明
void mkdir(UCB& fileOwner, FCB& parentDir, string dirName);
void rmdir(FCB& parentDir, string dirName, string argument);

// sudo操作
void sudo() {
	isSuperUserDo = true;
}

// 用户登录
void login() {
	string inputUsername, inputPassword;

	cout << "用户名：";
	getline(cin, inputUsername);

	cout << "密码：";
	getline(cin, inputPassword);

	// 查找匹配的用户
	for (auto& user : users) {
		if (user->userName == inputUsername && user->password == inputPassword) {
			curUserPtr = user;
			return;
		}
	}

	cout << "用户名或密码错误" << endl;
}

// 注销登录
void logout() {
	if (curUserPtr != nullptr) {
		curUserPtr = guest;
	}
	else {
		cout << "错误！当前无用户登录" << endl;
	}
}

// 添加用户
void addUser() {
	// 是否已达用户数量上限
	if (users.size() == MAX_USER_NUM) {
		cout << "已达用户数量上限" << endl;
		return;
	}

	// 判断是否是管理员操作
	if (isSuperUserDo || curUserPtr->userID == admin->userID) {
		string newUsername, newPassword;

		// 创建新用户
		cout << "新用户名：";
		cin >> newUsername;

		cout << "密码：";
		cin >> newPassword;

		UCB* newUser = new UCB{ newUsername, newPassword, userID++ };

		// 创建home目录
		mkdir(*newUser, *curDirPtr, newUsername);

		users.push_back(newUser);

		cout << "添加成功" << endl;
	}
	else {
		cout << "错误！无操作权限" << endl;
	}
}

// 删除用户函数
void delUser(string usernameToDelete) {
	// 是否不存在用户
	if (users.size() == 0) {
		cout << "不存在用户" << endl;
		return;
	}

	// 判断是否是管理员操作
	if (isSuperUserDo || curUserPtr->userID == admin->userID) {
		// 查找用户是否存在
		auto it = find_if(users.begin(), users.end(), EqualTargetUserName(usernameToDelete));

		if (it != users.end()) {
			// 提示是否确认删除
			char confirm;
			cout << "确认删除用户 " << usernameToDelete << "？(y/n): ";
			cin >> confirm;

			if (confirm == 'y' || confirm == 'Y') {
				// 删除home目录
				rmdir(*(admin->homeDirectory), (*it)->homeDirectory->fileName,"-rf");

				// 删除用户
				users.erase(it);
				cout << "删除成功" << endl;
			}
		}
		else {
			cout << "用户 " << usernameToDelete << " 不存在。" << endl;
		}
	}
	else {
		cout << "错误！无操作权限" << endl;
	}
}
