#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_lowSpeedServer(nullptr),
    m_highSpeedServer(nullptr),
    m_dashboardServer(nullptr)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete m_lowSpeedServer;
    delete m_highSpeedServer;
    delete m_dashboardServer;
    delete ui;
}

void MainWindow::on_URInputPort1_clicked()
{
    if(m_lowSpeedServer == nullptr)
    {
        m_lowSpeedServer = new URLowSpeedport();
        m_lowSpeedServer->show();
        m_lowSpeedServer->exec();
    }else {
        QMessageBox::information(this,"UR3e Simulator","Low speed server is running already!");
    }
}

void MainWindow::on_URInputPort2_clicked()
{
    if(m_lowSpeedServer==nullptr)
    {
        QMessageBox::information(this,"UR3e Simulator","Start Low speed server first!");
        return;
    }
    if(m_highSpeedServer == nullptr)
    {
        m_highSpeedServer = new URHighSpeedport();

        connect(m_highSpeedServer,SIGNAL(sigProgramRunningStatusChanged(bool)),m_lowSpeedServer,SLOT(handleProgrammingStatusChange(bool)));
        connect(m_highSpeedServer,SIGNAL(signewTCP(URLowSpeedport::UR3eTCP)),this,SLOT(setTCP(URLowSpeedport::UR3eTCP)));
        m_highSpeedServer->show();
        m_highSpeedServer->exec();
    }else {
        QMessageBox::information(this,"UR3e Simulator","High speed server is running already!");
    }
}

void MainWindow::on_sendNewTCP_clicked()
{
    if(m_lowSpeedServer==nullptr)
    {
        QMessageBox::information(this,"UR3e Simulator","Start Low speed server first!");
        return;
    }
    URLowSpeedport::UR3eTCP newTCP;
    newTCP.ur3X_m = ui->x_tcp->text().toDouble();
    newTCP.ur3Y_m = ui->y_tcp->text().toDouble();
    newTCP.ur3Z_m = ui->z_tcp->text().toDouble();

    newTCP.ur3RX_rad = ui->rx_tcp->text().toDouble();
    newTCP.ur3RY_rad = ui->ry_tcp->text().toDouble();
    newTCP.ur3RZ_rad = ui->rz_tcp->text().toDouble();
    m_lowSpeedServer->setTCP(newTCP);
}
void MainWindow::setTCP(URLowSpeedport::UR3eTCP newTCP)
{
    m_lowSpeedServer->setTCP(newTCP);
}

void MainWindow::on_URDashboardPort_clicked()
{
    if(m_lowSpeedServer == nullptr)
    {
        QMessageBox::information(this,"UR3e Simulator","Start Low speed server first!");
        return;
    }
    if(m_highSpeedServer == nullptr)
    {
        QMessageBox::information(this,"UR3e Simulator","Start High speed server first!");
        return;
    }
    if(m_dashboardServer == nullptr)
    {
    m_dashboardServer = new URDashboardServer();
    connect(m_dashboardServer,SIGNAL(sigDashboardCommand(QString)),this,SLOT(handleDashboardCommand(QString)));
    m_dashboardServer->show();
    m_dashboardServer->exec();
    }else {
        QMessageBox::information(this,"UR3e Simulator","Dashboard server is running already!");
    }
}

void MainWindow::handleDashboardCommand(QString p_command)
{
    if(m_highSpeedServer == nullptr)
    {
        QMessageBox::information(this,"UR3e Simulator","Start High speed server first!");
        return;
    }
    qDebug()<<"MainWindow - handleDashboardCommand - Command rcvd = " << p_command;
    if(p_command.contains("play"))
    {
        m_highSpeedServer->setURMovementState(0);
    }else if(p_command.contains("pause"))
    {
        m_highSpeedServer->setURMovementState(1);
    }else if(p_command.contains("stop"))
    {
        m_highSpeedServer->setURMovementState(2);
    }
}
