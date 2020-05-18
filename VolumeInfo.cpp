#include "volumeinfo.h"
VolumeInfo::VolumeInfo() {}
VolumeInfo::VolumeInfo(DWORD SectorsPerCluster, DWORD BytesPerSector, DWORD NumberOfFreeClusters, DWORD TotalNumberOfClusters)
{
    this->SectorsPerCluster = SectorsPerCluster;
    this->BytesPerSector = BytesPerSector;
    this->NumberOfFreeClusters = NumberOfFreeClusters;
    this->TotalNumberOfClusters = TotalNumberOfClusters;
}

DWORD VolumeInfo::getSectorsPerCluster()
{
    return  this->SectorsPerCluster;
}

DWORD VolumeInfo::getBytesPerSector()
{
    return  this->BytesPerSector;
}

DWORD VolumeInfo::getNumberOfFreeClusters()
{
    return  this->NumberOfFreeClusters;
}

DWORD VolumeInfo::getTotalNumberOfClusters()
{
    return  this->TotalNumberOfClusters;
}
