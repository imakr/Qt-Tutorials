#include "URDashboardServer.hpp"

#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>


URDashboardServer::URDashboardServer(QWidget *parent)
    : QDialog(parent)
    , statusLabel(new QLabel)
    , tcpServer(Q_NULLPTR)
    , networkSession(nullptr)
    , clientConnection (nullptr)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, &QNetworkSession::opened, this, &URDashboardServer::sessionOpened);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    } else {
        sessionOpened();
    }
    QPushButton *quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    //! [3]
    connect(tcpServer, &QTcpServer::newConnection, this, &URDashboardServer::handleNewConnection);
    //! [3]
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = Q_NULLPTR;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        QVBoxLayout *outerVerticalLayout = new QVBoxLayout(this);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
        QHBoxLayout *outerHorizontalLayout = new QHBoxLayout;
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        QGroupBox *groupBox = new QGroupBox(QGuiApplication::applicationDisplayName());
        mainLayout = new QVBoxLayout(groupBox);
        outerHorizontalLayout->addWidget(groupBox);
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        outerVerticalLayout->addLayout(outerHorizontalLayout);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
    } else {
        mainLayout = new QVBoxLayout(this);
    }

    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setWindowTitle(QGuiApplication::applicationDisplayName());
}

void URDashboardServer::sessionOpened()
{
    // Save the used configuration
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer(this);
    QHostAddress address("127.0.0.1");
    if (!tcpServer->listen(address,29999)) {
        QMessageBox::critical(this, tr("UR3e Dashboard Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }
    statusLabel->setText(tr("The UR3e Dashboard server is running on\n\nIP: %1\nport: %2\n\n")
                         .arg("127.0.0.1").arg(tcpServer->serverPort()));
}

void URDashboardServer::handleNewConnection()
{
    qDebug() << "URDashboardServer - handleNewConnection Detected.. ";
    if(clientConnection == nullptr)
    {
        clientConnection = tcpServer->nextPendingConnection();
        connect(clientConnection, &QAbstractSocket::disconnected,
                this, &URDashboardServer::disconnectFromServer);
        connect(clientConnection,SIGNAL(readyRead()),this,SLOT(readClientData()));
    }else
    {
        qDebug() << "URDashboardServer - handleNewConnection - Already a client connection is in active state.. ";
    }
}
void URDashboardServer::disconnectFromServer()
{
    clientConnection->disconnectFromHost();
    clientConnection = nullptr;
}

void URDashboardServer::readClientData()
{
    QByteArray clientdata = clientConnection->readAll();
    qDebug()<<"URDashboardServer -  readClientData " << clientdata.toStdString().c_str();
    QString rcvdData(clientdata.toStdString().c_str());
    qDebug()<<"URDashboardServer -  readClientData - command rcvd :" <<rcvdData;
    emit sigDashboardCommand(rcvdData);
}
