#include "astmf3107mainwindow.h"
#include "ui_astmf3107mainwindow.h"

ASTMF3107MainWindow::ASTMF3107MainWindow(QWidget *parent) :
  ui(new Ui::ASTMF3107MainWindow)
{
    ui->setupUi(this);
     qDebug()<<"ASTM Product : ASTMF3107 Created";
}

ASTMF3107MainWindow::~ASTMF3107MainWindow()
{
    delete ui;
}
