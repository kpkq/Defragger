#include "MainWindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    this->setWindowTitle("MyDefragger");
}

fs::path validatePath(QString path)
{
    std::string tmp(path.toStdString());
    while (1)
    {
        int index = tmp.find('/', index);
        if (index == std::string::npos) break;
        tmp.replace(index, 1, "\\");
    }
    return tmp;
}

void MainWindow::OpenButtonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this);
    this->path = validatePath(filePath);
}

void MainWindow::OpenFolderButtonClicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this);
    this->path = validatePath(filePath);
}

void MainWindow::DefragButtonClicked()
{
    fs::path path = this->path;
    std::vector<DefragObj> fileList;
    if (path.empty())
    {
        ui.textBrowser->append("File not defined\n");
        return;
    }
    if (fs::is_directory(path))
    {
        for (auto& p : fs::recursive_directory_iterator(path))
        {
            if(fs::is_directory(p)) continue;
            DefragObj dObj(p);
            fileList.push_back(dObj);
        }
    }
    else
    {
        DefragObj dObj(path);
        fileList.push_back(dObj);
    }
    for (int i = 0; i < fileList.size(); i++)
    {
        Defragmenter defragger;
        if (fileList[i].openFile())
        {
            ui.textBrowser->append("File ");
            ui.textBrowser->append(QString::fromStdString(fileList[i].getFileName()));
            ui.textBrowser->append(" opened\n");
        }
        else
            ui.textBrowser->append("File open error\n");
        if (!fileList[i].findFileSize()) ui.textBrowser->append("Get file size error\n");
        defragger.setVolumeName(path);
        defragger.getClusterSize();
        defragger.getClusterCount(fileList[i]);
        defragger.getRetrievalPointersBuffer(fileList[i]);
        if(!i) ui.textBrowser->append(defragger.getVolumeInfo());
        if (defragger.getExtentCount() == 1)
        {
            ui.textBrowser->append("File isn't fragmented\n");
            continue;
        }
        else
        {
            ui.textBrowser->append("File is divided into ");
            ui.textBrowser->append(QString::number((int)defragger.getExtentCount()));
            ui.textBrowser->append(" fragments\n");
        }
        ULONG64 BLcn = -1, ELcn = -1;
        if (defragger.findFreeBlock(&BLcn, &ELcn))
            ui.textBrowser->append("Free block found successfully \n");
        else
            ui.textBrowser->append("Error while finding free block\n");
        defragger.setLCN(BLcn, ELcn);
        if (defragger.moveFileClusters(fileList[i]))
            ui.textBrowser->append("File defragmented successfully \n");
        else
            ui.textBrowser->append("Error while file defragmenting\n");
    }
}
