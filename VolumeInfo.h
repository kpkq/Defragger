#pragma once
#include "header.h"

class VolumeInfo
{
private:
    DWORD SectorsPerCluster = 0;
    DWORD BytesPerSector = 0;
    DWORD NumberOfFreeClusters = 0;
    DWORD TotalNumberOfClusters = 0;
public:
    VolumeInfo();
    VolumeInfo(DWORD SectorsPerCluster, DWORD BytesPerSector, DWORD NumberOfFreeClusters, DWORD TotalNumberOfClusters);
    DWORD getSectorsPerCluster();
    DWORD getBytesPerSector();
    DWORD getNumberOfFreeClusters();
    DWORD getTotalNumberOfClusters();
};