#include "URHighSpeedport.hpp"

#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>


URHighSpeedport::URHighSpeedport(QWidget *parent)
    : QDialog(parent)
    , statusLabel(new QLabel)
    , tcpServer(Q_NULLPTR)
    , networkSession(nullptr)
    , clientConnection (nullptr)
    , mIsUR3eProgramExecutionInProgress(false)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    statusLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    timer = new QTimer(this);
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
        connect(networkSession, &QNetworkSession::opened, this, &URHighSpeedport::sessionOpened);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    } else {
        sessionOpened();
    }
    connect(timer,SIGNAL(timeout()),this,SLOT(sendRealTimeData()));

    QPushButton *quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    //! [3]
    connect(tcpServer, &QTcpServer::newConnection, this, &URHighSpeedport::handleNewConnection);
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
    initDummyData();
    setWindowTitle(QGuiApplication::applicationDisplayName());
}

void URHighSpeedport::sessionOpened()
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
    if (!tcpServer->listen(address,30003)) {
        QMessageBox::critical(this, tr("UR3e high speed Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }
    statusLabel->setText(tr("The UR3e high speed is running on\n\nIP: %1\nport: %2\n\n")
                         .arg("127.0.0.1").arg(tcpServer->serverPort()));
}

void URHighSpeedport::handleNewConnection()
{
    qDebug() << "URHighSpeedport - handleNewConnection Detected.. ";
    if(clientConnection == nullptr)
    {
        clientConnection = tcpServer->nextPendingConnection();
        connect(clientConnection, &QAbstractSocket::disconnected,
                this, &URHighSpeedport::disconnectFromServer);
        connect(clientConnection,SIGNAL(readyRead()),this,SLOT(readClientData()));
    }else
    {
        qDebug() << "URHighSpeedport - handleNewConnection - Already a client connection is in active state.. ";
    }
}
void URHighSpeedport::disconnectFromServer()
{
    clientConnection->disconnectFromHost();
    clientConnection = nullptr;
}

void URHighSpeedport::sendRealTimeData()
{
    qDebug() << "URHighSpeedport - sendRealTimeData = ";
    if(!mClientFile.atEnd())
    {
        char m_urCmd[1024];
        memset(m_urCmd,'\0',1024);
        qint64 bytesRead = mClientFile.readLine(m_urCmd,1024);
        qDebug() << "URHighSpeedport - m_urCmd = " << m_urCmd;
        qDebug() << "URHighSpeedport - readline size = " << bytesRead;
        qDebug() << "URHighSpeedport - current file ptr pos = " << mClientFile.pos();
        QString readline(m_urCmd);
        if(readline.contains("move"))
        {
            QStringList splitdata = readline.split(",");
            URLowSpeedport::UR3eTCP newTCP;
            newTCP.ur3Y_m = splitdata[1].toDouble();
            newTCP.ur3Z_m = splitdata[2].toDouble();
            newTCP.ur3RX_rad = splitdata[3].toDouble();
            newTCP.ur3RY_rad = splitdata[4].toDouble();

            QStringList tcp = splitdata[0].split("[");
            newTCP.ur3X_m = tcp[1].toDouble();
            tcp.clear();
            tcp = splitdata[5].split("]");
            newTCP.ur3RZ_rad = tcp[0].toDouble();
            qDebug() << "URHighSpeedport - readline = Emit new TCP";
            emit signewTCP(newTCP);
        }
    }else
    {
        mClientFile.close();
        qDebug() << "URHighSpeedport - readline = EOF Reached";
        emit sigProgramRunningStatusChanged(false);
        timer->stop();
        mIsUR3eProgramExecutionInProgress = false;
    }
}
void URHighSpeedport::readClientData()
{
    QByteArray clientdata = clientConnection->readAll();
    qDebug()<<"URHighSpeedport -  readClientData " << clientdata.toStdString().c_str();
    QString rcvdData(clientdata.toStdString().c_str());
    qDebug()<<"URHighSpeedport -  readClientData - Script start compare :" <<rcvdData.compare("def RCTB_Test_UR_Script():");
    qDebug()<<"URHighSpeedport -  readClientData - Script end compare :" <<rcvdData.compare("end");
    if(rcvdData.contains("def RCTB_Test_UR_Script():"))
    {
        qDebug()<<"URHighSpeedport -  readClientData - Script start detected :" ;
        QFile file("clientdata.txt");
        file.open( QIODevice::WriteOnly | QFile::Text );
        QTextStream inData( &file );
        inData << clientdata.toStdString().c_str();
        file.close();
    }else
    {
        qDebug()<<"URHighSpeedport -  readClientData - Script body detected :" ;
        QFile file("clientdata.txt");
        file.open( QIODevice::Append | QFile::Text );
        QTextStream inData( &file );
        inData << rcvdData;
        file.close();
        if(rcvdData.contains("end"))
        {
            qDebug()<<"URHighSpeedport -  readClientData - Script end detected :" ;
            emit sigProgramRunningStatusChanged(true);
            mIsUR3eProgramExecutionInProgress = true;
            timer->start(100);
            mClientFile.setFileName("clientdata.txt");
            mClientFile.open(QIODevice::ReadOnly | QFile::Text);
            qDebug()<<"URHighSpeedport -  readClientData - clientdata file size = " <<mClientFile.size();  ;
            qDebug() << "URHighSpeedport - readClientData - pos = " << mClientFile.pos();
        }
    }
}

void URHighSpeedport::setURMovementState(int p_value)
{
    if(mIsUR3eProgramExecutionInProgress == true)
    {
        switch(p_value)
        {
        case 0:
        {
            qDebug() << "URHighSpeedport - setURMovementState = PLAY command received";
            timer->start(100);
            break;
        }
        case 1:
        {
            qDebug() << "URHighSpeedport - setURMovementState = PAUSE command received";
            timer->stop();
            break;
        }
        case 2:
        {
            mClientFile.close();
            qDebug() << "URHighSpeedport - setURMovementState = STOP command received";
            emit sigProgramRunningStatusChanged(false);
            timer->stop();
            mIsUR3eProgramExecutionInProgress = false;
            break;
        }
        default:
        {
            break;
        }
        }
    }
}
void URHighSpeedport::initDummyData()
{
    DummyData[0]=0;
    DummyData[1]=0;
    DummyData[2]=4;
    DummyData[3]=92;
    DummyData[4]=64;
    DummyData[5]=248;
    DummyData[6]=11;
    DummyData[7]=70;
    DummyData[8]=196;
    DummyData[9]=155;
    DummyData[10]=165;
    DummyData[11]=227;
    DummyData[12]=63;
    DummyData[13]=249;
    DummyData[14]=123;
    DummyData[15]=159;
    DummyData[16]=128;
    DummyData[17]=2;
    DummyData[18]=52;
    DummyData[19]=65;
    DummyData[20]=192;
    DummyData[21]=10;
    DummyData[22]=211;
    DummyData[23]=130;
    DummyData[24]=82;
    DummyData[25]=167;
    DummyData[26]=11;
    DummyData[27]=24;
    DummyData[28]=63;
    DummyData[29]=255;
    DummyData[30]=158;
    DummyData[31]=250;
    DummyData[32]=80;
    DummyData[33]=248;
    DummyData[34]=119;
    DummyData[35]=152;
    DummyData[36]=191;
    DummyData[37]=251;
    DummyData[38]=247;
    DummyData[39]=181;
    DummyData[40]=165;
    DummyData[41]=72;
    DummyData[42]=11;
    DummyData[43]=160;
    DummyData[44]=191;
    DummyData[45]=248;
    DummyData[46]=169;
    DummyData[47]=166;
    DummyData[48]=81;
    DummyData[49]=30;
    DummyData[50]=153;
    DummyData[51]=112;
    DummyData[52]=192;
    DummyData[53]=15;
    DummyData[54]=236;
    DummyData[55]=143;
    DummyData[56]=232;
    DummyData[57]=131;
    DummyData[58]=97;
    DummyData[59]=176;
    DummyData[60]=0;
    DummyData[61]=0;
    DummyData[62]=0;
    DummyData[63]=0;
    DummyData[64]=0;
    DummyData[65]=0;
    DummyData[66]=0;
    DummyData[67]=0;
    DummyData[68]=0;
    DummyData[69]=0;
    DummyData[70]=0;
    DummyData[71]=0;
    DummyData[72]=0;
    DummyData[73]=0;
    DummyData[74]=0;
    DummyData[75]=0;
    DummyData[76]=0;
    DummyData[77]=0;
    DummyData[78]=0;
    DummyData[79]=0;
    DummyData[80]=0;
    DummyData[81]=0;
    DummyData[82]=0;
    DummyData[83]=0;
    DummyData[84]=0;
    DummyData[85]=0;
    DummyData[86]=0;
    DummyData[87]=0;
    DummyData[88]=0;
    DummyData[89]=0;
    DummyData[90]=0;
    DummyData[91]=0;
    DummyData[92]=0;
    DummyData[93]=0;
    DummyData[94]=0;
    DummyData[95]=0;
    DummyData[96]=0;
    DummyData[97]=0;
    DummyData[98]=0;
    DummyData[99]=0;
    DummyData[100]=0;
    DummyData[101]=0;
    DummyData[102]=0;
    DummyData[103]=0;
    DummyData[104]=0;
    DummyData[105]=0;
    DummyData[106]=0;
    DummyData[107]=0;
    DummyData[108]=0;
    DummyData[109]=0;
    DummyData[110]=0;
    DummyData[111]=0;
    DummyData[112]=0;
    DummyData[113]=0;
    DummyData[114]=0;
    DummyData[115]=0;
    DummyData[116]=0;
    DummyData[117]=0;
    DummyData[118]=0;
    DummyData[119]=0;
    DummyData[120]=0;
    DummyData[121]=0;
    DummyData[122]=0;
    DummyData[123]=0;
    DummyData[124]=0;
    DummyData[125]=0;
    DummyData[126]=0;
    DummyData[127]=0;
    DummyData[128]=0;
    DummyData[129]=0;
    DummyData[130]=0;
    DummyData[131]=0;
    DummyData[132]=0;
    DummyData[133]=0;
    DummyData[134]=0;
    DummyData[135]=0;
    DummyData[136]=0;
    DummyData[137]=0;
    DummyData[138]=0;
    DummyData[139]=0;
    DummyData[140]=0;
    DummyData[141]=0;
    DummyData[142]=0;
    DummyData[143]=0;
    DummyData[144]=0;
    DummyData[145]=0;
    DummyData[146]=0;
    DummyData[147]=0;
    DummyData[148]=0;
    DummyData[149]=0;
    DummyData[150]=0;
    DummyData[151]=0;
    DummyData[152]=0;
    DummyData[153]=0;
    DummyData[154]=0;
    DummyData[155]=0;
    DummyData[156]=188;
    DummyData[157]=131;
    DummyData[158]=175;
    DummyData[159]=172;
    DummyData[160]=174;
    DummyData[161]=184;
    DummyData[162]=88;
    DummyData[163]=34;
    DummyData[164]=63;
    DummyData[165]=240;
    DummyData[166]=50;
    DummyData[167]=243;
    DummyData[168]=23;
    DummyData[169]=187;
    DummyData[170]=51;
    DummyData[171]=151;
    DummyData[172]=191;
    DummyData[173]=204;
    DummyData[174]=156;
    DummyData[175]=155;
    DummyData[176]=169;
    DummyData[177]=243;
    DummyData[178]=155;
    DummyData[179]=191;
    DummyData[180]=191;
    DummyData[181]=191;
    DummyData[182]=165;
    DummyData[183]=95;
    DummyData[184]=134;
    DummyData[185]=73;
    DummyData[186]=224;
    DummyData[187]=75;
    DummyData[188]=63;
    DummyData[189]=14;
    DummyData[190]=54;
    DummyData[191]=244;
    DummyData[192]=150;
    DummyData[193]=192;
    DummyData[194]=102;
    DummyData[195]=170;
    DummyData[196]=191;
    DummyData[197]=2;
    DummyData[198]=193;
    DummyData[199]=127;
    DummyData[200]=237;
    DummyData[201]=0;
    DummyData[202]=34;
    DummyData[203]=219;
    DummyData[204]=188;
    DummyData[205]=185;
    DummyData[206]=161;
    DummyData[207]=126;
    DummyData[208]=79;
    DummyData[209]=128;
    DummyData[210]=0;
    DummyData[211]=0;
    DummyData[212]=64;
    DummyData[213]=37;
    DummyData[214]=16;
    DummyData[215]=153;
    DummyData[216]=236;
    DummyData[217]=208;
    DummyData[218]=163;
    DummyData[219]=142;
    DummyData[220]=191;
    DummyData[221]=252;
    DummyData[222]=164;
    DummyData[223]=44;
    DummyData[224]=250;
    DummyData[225]=1;
    DummyData[226]=187;
    DummyData[227]=112;
    DummyData[228]=191;
    DummyData[229]=220;
    DummyData[230]=178;
    DummyData[231]=51;
    DummyData[232]=214;
    DummyData[233]=129;
    DummyData[234]=237;
    DummyData[235]=123;
    DummyData[236]=63;
    DummyData[237]=43;
    DummyData[238]=52;
    DummyData[239]=234;
    DummyData[240]=49;
    DummyData[241]=169;
    DummyData[242]=127;
    DummyData[243]=169;
    DummyData[244]=191;
    DummyData[245]=32;
    DummyData[246]=240;
    DummyData[247]=217;
    DummyData[248]=155;
    DummyData[249]=221;
    DummyData[250]=58;
    DummyData[251]=40;
    DummyData[252]=63;
    DummyData[253]=249;
    DummyData[254]=123;
    DummyData[255]=147;
    DummyData[256]=0;
    DummyData[257]=0;
    DummyData[258]=0;
    DummyData[259]=0;
    DummyData[260]=192;
    DummyData[261]=10;
    DummyData[262]=211;
    DummyData[263]=130;
    DummyData[264]=82;
    DummyData[265]=177;
    DummyData[266]=124;
    DummyData[267]=216;
    DummyData[268]=63;
    DummyData[269]=255;
    DummyData[270]=159;
    DummyData[271]=19;
    DummyData[272]=209;
    DummyData[273]=16;
    DummyData[274]=180;
    DummyData[275]=96;
    DummyData[276]=191;
    DummyData[277]=251;
    DummyData[278]=247;
    DummyData[279]=206;
    DummyData[280]=165;
    DummyData[281]=98;
    DummyData[282]=249;
    DummyData[283]=176;
    DummyData[284]=191;
    DummyData[285]=248;
    DummyData[286]=169;
    DummyData[287]=178;
    DummyData[288]=209;
    DummyData[289]=16;
    DummyData[290]=180;
    DummyData[291]=96;
    DummyData[292]=192;
    DummyData[293]=15;
    DummyData[294]=236;
    DummyData[295]=150;
    DummyData[296]=104;
    DummyData[297]=136;
    DummyData[298]=90;
    DummyData[299]=48;
    DummyData[300]=0;
    DummyData[301]=0;
    DummyData[302]=0;
    DummyData[303]=0;
    DummyData[304]=0;
    DummyData[305]=0;
    DummyData[306]=0;
    DummyData[307]=0;
    DummyData[308]=0;
    DummyData[309]=0;
    DummyData[310]=0;
    DummyData[311]=0;
    DummyData[312]=0;
    DummyData[313]=0;
    DummyData[314]=0;
    DummyData[315]=0;
    DummyData[316]=128;
    DummyData[317]=0;
    DummyData[318]=0;
    DummyData[319]=0;
    DummyData[320]=0;
    DummyData[321]=0;
    DummyData[322]=0;
    DummyData[323]=0;
    DummyData[324]=128;
    DummyData[325]=0;
    DummyData[326]=0;
    DummyData[327]=0;
    DummyData[328]=0;
    DummyData[329]=0;
    DummyData[330]=0;
    DummyData[331]=0;
    DummyData[332]=0;
    DummyData[333]=0;
    DummyData[334]=0;
    DummyData[335]=0;
    DummyData[336]=0;
    DummyData[337]=0;
    DummyData[338]=0;
    DummyData[339]=0;
    DummyData[340]=0;
    DummyData[341]=0;
    DummyData[342]=0;
    DummyData[343]=0;
    DummyData[344]=0;
    DummyData[345]=0;
    DummyData[346]=0;
    DummyData[347]=0;
    DummyData[348]=63;
    DummyData[349]=174;
    DummyData[350]=65;
    DummyData[351]=203;
    DummyData[352]=64;
    DummyData[353]=0;
    DummyData[354]=0;
    DummyData[355]=0;
    DummyData[356]=63;
    DummyData[357]=240;
    DummyData[358]=10;
    DummyData[359]=197;
    DummyData[360]=224;
    DummyData[361]=0;
    DummyData[362]=0;
    DummyData[363]=0;
    DummyData[364]=191;
    DummyData[365]=191;
    DummyData[366]=96;
    DummyData[367]=62;
    DummyData[368]=224;
    DummyData[369]=0;
    DummyData[370]=0;
    DummyData[371]=0;
    DummyData[372]=191;
    DummyData[373]=197;
    DummyData[374]=130;
    DummyData[375]=25;
    DummyData[376]=32;
    DummyData[377]=0;
    DummyData[378]=0;
    DummyData[379]=0;
    DummyData[380]=63;
    DummyData[381]=168;
    DummyData[382]=115;
    DummyData[383]=209;
    DummyData[384]=0;
    DummyData[385]=0;
    DummyData[386]=0;
    DummyData[387]=0;
    DummyData[388]=191;
    DummyData[389]=159;
    DummyData[390]=39;
    DummyData[391]=51;
    DummyData[392]=0;
    DummyData[393]=0;
    DummyData[394]=0;
    DummyData[395]=0;
    DummyData[396]=188;
    DummyData[397]=131;
    DummyData[398]=175;
    DummyData[399]=172;
    DummyData[400]=174;
    DummyData[401]=184;
    DummyData[402]=88;
    DummyData[403]=34;
    DummyData[404]=63;
    DummyData[405]=240;
    DummyData[406]=50;
    DummyData[407]=243;
    DummyData[408]=23;
    DummyData[409]=187;
    DummyData[410]=51;
    DummyData[411]=151;
    DummyData[412]=191;
    DummyData[413]=204;
    DummyData[414]=156;
    DummyData[415]=155;
    DummyData[416]=169;
    DummyData[417]=243;
    DummyData[418]=155;
    DummyData[419]=191;
    DummyData[420]=191;
    DummyData[421]=191;
    DummyData[422]=165;
    DummyData[423]=95;
    DummyData[424]=134;
    DummyData[425]=73;
    DummyData[426]=224;
    DummyData[427]=75;
    DummyData[428]=63;
    DummyData[429]=14;
    DummyData[430]=54;
    DummyData[431]=244;
    DummyData[432]=150;
    DummyData[433]=192;
    DummyData[434]=102;
    DummyData[435]=170;
    DummyData[436]=191;
    DummyData[437]=2;
    DummyData[438]=193;
    DummyData[439]=127;
    DummyData[440]=237;
    DummyData[441]=0;
    DummyData[442]=34;
    DummyData[443]=219;
    DummyData[444]=63;
    DummyData[445]=193;
    DummyData[446]=172;
    DummyData[447]=111;
    DummyData[448]=184;
    DummyData[449]=100;
    DummyData[450]=89;
    DummyData[451]=159;
    DummyData[452]=191;
    DummyData[453]=110;
    DummyData[454]=36;
    DummyData[455]=80;
    DummyData[456]=235;
    DummyData[457]=215;
    DummyData[458]=232;
    DummyData[459]=192;
    DummyData[460]=63;
    DummyData[461]=216;
    DummyData[462]=255;
    DummyData[463]=27;
    DummyData[464]=161;
    DummyData[465]=236;
    DummyData[466]=232;
    DummyData[467]=208;
    DummyData[468]=63;
    DummyData[469]=247;
    DummyData[470]=98;
    DummyData[471]=236;
    DummyData[472]=217;
    DummyData[473]=3;
    DummyData[474]=190;
    DummyData[475]=197;
    DummyData[476]=63;
    DummyData[477]=230;
    DummyData[478]=166;
    DummyData[479]=148;
    DummyData[480]=179;
    DummyData[481]=175;
    DummyData[482]=31;
    DummyData[483]=26;
    DummyData[484]=191;
    DummyData[485]=228;
    DummyData[486]=12;
    DummyData[487]=155;
    DummyData[488]=240;
    DummyData[489]=41;
    DummyData[490]=244;
    DummyData[491]=183;
    DummyData[492]=0;
    DummyData[493]=0;
    DummyData[494]=0;
    DummyData[495]=0;
    DummyData[496]=0;
    DummyData[497]=0;
    DummyData[498]=0;
    DummyData[499]=0;
    DummyData[500]=0;
    DummyData[501]=0;
    DummyData[502]=0;
    DummyData[503]=0;
    DummyData[504]=0;
    DummyData[505]=0;
    DummyData[506]=0;
    DummyData[507]=0;
    DummyData[508]=0;
    DummyData[509]=0;
    DummyData[510]=0;
    DummyData[511]=0;
    DummyData[512]=0;
    DummyData[513]=0;
    DummyData[514]=0;
    DummyData[515]=0;
    DummyData[516]=0;
    DummyData[517]=0;
    DummyData[518]=0;
    DummyData[519]=0;
    DummyData[520]=0;
    DummyData[521]=0;
    DummyData[522]=0;
    DummyData[523]=0;
    DummyData[524]=0;
    DummyData[525]=0;
    DummyData[526]=0;
    DummyData[527]=0;
    DummyData[528]=0;
    DummyData[529]=0;
    DummyData[530]=0;
    DummyData[531]=0;
    DummyData[532]=0;
    DummyData[533]=0;
    DummyData[534]=0;
    DummyData[535]=0;
    DummyData[536]=0;
    DummyData[537]=0;
    DummyData[538]=0;
    DummyData[539]=0;
    DummyData[540]=192;
    DummyData[541]=47;
    DummyData[542]=103;
    DummyData[543]=41;
    DummyData[544]=113;
    DummyData[545]=108;
    DummyData[546]=7;
    DummyData[547]=46;
    DummyData[548]=192;
    DummyData[549]=75;
    DummyData[550]=31;
    DummyData[551]=66;
    DummyData[552]=73;
    DummyData[553]=152;
    DummyData[554]=53;
    DummyData[555]=123;
    DummyData[556]=192;
    DummyData[557]=31;
    DummyData[558]=98;
    DummyData[559]=209;
    DummyData[560]=244;
    DummyData[561]=212;
    DummyData[562]=34;
    DummyData[563]=92;
    DummyData[564]=191;
    DummyData[565]=193;
    DummyData[566]=8;
    DummyData[567]=8;
    DummyData[568]=69;
    DummyData[569]=206;
    DummyData[570]=49;
    DummyData[571]=10;
    DummyData[572]=63;
    DummyData[573]=151;
    DummyData[574]=167;
    DummyData[575]=252;
    DummyData[576]=171;
    DummyData[577]=48;
    DummyData[578]=150;
    DummyData[579]=229;
    DummyData[580]=63;
    DummyData[581]=217;
    DummyData[582]=55;
    DummyData[583]=110;
    DummyData[584]=98;
    DummyData[585]=151;
    DummyData[586]=61;
    DummyData[587]=163;
    DummyData[588]=63;
    DummyData[589]=193;
    DummyData[590]=172;
    DummyData[591]=131;
    DummyData[592]=109;
    DummyData[593]=48;
    DummyData[594]=165;
    DummyData[595]=109;
    DummyData[596]=191;
    DummyData[597]=110;
    DummyData[598]=21;
    DummyData[599]=222;
    DummyData[600]=151;
    DummyData[601]=168;
    DummyData[602]=89;
    DummyData[603]=192;
    DummyData[604]=63;
    DummyData[605]=216;
    DummyData[606]=255;
    DummyData[607]=32;
    DummyData[608]=49;
    DummyData[609]=227;
    DummyData[610]=76;
    DummyData[611]=209;
    DummyData[612]=63;
    DummyData[613]=247;
    DummyData[614]=98;
    DummyData[615]=229;
    DummyData[616]=55;
    DummyData[617]=32;
    DummyData[618]=54;
    DummyData[619]=91;
    DummyData[620]=63;
    DummyData[621]=230;
    DummyData[622]=166;
    DummyData[623]=162;
    DummyData[624]=137;
    DummyData[625]=111;
    DummyData[626]=130;
    DummyData[627]=145;
    DummyData[628]=191;
    DummyData[629]=228;
    DummyData[630]=12;
    DummyData[631]=97;
    DummyData[632]=100;
    DummyData[633]=157;
    DummyData[634]=49;
    DummyData[635]=162;
    DummyData[636]=0;
    DummyData[637]=0;
    DummyData[638]=0;
    DummyData[639]=0;
    DummyData[640]=0;
    DummyData[641]=0;
    DummyData[642]=0;
    DummyData[643]=0;
    DummyData[644]=0;
    DummyData[645]=0;
    DummyData[646]=0;
    DummyData[647]=0;
    DummyData[648]=0;
    DummyData[649]=0;
    DummyData[650]=0;
    DummyData[651]=0;
    DummyData[652]=128;
    DummyData[653]=0;
    DummyData[654]=0;
    DummyData[655]=0;
    DummyData[656]=0;
    DummyData[657]=0;
    DummyData[658]=0;
    DummyData[659]=0;
    DummyData[660]=0;
    DummyData[661]=0;
    DummyData[662]=0;
    DummyData[663]=0;
    DummyData[664]=0;
    DummyData[665]=0;
    DummyData[666]=0;
    DummyData[667]=0;
    DummyData[668]=0;
    DummyData[669]=0;
    DummyData[670]=0;
    DummyData[671]=0;
    DummyData[672]=0;
    DummyData[673]=0;
    DummyData[674]=0;
    DummyData[675]=0;
    DummyData[676]=0;
    DummyData[677]=0;
    DummyData[678]=0;
    DummyData[679]=0;
    DummyData[680]=0;
    DummyData[681]=0;
    DummyData[682]=0;
    DummyData[683]=0;
    DummyData[684]=0;
    DummyData[685]=0;
    DummyData[686]=0;
    DummyData[687]=0;
    DummyData[688]=0;
    DummyData[689]=0;
    DummyData[690]=0;
    DummyData[691]=0;
    DummyData[692]=64;
    DummyData[693]=63;
    DummyData[694]=0;
    DummyData[695]=0;
    DummyData[696]=0;
    DummyData[697]=0;
    DummyData[698]=0;
    DummyData[699]=0;
    DummyData[700]=64;
    DummyData[701]=64;
    DummyData[702]=40;
    DummyData[703]=0;
    DummyData[704]=0;
    DummyData[705]=0;
    DummyData[706]=0;
    DummyData[707]=0;
    DummyData[708]=64;
    DummyData[709]=64;
    DummyData[710]=184;
    DummyData[711]=0;
    DummyData[712]=0;
    DummyData[713]=0;
    DummyData[714]=0;
    DummyData[715]=0;
    DummyData[716]=64;
    DummyData[717]=67;
    DummyData[718]=88;
    DummyData[719]=0;
    DummyData[720]=0;
    DummyData[721]=0;
    DummyData[722]=0;
    DummyData[723]=0;
    DummyData[724]=64;
    DummyData[725]=68;
    DummyData[726]=88;
    DummyData[727]=0;
    DummyData[728]=0;
    DummyData[729]=0;
    DummyData[730]=0;
    DummyData[731]=0;
    DummyData[732]=64;
    DummyData[733]=68;
    DummyData[734]=216;
    DummyData[735]=0;
    DummyData[736]=0;
    DummyData[737]=0;
    DummyData[738]=0;
    DummyData[739]=0;
    DummyData[740]=63;
    DummyData[741]=227;
    DummyData[742]=228;
    DummyData[743]=213;
    DummyData[744]=216;
    DummyData[745]=14;
    DummyData[746]=73;
    DummyData[747]=111;
    DummyData[748]=64;
    DummyData[749]=0;
    DummyData[750]=0;
    DummyData[751]=0;
    DummyData[752]=0;
    DummyData[753]=0;
    DummyData[754]=0;
    DummyData[755]=0;
    DummyData[756]=64;
    DummyData[757]=28;
    DummyData[758]=0;
    DummyData[759]=0;
    DummyData[760]=0;
    DummyData[761]=0;
    DummyData[762]=0;
    DummyData[763]=0;
    DummyData[764]=64;
    DummyData[765]=111;
    DummyData[766]=160;
    DummyData[767]=0;
    DummyData[768]=0;
    DummyData[769]=0;
    DummyData[770]=0;
    DummyData[771]=0;
    DummyData[772]=64;
    DummyData[773]=111;
    DummyData[774]=160;
    DummyData[775]=0;
    DummyData[776]=0;
    DummyData[777]=0;
    DummyData[778]=0;
    DummyData[779]=0;
    DummyData[780]=64;
    DummyData[781]=111;
    DummyData[782]=160;
    DummyData[783]=0;
    DummyData[784]=0;
    DummyData[785]=0;
    DummyData[786]=0;
    DummyData[787]=0;
    DummyData[788]=64;
    DummyData[789]=111;
    DummyData[790]=160;
    DummyData[791]=0;
    DummyData[792]=0;
    DummyData[793]=0;
    DummyData[794]=0;
    DummyData[795]=0;
    DummyData[796]=64;
    DummyData[797]=111;
    DummyData[798]=160;
    DummyData[799]=0;
    DummyData[800]=0;
    DummyData[801]=0;
    DummyData[802]=0;
    DummyData[803]=0;
    DummyData[804]=64;
    DummyData[805]=111;
    DummyData[806]=160;
    DummyData[807]=0;
    DummyData[808]=0;
    DummyData[809]=0;
    DummyData[810]=0;
    DummyData[811]=0;
    DummyData[812]=63;
    DummyData[813]=240;
    DummyData[814]=0;
    DummyData[815]=0;
    DummyData[816]=0;
    DummyData[817]=0;
    DummyData[818]=0;
    DummyData[819]=0;
    DummyData[820]=64;
    DummyData[821]=13;
    DummyData[822]=60;
    DummyData[823]=62;
    DummyData[824]=91;
    DummyData[825]=193;
    DummyData[826]=199;
    DummyData[827]=164;
    DummyData[828]=64;
    DummyData[829]=19;
    DummyData[830]=57;
    DummyData[831]=165;
    DummyData[832]=130;
    DummyData[833]=223;
    DummyData[834]=245;
    DummyData[835]=75;
    DummyData[836]=64;
    DummyData[837]=19;
    DummyData[838]=250;
    DummyData[839]=183;
    DummyData[840]=48;
    DummyData[841]=159;
    DummyData[842]=75;
    DummyData[843]=129;
    DummyData[844]=64;
    DummyData[845]=10;
    DummyData[846]=119;
    DummyData[847]=150;
    DummyData[848]=91;
    DummyData[849]=207;
    DummyData[850]=216;
    DummyData[851]=208;
    DummyData[852]=64;
    DummyData[853]=10;
    DummyData[854]=167;
    DummyData[855]=71;
    DummyData[856]=194;
    DummyData[857]=50;
    DummyData[858]=145;
    DummyData[859]=227;
    DummyData[860]=64;
    DummyData[861]=9;
    DummyData[862]=118;
    DummyData[863]=214;
    DummyData[864]=248;
    DummyData[865]=80;
    DummyData[866]=152;
    DummyData[867]=213;
    DummyData[868]=192;
    DummyData[869]=29;
    DummyData[870]=29;
    DummyData[871]=13;
    DummyData[872]=224;
    DummyData[873]=0;
    DummyData[874]=0;
    DummyData[875]=0;
    DummyData[876]=64;
    DummyData[877]=24;
    DummyData[878]=249;
    DummyData[879]=239;
    DummyData[880]=160;
    DummyData[881]=0;
    DummyData[882]=0;
    DummyData[883]=0;
    DummyData[884]=191;
    DummyData[885]=210;
    DummyData[886]=99;
    DummyData[887]=49;
    DummyData[888]=32;
    DummyData[889]=0;
    DummyData[890]=0;
    DummyData[891]=0;
    DummyData[892]=0;
    DummyData[893]=0;
    DummyData[894]=0;
    DummyData[895]=0;
    DummyData[896]=0;
    DummyData[897]=0;
    DummyData[898]=0;
    DummyData[899]=0;
    DummyData[900]=0;
    DummyData[901]=0;
    DummyData[902]=0;
    DummyData[903]=0;
    DummyData[904]=0;
    DummyData[905]=0;
    DummyData[906]=0;
    DummyData[907]=0;
    DummyData[908]=0;
    DummyData[909]=0;
    DummyData[910]=0;
    DummyData[911]=0;
    DummyData[912]=0;
    DummyData[913]=0;
    DummyData[914]=0;
    DummyData[915]=0;
    DummyData[916]=0;
    DummyData[917]=0;
    DummyData[918]=0;
    DummyData[919]=0;
    DummyData[920]=0;
    DummyData[921]=0;
    DummyData[922]=0;
    DummyData[923]=0;
    DummyData[924]=0;
    DummyData[925]=0;
    DummyData[926]=0;
    DummyData[927]=0;
    DummyData[928]=0;
    DummyData[929]=0;
    DummyData[930]=0;
    DummyData[931]=0;
    DummyData[932]=0;
    DummyData[933]=0;
    DummyData[934]=0;
    DummyData[935]=0;
    DummyData[936]=0;
    DummyData[937]=0;
    DummyData[938]=0;
    DummyData[939]=0;
    DummyData[940]=0;
    DummyData[941]=0;
    DummyData[942]=0;
    DummyData[943]=0;
    DummyData[944]=0;
    DummyData[945]=0;
    DummyData[946]=0;
    DummyData[947]=0;
    DummyData[948]=0;
    DummyData[949]=0;
    DummyData[950]=0;
    DummyData[951]=0;
    DummyData[952]=0;
    DummyData[953]=0;
    DummyData[954]=0;
    DummyData[955]=0;
    DummyData[956]=64;
    DummyData[957]=38;
    DummyData[958]=0;
    DummyData[959]=0;
    DummyData[960]=0;
    DummyData[961]=0;
    DummyData[962]=0;
    DummyData[963]=0;
    DummyData[964]=0;
    DummyData[965]=0;
    DummyData[966]=0;
    DummyData[967]=0;
    DummyData[968]=0;
    DummyData[969]=0;
    DummyData[970]=0;
    DummyData[971]=0;
    DummyData[972]=64;
    DummyData[973]=71;
    DummyData[974]=230;
    DummyData[975]=64;
    DummyData[976]=32;
    DummyData[977]=0;
    DummyData[978]=0;
    DummyData[979]=0;
    DummyData[980]=64;
    DummyData[981]=71;
    DummyData[982]=242;
    DummyData[983]=191;
    DummyData[984]=0;
    DummyData[985]=0;
    DummyData[986]=0;
    DummyData[987]=0;
    DummyData[988]=63;
    DummyData[989]=233;
    DummyData[990]=227;
    DummyData[991]=154;
    DummyData[992]=160;
    DummyData[993]=0;
    DummyData[994]=0;
    DummyData[995]=0;
    DummyData[996]=64;
    DummyData[997]=71;
    DummyData[998]=232;
    DummyData[999]=57;
    DummyData[1000]=128;
    DummyData[1001]=0;
    DummyData[1002]=0;
    DummyData[1003]=0;
    DummyData[1004]=64;
    DummyData[1005]=71;
    DummyData[1006]=230;
    DummyData[1007]=99;
    DummyData[1008]=224;
    DummyData[1009]=0;
    DummyData[1010]=0;
    DummyData[1011]=0;
    DummyData[1012]=64;
    DummyData[1013]=71;
    DummyData[1014]=237;
    DummyData[1015]=186;
    DummyData[1016]=128;
    DummyData[1017]=0;
    DummyData[1018]=0;
    DummyData[1019]=0;
    DummyData[1020]=64;
    DummyData[1021]=71;
    DummyData[1022]=228;
    DummyData[1023]=142;
    DummyData[1024]=64;
    DummyData[1025]=0;
    DummyData[1026]=0;
    DummyData[1027]=0;
    DummyData[1028]=64;
    DummyData[1029]=71;
    DummyData[1030]=232;
    DummyData[1031]=57;
    DummyData[1032]=128;
    DummyData[1033]=0;
    DummyData[1034]=0;
    DummyData[1035]=0;
    DummyData[1036]=64;
    DummyData[1037]=71;
    DummyData[1038]=223;
    DummyData[1039]=13;
    DummyData[1040]=64;
    DummyData[1041]=0;
    DummyData[1042]=0;
    DummyData[1043]=0;
    DummyData[1044]=0;
    DummyData[1045]=0;
    DummyData[1046]=0;
    DummyData[1047]=0;
    DummyData[1048]=0;
    DummyData[1049]=0;
    DummyData[1050]=0;
    DummyData[1051]=0;
    DummyData[1052]=63;
    DummyData[1053]=240;
    DummyData[1054]=0;
    DummyData[1055]=0;
    DummyData[1056]=0;
    DummyData[1057]=0;
    DummyData[1058]=0;
    DummyData[1059]=0;
    DummyData[1060]=63;
    DummyData[1061]=179;
    DummyData[1062]=36;
    DummyData[1063]=82;
    DummyData[1064]=116;
    DummyData[1065]=30;
    DummyData[1066]=95;
    DummyData[1067]=145;
    DummyData[1068]=63;
    DummyData[1069]=206;
    DummyData[1070]=177;
    DummyData[1071]=249;
    DummyData[1072]=197;
    DummyData[1073]=223;
    DummyData[1074]=190;
    DummyData[1075]=34;
    DummyData[1076]=63;
    DummyData[1077]=185;
    DummyData[1078]=198;
    DummyData[1079]=20;
    DummyData[1080]=125;
    DummyData[1081]=119;
    DummyData[1082]=89;
    DummyData[1083]=140;
    DummyData[1084]=128;
    DummyData[1085]=0;
    DummyData[1086]=0;
    DummyData[1087]=0;
    DummyData[1088]=0;
    DummyData[1089]=0;
    DummyData[1090]=0;
    DummyData[1091]=0;
    DummyData[1092]=0;
    DummyData[1093]=0;
    DummyData[1094]=0;
    DummyData[1095]=0;
    DummyData[1096]=0;
    DummyData[1097]=0;
    DummyData[1098]=0;
    DummyData[1099]=0;
    DummyData[1100]=0;
    DummyData[1101]=0;
    DummyData[1102]=0;
    DummyData[1103]=0;
    DummyData[1104]=0;
    DummyData[1105]=0;
    DummyData[1106]=0;
    DummyData[1107]=0;
    DummyData[1108]=63;
    DummyData[1109]=240;
    DummyData[1110]=0;
    DummyData[1111]=0;
    DummyData[1112]=0;
    DummyData[1113]=0;
    DummyData[1114]=0;
    DummyData[1115]=0;
}
