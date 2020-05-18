#pragma once
#include "Header.h"
class DefragObj
{
private:
    fs::path filePath;
    HANDLE hFile;
    LARGE_INTEGER fileSize;
public:
    DefragObj(fs::path filePath);
    DefragObj();
    BOOL openFile();
    HANDLE getFileHandle();
    std::string getFileName();
    BOOL findFileSize();
    LARGE_INTEGER getFileSize();
    fs::path getFilePath();
};