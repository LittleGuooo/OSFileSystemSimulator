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
extern FCB* curDirPtr;    //��ǰĿ¼
extern int userID; //�û�id
extern bool isSuperUserDo;	//����Ա��ʱ��¼
extern DFBM* dist_free_block_manager;   //��������������
extern MFD* master_file_directory;  //���ļ�Ŀ¼

// ��������
void mkdir(UCB& fileOwner, FCB& parentDir, string dirName);
void rmdir(FCB& parentDir, string dirName, string argument);

// sudo����
void sudo() {
	isSuperUserDo = true;
}

// �û���¼
void login() {
	string inputUsername, inputPassword;

	cout << "�û�����";
	getline(cin, inputUsername);

	cout << "���룺";
	getline(cin, inputPassword);

	// ����ƥ����û�
	for (auto& user : users) {
		if (user->userName == inputUsername && user->password == inputPassword) {
			curUserPtr = user;
			return;
		}
	}

	cout << "�û������������" << endl;
}

// ע����¼
void logout() {
	if (curUserPtr != nullptr) {
		curUserPtr = guest;
	}
	else {
		cout << "���󣡵�ǰ���û���¼" << endl;
	}
}

// ����û�
void addUser() {
	// �Ƿ��Ѵ��û���������
	if (users.size() == MAX_USER_NUM) {
		cout << "�Ѵ��û���������" << endl;
		return;
	}

	// �ж��Ƿ��ǹ���Ա����
	if (isSuperUserDo || curUserPtr->userID == admin->userID) {
		string newUsername, newPassword;

		// �������û�
		cout << "���û�����";
		cin >> newUsername;

		cout << "���룺";
		cin >> newPassword;

		UCB* newUser = new UCB{ newUsername, newPassword, userID++ };

		// ����homeĿ¼
		mkdir(*newUser, *curDirPtr, newUsername);

		users.push_back(newUser);

		cout << "��ӳɹ�" << endl;
	}
	else {
		cout << "�����޲���Ȩ��" << endl;
	}
}

// ɾ���û�����
void delUser(string usernameToDelete) {
	// �Ƿ񲻴����û�
	if (users.size() == 0) {
		cout << "�������û�" << endl;
		return;
	}

	// �ж��Ƿ��ǹ���Ա����
	if (isSuperUserDo || curUserPtr->userID == admin->userID) {
		// �����û��Ƿ����
		auto it = find_if(users.begin(), users.end(), EqualTargetUserName(usernameToDelete));

		if (it != users.end()) {
			// ��ʾ�Ƿ�ȷ��ɾ��
			char confirm;
			cout << "ȷ��ɾ���û� " << usernameToDelete << "��(y/n): ";
			cin >> confirm;

			if (confirm == 'y' || confirm == 'Y') {
				// ɾ��homeĿ¼
				rmdir(*(admin->homeDirectory), (*it)->homeDirectory->fileName,"-rf");

				// ɾ���û�
				users.erase(it);
				cout << "ɾ���ɹ�" << endl;
			}
		}
		else {
			cout << "�û� " << usernameToDelete << " �����ڡ�" << endl;
		}
	}
	else {
		cout << "�����޲���Ȩ��" << endl;
	}
}
