#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "header.h"
#include "defragobj.h"
#include "defragmenter.h"
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	friend fs::path validatePath(QString);
public slots:
	void OpenButtonClicked();
	void DefragButtonClicked();
	void OpenFolderButtonClicked();
private:
	Ui::MainWindowClass ui;
	fs::path path;
};

