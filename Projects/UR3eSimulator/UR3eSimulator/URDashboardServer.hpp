#ifndef URDASHBOARDSERVER_HPP
#define URDASHBOARDSERVER_HPP


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


class URDashboardServer : public QDialog
{
    Q_OBJECT

public:
    explicit URDashboardServer(QWidget *parent = Q_NULLPTR);
  public slots:
    void sessionOpened();
    void handleNewConnection();
    void disconnectFromServer();
    void readClientData();

signals:
    void sigDashboardCommand(QString p_command);
private:
    QLabel *statusLabel;
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
    QTcpSocket *clientConnection;
};
#endif // URDASHBOARDSERVER_HPP
