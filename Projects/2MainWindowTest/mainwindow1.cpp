#include "mainwindow1.h"
#include "ui_mainwindow1.h"

MainWindow1::MainWindow1(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow1)
{
    ui->setupUi(this);
}

MainWindow1::~MainWindow1()
{
    delete ui;
}

void MainWindow1::on_nextmainwindow_clicked()
{
    qDebug()<<"Button Clicked...";
    this->hide();
    //Solution - 1 : Have the second mainwindow as a member variable and call show() as shown below
    mw2.show();
    //Solution - 2 : Create a variable on heap as shown below. But this causes memory leak.
    //MainWindow2 *qmw2 = new MainWindow2();
    //qmw2->show();
     qDebug()<<"Button Click exited...";
}
