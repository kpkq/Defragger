#include "defragmenter.h"
#include <iostream>
Defragmenter::Defragmenter() {}

BOOL Defragmenter::setVolumeName(fs::path path)
{
    sprintf(this->volumeName, "\\\\.\\%c:", path.c_str()[0]);
    this->hDisk = CreateFile(volumeName, GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk == INVALID_HANDLE_VALUE)
        return 0;
    return 1;

}

BOOL Defragmenter::getClusterSize()
{
    DWORD SectorsPerCluster = 0, BytesPerSector = 0, NumberOfFreeClusters = 0, TotalNumberOfClusters = 0, ClusterSize = 0;
    strncat(volumeName, "\\", 1);
    if (GetDiskFreeSpaceA(volumeName,
        &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters))
    {
        this->clusterSize = SectorsPerCluster * BytesPerSector;
        VolumeInfo info(BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters, ClusterSize);
        this->info = info;
        return 1;
    }
    else return 0;
}

BOOL Defragmenter::getRetrievalPointersBuffer(DefragObj obj)
{
    ULONG OutSize;
    STARTING_VCN_INPUT_BUFFER  InBuf;
    PRETRIEVAL_POINTERS_BUFFER OutBuf;
    OutSize = (ULONG)sizeof(RETRIEVAL_POINTERS_BUFFER) + (obj.getFileSize().QuadPart / this->clusterSize) * sizeof(OutBuf->Extents);
    OutBuf = (PRETRIEVAL_POINTERS_BUFFER)malloc(OutSize);
    InBuf.StartingVcn.QuadPart = 0;
    ULONG   Bytes;
    if (!DeviceIoControl(obj.getFileHandle(), FSCTL_GET_RETRIEVAL_POINTERS, &InBuf, sizeof(InBuf), OutBuf, OutSize, &Bytes, NULL))
        return 0;
    this->OutBuf = OutBuf;
    return 1;

}

DWORD Defragmenter::getExtentCount()
{
    return this->OutBuf->ExtentCount;
}

BOOL Defragmenter::getClusterCount(DefragObj obj)
{
    this->clusterCount = (obj.getFileSize().QuadPart + this->clusterSize - 1) / this->clusterSize;
    return 1;
}

