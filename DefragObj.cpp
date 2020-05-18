#include "DefragObj.h"

DefragObj::DefragObj(fs::path filePath)
{
    this->filePath = filePath;
}

BOOL DefragObj::openFile()
{
    hFile = CreateFile(filePath.u8string().c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return 0;
    return 1;
}

HANDLE DefragObj::getFileHandle()
{
    return this->hFile;
}

LARGE_INTEGER DefragObj::getFileSize()
{
    return this->fileSize;
}

BOOL DefragObj::findFileSize()
{
    GetFileSizeEx(this->hFile, &this->fileSize);
    if(fileSize.QuadPart != 0) return 1;
    else return 0;
}

fs::path DefragObj::getFilePath()
{
    return this->filePath;
}

std::string DefragObj::getFileName()
{
    return this->filePath.u8string();
}