#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    s1(nullptr),
    s2(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_URInputPort1_clicked()
{
    s1 = new URLowSpeedport();
    s1->show();
    s1->exec();
}

void MainWindow::on_URInputPort2_clicked()
{
    s2 = new URHighSpeedport();
    if(s1!=nullptr)
    {
        connect(s2,SIGNAL(sigProgramRunningStatusChanged(bool)),s1,SLOT(handleProgrammingStatusChange(bool)));
        connect(s2,SIGNAL(signewTCP(URLowSpeedport::UR3eTCP)),this,SLOT(setTCP(URLowSpeedport::UR3eTCP)));
    }
    s2->show();
    s2->exec();

}

void MainWindow::on_sendNewTCP_clicked()
{
    URLowSpeedport::UR3eTCP newTCP;
    newTCP.ur3X_m = ui->x_tcp->text().toDouble();
    newTCP.ur3Y_m = ui->y_tcp->text().toDouble();
    newTCP.ur3Z_m = ui->z_tcp->text().toDouble();

    newTCP.ur3RX_rad = ui->rx_tcp->text().toDouble();
    newTCP.ur3RY_rad = ui->ry_tcp->text().toDouble();
    newTCP.ur3RZ_rad = ui->rz_tcp->text().toDouble();
    s1->setTCP(newTCP);
}
void MainWindow::setTCP(URLowSpeedport::UR3eTCP newTCP)
{
      s1->setTCP(newTCP);
}
