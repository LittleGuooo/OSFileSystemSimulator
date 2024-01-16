#include<vector>
#include<string>

#include "DataStruct.h"

using namespace std;

// 定义全局变量
vector<UCB*> users;  //所有用户
UCB* curUserPtr;    //当前用户
FCB* curDirPtr;    //当前目录
FCB* preDirPtr;		//上级目录
UCB* admin;  //管理员
UCB* guest;	//游客
int userID = 0; //用户id
int fileID;	//文件ID
OFT* openFileTable;	//系统打开文件表
bool isSuperUserDo;	//管理员暂时登录
DFBM* dist_free_block_manager; //空闲物理块管理器
MFD* master_file_directory;   //主文件目录
int realFileNameCount;		//生成真实的文件名
int realDirNameCount;		//生成真实的目录名
FCB* parentDirOfRootDir;		//用于生成和删除根目录的目录文件

// 函数声明
void mkdir(UCB& fileOwner, FCB& parentDir, string dirName);
void rmdir(FCB& parentDir, string dirName,string argument);

// 初始化
void initializeFileSystem() {
    // 初始化空闲磁盘管理器
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

    // 初始化管理员用户
    admin = new UCB{ "root", "root", userID++, nullptr };
    users.push_back(admin);
    curUserPtr = admin;

    // 初始化根目录
    parentDirOfRootDir = new FCB();
    parentDirOfRootDir->fd = new FD;
    parentDirOfRootDir->fileID = -1;
    mkdir(*admin, *parentDirOfRootDir, "rootDir");
    admin->homeDirectory->parentDir = nullptr;
    MFD* master_file_directory = new MFD();
    master_file_directory->rootDirectory = admin->homeDirectory;

    // 初始化测试用户
    UCB* defaultUser = new UCB{ "xu", "666", userID++ };
    mkdir(*defaultUser, *(master_file_directory->rootDirectory), defaultUser->userName);
    users.push_back(defaultUser);

    // 初始化游客用户
    guest = new UCB{ "guest", "", -1 };

    // 初始化当前用户
    curUserPtr = guest;

    // 初始化当前目录
    curDirPtr = master_file_directory->rootDirectory;

    // 初始化上级目录
    preDirPtr = master_file_directory->rootDirectory->parentDir;

    // 初始化sudo命令
    isSuperUserDo = false;

    // 初始化系统打开文件表
    openFileTable = new OFT();
}

// 析构
void cleanUpFileSystem() {
    // 删除根目录
    rmdir(*parentDirOfRootDir, admin->homeDirectory->fileName, "-rf");

    // 删除磁盘空间
    for (auto& distBlock : dist_free_block_manager->distBlocks) {
        delete distBlock;
    }
}
