#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    try
    {
        m_urRTSocket = new QTcpSocket();
        m_urRTDSocket = new QTcpSocket();
    }
    catch (const std::bad_alloc& ba)
    {
        delete m_urRTSocket;
        delete m_urRTDSocket;
        qDebug() << "URDMGR:bad_alloc caught: " << ba.what() << '\n';
    }
    catch(...)
    {
        qDebug() << "URDMGR:bad_alloc caught: " << '\n';
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Button_ConnectToUR_clicked()
{
    QString urIPFromLineEdit = ui->lineEdit_URIP->text();
    m_urRTSocket->setReadBufferSize(RT_SIZE_PORT);
    m_urRTDSocket->setReadBufferSize(2*RTD_SIZE_PORT);
    if(ui->urport2_radioButton->isChecked())
    {
        connect(m_urRTDSocket,SIGNAL(readyRead()),this,SLOT(mReadURRealTimeData()));
        m_urRTDSocket->connectToHost(urIPFromLineEdit, 30003);
    }else
    {
        connect(m_urRTSocket,SIGNAL(readyRead()),this,SLOT(readURRTData()));
        m_urRTSocket->connectToHost(urIPFromLineEdit,30002);
    }
}

void MainWindow::on_Button_DisconnectUR_clicked()
{
    m_urRTDSocket->close();
    m_urRTSocket->close();
}

void MainWindow::mReadURRealTimeData()
{
    qint64 BytesAvailable= m_urRTDSocket->bytesAvailable();
    qDebug()<<"RTDECW1 : BytesAvail="<<BytesAvailable;
    QByteArray socketdata = m_urRTDSocket->readAll();
    if(socketdata.size() >=RTD_SIZE_PORT)
    {
        QFile file("30003Log.txt");
        file.open(QIODevice::Append | QFile::Text);
        QTextStream InData(&file);
        InData <<"Size:"<<socketdata.size()<<endl<<"QB="<<socketdata;
        for(int i = 0;i<1116;i++)
        {
            mc_rtde_read_data[i]=(uint8_t)socketdata[i];
            InData <<"["<<i<<"]:"<<mc_rtde_read_data[i];
        }
        InData << "\n";
        InData.flush();
        file.close();
    }
}

void MainWindow::readURRTData()
{
    uchar ucReadBuffer[RT_SIZE_PORT]={};// 716 for SW.Ver: 5.4
    int64_t llBytesAvailabe=m_urRTSocket->bytesAvailable();
    QByteArray abReplyBuffer = m_urRTSocket->readAll();
    if ( llBytesAvailabe != RT_SIZE_PORT )// 716 for SW.Ver: 5.4
    {
        return;
    }

    QFile file("30002Log.txt");
    file.open(QIODevice::Append | QFile::Text);
    QTextStream InData(&file);
    InData <<"Size:"<<abReplyBuffer.size()<<endl<<"QB="<<abReplyBuffer;
    for(int iBufIndex = 0; iBufIndex < RT_SIZE_PORT; iBufIndex++)
    {
        ucReadBuffer[iBufIndex] = static_cast<uint8_t>(abReplyBuffer[iBufIndex]);
        InData <<"["<<iBufIndex<<"]:"<<ucReadBuffer[iBufIndex];
    }
    InData << "\n";
    InData.flush();
    file.close();
}
