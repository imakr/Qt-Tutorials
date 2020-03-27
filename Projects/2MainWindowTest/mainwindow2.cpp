#include "mainwindow2.h"
#include "ui_mainwindow2.h"

MainWindow2::MainWindow2(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow2)
{
    ui->setupUi(this);
}

MainWindow2::~MainWindow2()
{
    delete ui;
}

void MainWindow2::on_hitme_clicked()
{
    qDebug()<<"Hit Me Button Clicked";

  ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow2::on_pushButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
