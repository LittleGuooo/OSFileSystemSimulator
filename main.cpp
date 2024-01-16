#include <iostream>
#include <ctime>
#include <cstring>
#include <vector>

#include "DataStruct.h"

using namespace std;

// 函数声明
void initializeFileSystem();
void runShell();
void cleanUpFileSystem();

int main() {
    initializeFileSystem();
    runShell();
    cleanUpFileSystem();

    return 0;
}
