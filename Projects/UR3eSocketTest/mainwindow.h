#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QFile>
#include <QTextStream>
#define RTD_SIZE_PORT 1116
#define RT_SIZE_PORT 716

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_Button_ConnectToUR_clicked();

    void on_Button_DisconnectUR_clicked();

    void mReadURRealTimeData();
    void readURRTData();
private:
    Ui::MainWindow *ui;
    uint64_t m_expectedPacketSize;
    QTcpSocket *m_urRTSocket;
    QTcpSocket *m_urRTDSocket;
     unsigned char mc_rtde_read_data[2*RTD_SIZE_PORT];
};

#endif // MAINWINDOW_H
