#include<vector>
#include<string>

#include "DataStruct.h"

using namespace std;

// ����ȫ�ֱ���
vector<UCB*> users;  //�����û�
UCB* curUserPtr;    //��ǰ�û�
FCB* curDirPtr;    //��ǰĿ¼
FCB* preDirPtr;		//�ϼ�Ŀ¼
UCB* admin;  //����Ա
UCB* guest;	//�ο�
int userID = 0; //�û�id
int fileID;	//�ļ�ID
OFT* openFileTable;	//ϵͳ���ļ���
bool isSuperUserDo;	//����Ա��ʱ��¼
DFBM* dist_free_block_manager; //��������������
MFD* master_file_directory;   //���ļ�Ŀ¼
int realFileNameCount;		//������ʵ���ļ���
int realDirNameCount;		//������ʵ��Ŀ¼��
FCB* parentDirOfRootDir;		//�������ɺ�ɾ����Ŀ¼��Ŀ¼�ļ�

// ��������
void mkdir(UCB& fileOwner, FCB& parentDir, string dirName);
void rmdir(FCB& parentDir, string dirName,string argument);

// ��ʼ��
void initializeFileSystem() {
    // ��ʼ�����д��̹�����
    dist_free_block_manager = new DFBM();
    dist_free_block_manager->distBlocks.resize(DISK_BLOCK_NUM);
    dist_free_block_manager->totalBlockNum = DISK_BLOCK_NUM;
    dist_free_block_manager->freeBlockNum = DISK_BLOCK_NUM;
    dist_free_block_manager->usedBlockNUm = 0;
    DB* curDiskBlock;
    for (int i = 0; i < DISK_BLOCK_NUM; i++) {
        curDiskBlock = new DB{ DISK_BLOCK_LENGTH, i, false, nullptr };
        dist_free_block_manager->distBlocks[i] = curDiskBlock;
    }

    // ��ʼ������Ա�û�
    admin = new UCB{ "root", "root", userID++, nullptr };
    users.push_back(admin);
    curUserPtr = admin;

    // ��ʼ����Ŀ¼
    parentDirOfRootDir = new FCB();
    parentDirOfRootDir->fd = new FD;
    parentDirOfRootDir->fileID = -1;
    mkdir(*admin, *parentDirOfRootDir, "rootDir");
    admin->homeDirectory->parentDir = nullptr;
    MFD* master_file_directory = new MFD();
    master_file_directory->rootDirectory = admin->homeDirectory;

    // ��ʼ�������û�
    UCB* defaultUser = new UCB{ "xu", "666", userID++ };
    mkdir(*defaultUser, *(master_file_directory->rootDirectory), defaultUser->userName);
    users.push_back(defaultUser);

    // ��ʼ���ο��û�
    guest = new UCB{ "guest", "", -1 };

    // ��ʼ����ǰ�û�
    curUserPtr = guest;

    // ��ʼ����ǰĿ¼
    curDirPtr = master_file_directory->rootDirectory;

    // ��ʼ���ϼ�Ŀ¼
    preDirPtr = master_file_directory->rootDirectory->parentDir;

    // ��ʼ��sudo����
    isSuperUserDo = false;

    // ��ʼ��ϵͳ���ļ���
    openFileTable = new OFT();
}

// ����
void cleanUpFileSystem() {
    // ɾ����Ŀ¼
    rmdir(*parentDirOfRootDir, admin->homeDirectory->fileName, "-rf");

    // ɾ�����̿ռ�
    for (auto& distBlock : dist_free_block_manager->distBlocks) {
        delete distBlock;
    }
}
