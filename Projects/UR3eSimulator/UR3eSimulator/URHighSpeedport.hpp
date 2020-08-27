#ifndef PORT30003_HPP
#define PORT30003_HPP


#include <stdint.h>

#include <QDialog>
#include <QTimer>
#include <QTcpSocket>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
#include <URLowSpeedport.hpp>
QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QTcpServer;
class QNetworkSession;
QT_END_NAMESPACE

//! [0]
class URHighSpeedport : public QDialog
{
    Q_OBJECT

public:
    explicit URHighSpeedport(QWidget *parent = Q_NULLPTR);
    void setPort(quint16 port)
    {
        m_port = port;
    }
    void initDummyData();
public slots:
    void sessionOpened();
    void handleNewConnection();
    void sendRealTimeData();
    void disconnectFromServer();
    void readClientData();

signals:
    /* Description: Signal triggered when program running status is changed.
     * param[in]: p_status
     * return:  N/A  */
    void sigProgramRunningStatusChanged(bool p_status);
    void signewTCP(URLowSpeedport::UR3eTCP newTCP);
private:
    QLabel *statusLabel;
    QTcpServer *tcpServer;
    QStringList fortunes;
    QNetworkSession *networkSession;
    quint16 m_port;
    QTimer *timer;
    QTcpSocket *clientConnection;
    uint8_t DummyData[1115];
    QFile mClientFile;
    double ur3X_m;      // Linear displacement by UR Robot in X axis- current TCP
    double ur3Y_m;      // Linear displacement by UR Robot in Y axis- current TCP
    double ur3Z_m;      // Linear displacement by UR Robot in Z axis- current TCP
    double ur3RX_rad;   // Angular displacement by UR Robot in X axis- current TCP
    double ur3RY_rad;   // Angular displacement by UR Robot in Y axis- current TCP
    double ur3RZ_rad;   // Angular displacement by UR Robot in Z axis- current TCP
};
#endif // PORT30003_HPP