BOOL Defragmenter::findFreeBlock(ULONG64* BeginLcn, ULONG64* EndLcn)
{   STARTING_LCN_INPUT_BUFFER InBuffer;
    struct {
        ULONG64 StartingLcn;
        ULONG64 BitmapSize;
        BYTE Buffer[32768];           /* Most efficient if binary multiple. */
    } Data;
    ULONG64 MaxLcn;
    struct {
        ULONG64 Start;
        ULONG64 End;
    } Excludes[3];
    ULONG64 Lcn;
    ULONG64 ClusterStart;
    int Index;
    int IndexMax;
    BYTE Mask;
    int InUse;
    int PrevInUse;
    int Result;
    DWORD w;
    ULONG64 MinimumLcn = 0;
    DWORD MinimumSize = this->clusterCount;
    MaxLcn = 0;
    Excludes[0].Start = 0;
    Excludes[0].End = 0;
    Excludes[1].Start = 0;
    Excludes[1].End = 0;
    Excludes[2].Start = 0;
    Excludes[2].End = 0;
    /* Main loop to walk through the entire clustermap. */
    Lcn = MinimumLcn;
    ClusterStart = 0;
    PrevInUse = 1;
    do {

        /* Sanity check. */
        if ((MaxLcn > 0) && (Lcn >= MaxLcn)) return 0;

        /* Fetch a block of cluster data. */
        InBuffer.StartingLcn.QuadPart = Lcn;
        Result = DeviceIoControl(this->hDisk , FSCTL_GET_VOLUME_BITMAP,
            &InBuffer, sizeof(InBuffer),
            &Data, sizeof(Data),
            &w, NULL);
        if (Result == 0) {
            Result = GetLastError();
            if (Result != ERROR_MORE_DATA) {
                return 0;
            }
        }

        /* Analyze the clusterdata. We resume where the previous block left
        off. If a cluster is found that matches the criteria then return
        it's LCN (Logical Cluster Number). */
        Lcn = Data.StartingLcn;
        Index = 0;
        Mask = 1;
        IndexMax = sizeof(Data.Buffer);
        if (Data.BitmapSize / 8 < IndexMax) IndexMax = (int)(Data.BitmapSize / 8);
        while (Index < IndexMax) {
            InUse = (Data.Buffer[Index] & Mask);
            if (((Lcn >= Excludes[0].Start) && (Lcn < Excludes[0].End)) ||
                ((Lcn >= Excludes[1].Start) && (Lcn < Excludes[1].End)) ||
                ((Lcn >= Excludes[2].Start) && (Lcn < Excludes[2].End))) {
                InUse = 1;
            }
            if ((PrevInUse == 0) && (InUse != 0)) {

                if ((ClusterStart >= MinimumLcn) &&
                    (Lcn - ClusterStart >= MinimumSize)) {
                    *BeginLcn = ClusterStart;
                    if (EndLcn != NULL) *EndLcn = Lcn;
                    return 1;
                }
            }
            if ((PrevInUse != 0) && (InUse == 0)) ClusterStart = Lcn;
            PrevInUse = InUse;
            if (Mask == 128) {
                Mask = 1;
                Index = Index + 1;
            }
            else {
                Mask = Mask << 1;
            }
            Lcn = Lcn + 1;
        }

    } while ((Result == ERROR_MORE_DATA) &&
        (Lcn < Data.StartingLcn + Data.BitmapSize));

    if (PrevInUse == 0) {
        if ((ClusterStart >= MinimumLcn) &&
            (Lcn - ClusterStart >= MinimumSize)) {
            *BeginLcn = ClusterStart;
            if (EndLcn != NULL) *EndLcn = Lcn;
            return 1;
        }
    }

    return 0;
}


BOOL Defragmenter::moveFileClusters(DefragObj obj)
{
    MOVE_FILE_DATA MoveParams;
    MoveParams.FileHandle = obj.getFileHandle();
    MoveParams.StartingLcn.QuadPart = this->beginLCN;
    MoveParams.StartingVcn.QuadPart = 0;
    LARGE_INTEGER PrevVCN = OutBuf->StartingVcn;
    for (ULONG r = 0; r < OutBuf->ExtentCount; r++)
    {
        if (OutBuf->ExtentCount == 1) break;
        DWORD br;
        MoveParams.ClusterCount = OutBuf->Extents[r].NextVcn.QuadPart - PrevVCN.QuadPart;
        if (!DeviceIoControl(hDisk, FSCTL_MOVE_FILE, &MoveParams, sizeof(MoveParams), NULL, 0, &br, NULL))
            return 0;
        MoveParams.StartingLcn.QuadPart = MoveParams.StartingLcn.QuadPart + MoveParams.ClusterCount;
        MoveParams.StartingVcn.QuadPart = MoveParams.StartingVcn.QuadPart + MoveParams.ClusterCount;
        PrevVCN = OutBuf->Extents[r].NextVcn;
    }
    return 1;
}

void Defragmenter::setLCN(ULONG64 BeginLcn, ULONG64 EndLcn)
{
    ULONG64 tmp = BeginLcn, tmp2 = EndLcn;
    this->beginLCN = tmp;
    this->endLCN = tmp2;
}

QString Defragmenter::getVolumeInfo()
{
    QString tmp;
    tmp.append("-----------------------------------------------\n").append("Sectors per cluster: ").append(QString::number(this->info.getSectorsPerCluster()))
        .append("\nBytes per sector: ").append(QString::number(this->info.getBytesPerSector()))
            .append("\nNumber of free clusters : ").append(QString::number(this->info.getNumberOfFreeClusters()))
                 .append("\nCluster size: ").append(QString::number(clusterSize)).append("\n-----------------------------------------------\n");
    return tmp;
}


