#pragma once
#include "header.h"
#include "defragobj.h"
#include "volumeinfo.h"

class Defragmenter
{
private:
    char volumeName[4];
    HANDLE hDisk;
    DWORD clusterSize;
    ULONG clusterCount;
    ULONG64 beginLCN;
    ULONG64 endLCN;
    PRETRIEVAL_POINTERS_BUFFER OutBuf;
    VolumeInfo info;
public:
    Defragmenter();
    BOOL setVolumeName(fs::path path);
    BOOL getClusterSize();
    BOOL getRetrievalPointersBuffer(DefragObj obj);
    BOOL getClusterCount(DefragObj obj);
    BOOL findFreeBlock(ULONG64* BeginLcn, ULONG64* EndLcn);
    BOOL moveFileClusters(DefragObj obj);
    DWORD getExtentCount();
    void setLCN(ULONG64 BeginLcn, ULONG64 EndLcn);
    QString getVolumeInfo();
};

