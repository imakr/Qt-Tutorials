#ifndef URHIGHSPEEDSERVER_HPP
#define URHIGHSPEEDSERVER_HPP


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


class URHighSpeedport : public QDialog
{
    Q_OBJECT

public:
    explicit URHighSpeedport(QWidget *parent = Q_NULLPTR);
    void initDummyData();
    void setURMovementState(int p_value);
public slots:
    void sessionOpened();
    void handleNewConnection();
    void sendRealTimeData();
    void disconnectFromServer();
    void readClientData();

signals:
    void sigProgramRunningStatusChanged(bool p_status);
    void signewTCP(URLowSpeedport::UR3eTCP newTCP);
private:
    QLabel *statusLabel;
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
    quint16 m_port;
    QTimer *timer;
    QTcpSocket *clientConnection;
    uint8_t DummyData[1116];
    QFile mClientFile;
    bool mIsUR3eProgramExecutionInProgress;
    double ur3X_m;      // Linear displacement by UR Robot in X axis- current TCP
    double ur3Y_m;      // Linear displacement by UR Robot in Y axis- current TCP
    double ur3Z_m;      // Linear displacement by UR Robot in Z axis- current TCP
    double ur3RX_rad;   // Angular displacement by UR Robot in X axis- current TCP
    double ur3RY_rad;   // Angular displacement by UR Robot in Y axis- current TCP
    double ur3RZ_rad;   // Angular displacement by UR Robot in Z axis- current TCP

};
#endif // URHIGHSPEEDSERVER_HPP
