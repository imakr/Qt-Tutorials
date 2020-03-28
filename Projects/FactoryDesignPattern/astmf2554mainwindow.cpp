#include "astmf2554mainwindow.h"
#include "ui_astmf2554mainwindow.h"

ASTMF2554MainWindow::ASTMF2554MainWindow(QWidget *parent) :
   ui(new Ui::ASTMF2554MainWindow)
{
    ui->setupUi(this);
     qDebug()<<"ASTM Product : ASTMF2554 Created";
}

ASTMF2554MainWindow::~ASTMF2554MainWindow()
{
    delete ui;
}
