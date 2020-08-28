#include <QtWidgets>
#include <QtNetwork>

#include <stdlib.h>

#include "URLowSpeedport.hpp"

URLowSpeedport::URLowSpeedport(QWidget *parent)
    : QDialog(parent)
    , statusLabel(new QLabel)
    , tcpServer(Q_NULLPTR)
    , networkSession(nullptr)
    , clientConnection (nullptr)
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
        connect(networkSession, &QNetworkSession::opened, this, &URLowSpeedport::sessionOpened);
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
    connect(tcpServer, &QTcpServer::newConnection, this, &URLowSpeedport::handleNewConnection);
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

void URLowSpeedport::sessionOpened()
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
    if (!tcpServer->listen(address,30002)) {
        QMessageBox::critical(this, tr("UR3e Low speed Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    statusLabel->setText(tr("The UR3e low speed server is running on\n\nIP: %1\nport: %2\n\n")
                         .arg("127.0.0.1").arg(tcpServer->serverPort()));
}

void URLowSpeedport::handleNewConnection()
{
    qDebug() << "URLowSpeedport - handleNewConnection Detected.. ";
    if(clientConnection == nullptr)
    {
        timer->start(100);
        clientConnection = tcpServer->nextPendingConnection();
        connect(clientConnection, &QAbstractSocket::disconnected,
                this, &URLowSpeedport::disconnectFromServer);
    }else
    {
        qDebug() << "URLowSpeedport - handleNewConnection - Already a client connection is in active state.. ";
    }
}
void URLowSpeedport::disconnectFromServer()
{
    timer->stop();
    clientConnection->disconnectFromHost();
    clientConnection = nullptr;
}
void URLowSpeedport::sendRealTimeData()
{
    const char *data = reinterpret_cast<char*>(DummyData);
    clientConnection->write(data,716);
}

void URLowSpeedport::handleProgrammingStatusChange(bool p_status)
{
    qDebug() << "URLowSpeedport::handleProgrammingStatusChange received =" << p_status;
    if(p_status == false)
    {
        DummyData[23]=0;
    }else
    {
        DummyData[23]=1;
    }
}
void URLowSpeedport::getPackedValue(double p_value,IEEE754Packed64BitData& p_packedvalue)
{
    IEEE754PackedDoubleData tcpx;
    tcpx.packedDoubleValue = p_value;
    IEEE754PackedDoubleData tcpinvertedx;
    for(int i=7,j=0;j<8;i--,j++)
    {
        tcpinvertedx.byteAccess[i] = tcpx.byteAccess[j];
    }
    p_packedvalue.packed64BitValue = encodeDouble(tcpinvertedx.packedDoubleValue,64,11);  
}
void URLowSpeedport::setTCP(UR3eTCP p_newTCP)
{
     qDebug()<< "New TCP set Request received...=";
    IEEE754Packed64BitData tcp64bitX;
    getPackedValue(p_newTCP.ur3X_m,tcp64bitX);
    memcpy(&DummyData[308],tcp64bitX.byteAccess,sizeof(uint64_t));

    IEEE754Packed64BitData tcp64bitY;
    getPackedValue(p_newTCP.ur3Y_m,tcp64bitY);
    memcpy(&DummyData[316],tcp64bitY.byteAccess,sizeof(uint64_t));

    IEEE754Packed64BitData tcp64bitZ;
    getPackedValue(p_newTCP.ur3Z_m,tcp64bitZ);
    memcpy(&DummyData[324],tcp64bitZ.byteAccess,sizeof(uint64_t));

    IEEE754Packed64BitData tcp64bitRX;
    getPackedValue(p_newTCP.ur3RX_rad,tcp64bitRX);
    memcpy(&DummyData[332],tcp64bitRX.byteAccess,sizeof(uint64_t));

    IEEE754Packed64BitData tcp64bitRY;
    getPackedValue(p_newTCP.ur3RY_rad,tcp64bitRY);
    memcpy(&DummyData[340],tcp64bitRY.byteAccess,sizeof(uint64_t));

    IEEE754Packed64BitData tcp64bitRZ;
    getPackedValue(p_newTCP.ur3RZ_rad,tcp64bitRZ);
    memcpy(&DummyData[348],tcp64bitRZ.byteAccess,sizeof(uint64_t));
}
uint64_t URLowSpeedport::encodeDouble(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}
void URLowSpeedport::initDummyData()
{
    DummyData[0]=0;
    DummyData[1]=0;
    DummyData[2]=2;
    DummyData[3]=204;
    DummyData[4]=16;
    DummyData[5]=0;
    DummyData[6]=0;
    DummyData[7]=0;
    DummyData[8]=47;
    DummyData[9]=0;
    DummyData[10]=0;
    DummyData[11]=0;
    DummyData[12]=0;
    DummyData[13]=22;
    DummyData[14]=237;
    DummyData[15]=153;
    DummyData[16]=67;
    DummyData[17]=136;
    DummyData[18]=1;
    DummyData[19]=1;
    DummyData[20]=1;
    DummyData[21]=0;
    DummyData[22]=0;
    DummyData[23]=0;
    DummyData[24]=0;
    DummyData[25]=7;
    DummyData[26]=0;
    DummyData[27]=63;
    DummyData[28]=240;
    DummyData[29]=0;
    DummyData[30]=0;
    DummyData[31]=0;
    DummyData[32]=0;
    DummyData[33]=0;
    DummyData[34]=0;
    DummyData[35]=0;
    DummyData[36]=0;
    DummyData[37]=0;
    DummyData[38]=0;
    DummyData[39]=0;
    DummyData[40]=0;
    DummyData[41]=0;
    DummyData[42]=0;
    DummyData[43]=63;
    DummyData[44]=240;
    DummyData[45]=0;
    DummyData[46]=0;
    DummyData[47]=0;
    DummyData[48]=0;
    DummyData[49]=0;
    DummyData[50]=0;
    DummyData[51]=0;
    DummyData[52]=0;
    DummyData[53]=0;
    DummyData[54]=0;
    DummyData[55]=251;
    DummyData[56]=1;
    DummyData[57]=63;
    DummyData[58]=249;
    DummyData[59]=123;
    DummyData[60]=172;
    DummyData[61]=0;
    DummyData[62]=0;
    DummyData[63]=0;
    DummyData[64]=0;
    DummyData[65]=63;
    DummyData[66]=249;
    DummyData[67]=123;
    DummyData[68]=159;
    DummyData[69]=128;
    DummyData[70]=2;
    DummyData[71]=52;
    DummyData[72]=65;
    DummyData[73]=0;
    DummyData[74]=0;
    DummyData[75]=0;
    DummyData[76]=0;
    DummyData[77]=0;
    DummyData[78]=0;
    DummyData[79]=0;
    DummyData[80]=0;
    DummyData[81]=60;
    DummyData[82]=123;
    DummyData[83]=47;
    DummyData[84]=226;
    DummyData[85]=66;
    DummyData[86]=63;
    DummyData[87]=80;
    DummyData[88]=122;
    DummyData[89]=65;
    DummyData[90]=247;
    DummyData[91]=128;
    DummyData[92]=0;
    DummyData[93]=0;
    DummyData[94]=0;
    DummyData[95]=0;
    DummyData[96]=0;
    DummyData[97]=253;
    DummyData[98]=192;
    DummyData[99]=10;
    DummyData[100]=211;
    DummyData[101]=124;
    DummyData[102]=18;
    DummyData[103]=177;
    DummyData[104]=124;
    DummyData[105]=216;
    DummyData[106]=192;
    DummyData[107]=10;
    DummyData[108]=211;
    DummyData[109]=130;
    DummyData[110]=82;
    DummyData[111]=167;
    DummyData[112]=11;
    DummyData[113]=24;
    DummyData[114]=0;
    DummyData[115]=0;
    DummyData[116]=0;
    DummyData[117]=0;
    DummyData[118]=0;
    DummyData[119]=0;
    DummyData[120]=0;
    DummyData[121]=0;
    DummyData[122]=63;
    DummyData[123]=130;
    DummyData[124]=155;
    DummyData[125]=103;
    DummyData[126]=66;
    DummyData[127]=63;
    DummyData[128]=51;
    DummyData[129]=31;
    DummyData[130]=66;
    DummyData[131]=1;
    DummyData[132]=64;
    DummyData[133]=0;
    DummyData[134]=0;
    DummyData[135]=0;
    DummyData[136]=0;
    DummyData[137]=0;
    DummyData[138]=253;
    DummyData[139]=63;
    DummyData[140]=255;
    DummyData[141]=158;
    DummyData[142]=212;
    DummyData[143]=209;
    DummyData[144]=16;
    DummyData[145]=180;
    DummyData[146]=96;
    DummyData[147]=63;
    DummyData[148]=255;
    DummyData[149]=158;
    DummyData[150]=250;
    DummyData[151]=80;
    DummyData[152]=248;
    DummyData[153]=119;
    DummyData[154]=152;
    DummyData[155]=128;
    DummyData[156]=0;
    DummyData[157]=0;
    DummyData[158]=0;
    DummyData[159]=0;
    DummyData[160]=0;
    DummyData[161]=0;
    DummyData[162]=0;
    DummyData[163]=189;
    DummyData[164]=223;
    DummyData[165]=40;
    DummyData[166]=82;
    DummyData[167]=66;
    DummyData[168]=63;
    DummyData[169]=153;
    DummyData[170]=220;
    DummyData[171]=66;
    DummyData[172]=6;
    DummyData[173]=0;
    DummyData[174]=0;
    DummyData[175]=0;
    DummyData[176]=0;
    DummyData[177]=0;
    DummyData[178]=0;
    DummyData[179]=253;
    DummyData[180]=191;
    DummyData[181]=251;
    DummyData[182]=247;
    DummyData[183]=156;
    DummyData[184]=165;
    DummyData[185]=98;
    DummyData[186]=249;
    DummyData[187]=176;
    DummyData[188]=191;
    DummyData[189]=251;
    DummyData[190]=247;
    DummyData[191]=181;
    DummyData[192]=165;
    DummyData[193]=72;
    DummyData[194]=11;
    DummyData[195]=160;
    DummyData[196]=128;
    DummyData[197]=0;
    DummyData[198]=0;
    DummyData[199]=0;
    DummyData[200]=0;
    DummyData[201]=0;
    DummyData[202]=0;
    DummyData[203]=0;
    DummyData[204]=190;
    DummyData[205]=46;
    DummyData[206]=222;
    DummyData[207]=131;
    DummyData[208]=66;
    DummyData[209]=63;
    DummyData[210]=95;
    DummyData[211]=39;
    DummyData[212]=66;
    DummyData[213]=26;
    DummyData[214]=192;
    DummyData[215]=0;
    DummyData[216]=0;
    DummyData[217]=0;
    DummyData[218]=0;
    DummyData[219]=0;
    DummyData[220]=253;
    DummyData[221]=191;
    DummyData[222]=248;
    DummyData[223]=169;
    DummyData[224]=178;
    DummyData[225]=209;
    DummyData[226]=16;
    DummyData[227]=180;
    DummyData[228]=96;
    DummyData[229]=191;
    DummyData[230]=248;
    DummyData[231]=169;
    DummyData[232]=166;
    DummyData[233]=81;
    DummyData[234]=30;
    DummyData[235]=153;
    DummyData[236]=112;
    DummyData[237]=0;
    DummyData[238]=0;
    DummyData[239]=0;
    DummyData[240]=0;
    DummyData[241]=0;
    DummyData[242]=0;
    DummyData[243]=0;
    DummyData[244]=0;
    DummyData[245]=61;
    DummyData[246]=19;
    DummyData[247]=115;
    DummyData[248]=130;
    DummyData[249]=66;
    DummyData[250]=63;
    DummyData[251]=95;
    DummyData[252]=39;
    DummyData[253]=66;
    DummyData[254]=34;
    DummyData[255]=128;
    DummyData[256]=0;
    DummyData[257]=0;
    DummyData[258]=0;
    DummyData[259]=0;
    DummyData[260]=0;
    DummyData[261]=253;
    DummyData[262]=192;
    DummyData[263]=15;
    DummyData[264]=236;
    DummyData[265]=143;
    DummyData[266]=232;
    DummyData[267]=136;
    DummyData[268]=90;
    DummyData[269]=48;
    DummyData[270]=192;
    DummyData[271]=15;
    DummyData[272]=236;
    DummyData[273]=143;
    DummyData[274]=232;
    DummyData[275]=131;
    DummyData[276]=97;
    DummyData[277]=176;
    DummyData[278]=0;
    DummyData[279]=0;
    DummyData[280]=0;
    DummyData[281]=0;
    DummyData[282]=0;
    DummyData[283]=0;
    DummyData[284]=0;
    DummyData[285]=0;
    DummyData[286]=188;
    DummyData[287]=232;
    DummyData[288]=212;
    DummyData[289]=53;
    DummyData[290]=66;
    DummyData[291]=63;
    DummyData[292]=21;
    DummyData[293]=197;
    DummyData[294]=66;
    DummyData[295]=38;
    DummyData[296]=192;
    DummyData[297]=0;
    DummyData[298]=0;
    DummyData[299]=0;
    DummyData[300]=0;
    DummyData[301]=0;
    DummyData[302]=253;
    DummyData[303]=0;
    DummyData[304]=0;
    DummyData[305]=0;
    DummyData[306]=101;
    DummyData[307]=4;
    DummyData[308]=63;
    DummyData[309]=193;
    DummyData[310]=172;
    DummyData[311]=110;
    DummyData[312]=236;
    DummyData[313]=191;
    DummyData[314]=83;
    DummyData[315]=50;
    DummyData[316]=191;
    DummyData[317]=110;
    DummyData[318]=6;
    DummyData[319]=230;
    DummyData[320]=52;
    DummyData[321]=66;
    DummyData[322]=224;
    DummyData[323]=224;
    DummyData[324]=63;
    DummyData[325]=216;
    DummyData[326]=255;
    DummyData[327]=48;
    DummyData[328]=70;
    DummyData[329]=87;
    DummyData[330]=199;
    DummyData[331]=67;
    DummyData[332]=63;
    DummyData[333]=247;
    DummyData[334]=98;
    DummyData[335]=229;
    DummyData[336]=16;
    DummyData[337]=185;
    DummyData[338]=254;
    DummyData[339]=212;
    DummyData[340]=63;
    DummyData[341]=230;
    DummyData[342]=166;
    DummyData[343]=162;
    DummyData[344]=225;
    DummyData[345]=146;
    DummyData[346]=29;
    DummyData[347]=205;
    DummyData[348]=191;
    DummyData[349]=228;
    DummyData[350]=12;
    DummyData[351]=97;
    DummyData[352]=157;
    DummyData[353]=108;
    DummyData[354]=180;
    DummyData[355]=94;
    DummyData[356]=0;
    DummyData[357]=0;
    DummyData[358]=0;
    DummyData[359]=0;
    DummyData[360]=0;
    DummyData[361]=0;
    DummyData[362]=0;
    DummyData[363]=0;
    DummyData[364]=0;
    DummyData[365]=0;
    DummyData[366]=0;
    DummyData[367]=0;
    DummyData[368]=0;
    DummyData[369]=0;
    DummyData[370]=0;
    DummyData[371]=0;
    DummyData[372]=63;
    DummyData[373]=188;
    DummyData[374]=40;
    DummyData[375]=245;
    DummyData[376]=194;
    DummyData[377]=143;
    DummyData[378]=92;
    DummyData[379]=41;
    DummyData[380]=0;
    DummyData[381]=0;
    DummyData[382]=0;
    DummyData[383]=0;
    DummyData[384]=0;
    DummyData[385]=0;
    DummyData[386]=0;
    DummyData[387]=0;
    DummyData[388]=0;
    DummyData[389]=0;
    DummyData[390]=0;
    DummyData[391]=0;
    DummyData[392]=0;
    DummyData[393]=0;
    DummyData[394]=0;
    DummyData[395]=0;
    DummyData[396]=0;
    DummyData[397]=0;
    DummyData[398]=0;
    DummyData[399]=0;
    DummyData[400]=0;
    DummyData[401]=0;
    DummyData[402]=0;
    DummyData[403]=0;
    DummyData[404]=0;
    DummyData[405]=0;
    DummyData[406]=0;
    DummyData[407]=53;
    DummyData[408]=9;
    DummyData[409]=191;
    DummyData[410]=196;
    DummyData[411]=112;
    DummyData[412]=153;
    DummyData[413]=61;
    DummyData[414]=121;
    DummyData[415]=253;
    DummyData[416]=243;
    DummyData[417]=191;
    DummyData[418]=181;
    DummyData[419]=28;
    DummyData[420]=148;
    DummyData[421]=115;
    DummyData[422]=209;
    DummyData[423]=136;
    DummyData[424]=213;
    DummyData[425]=191;
    DummyData[426]=237;
    DummyData[427]=91;
    DummyData[428]=208;
    DummyData[429]=145;
    DummyData[430]=202;
    DummyData[431]=186;
    DummyData[432]=52;
    DummyData[433]=63;
    DummyData[434]=197;
    DummyData[435]=228;
    DummyData[436]=221;
    DummyData[437]=248;
    DummyData[438]=253;
    DummyData[439]=138;
    DummyData[440]=174;
    DummyData[441]=191;
    DummyData[442]=192;
    DummyData[443]=145;
    DummyData[444]=231;
    DummyData[445]=198;
    DummyData[446]=250;
    DummyData[447]=98;
    DummyData[448]=251;
    DummyData[449]=63;
    DummyData[450]=186;
    DummyData[451]=65;
    DummyData[452]=14;
    DummyData[453]=28;
    DummyData[454]=179;
    DummyData[455]=75;
    DummyData[456]=206;
    DummyData[457]=0;
    DummyData[458]=0;
    DummyData[459]=0;
    DummyData[460]=75;
    DummyData[461]=3;
    DummyData[462]=0;
    DummyData[463]=0;
    DummyData[464]=0;
    DummyData[465]=0;
    DummyData[466]=0;
    DummyData[467]=0;
    DummyData[468]=0;
    DummyData[469]=0;
    DummyData[470]=1;
    DummyData[471]=1;
    DummyData[472]=128;
    DummyData[473]=0;
    DummyData[474]=0;
    DummyData[475]=0;
    DummyData[476]=0;
    DummyData[477]=0;
    DummyData[478]=0;
    DummyData[479]=0;
    DummyData[480]=128;
    DummyData[481]=0;
    DummyData[482]=0;
    DummyData[483]=0;
    DummyData[484]=0;
    DummyData[485]=0;
    DummyData[486]=0;
    DummyData[487]=0;
    DummyData[488]=1;
    DummyData[489]=1;
    DummyData[490]=0;
    DummyData[491]=0;
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
    DummyData[506]=65;
    DummyData[507]=253;
    DummyData[508]=128;
    DummyData[509]=0;
    DummyData[510]=66;
    DummyData[511]=63;
    DummyData[512]=177;
    DummyData[513]=123;
    DummyData[514]=63;
    DummyData[515]=64;
    DummyData[516]=55;
    DummyData[517]=74;
    DummyData[518]=0;
    DummyData[519]=0;
    DummyData[520]=0;
    DummyData[521]=0;
    DummyData[522]=1;
    DummyData[523]=0;
    DummyData[524]=0;
    DummyData[525]=166;
    DummyData[526]=82;
    DummyData[527]=120;
    DummyData[528]=3;
    DummyData[529]=0;
    DummyData[530]=1;
    DummyData[531]=1;
    DummyData[532]=0;
    DummyData[533]=0;
    DummyData[534]=0;
    DummyData[535]=37;
    DummyData[536]=2;
    DummyData[537]=1;
    DummyData[538]=1;
    DummyData[539]=63;
    DummyData[540]=159;
    DummyData[541]=147;
    DummyData[542]=109;
    DummyData[543]=160;
    DummyData[544]=0;
    DummyData[545]=0;
    DummyData[546]=0;
    DummyData[547]=63;
    DummyData[548]=152;
    DummyData[549]=143;
    DummyData[550]=28;
    DummyData[551]=128;
    DummyData[552]=0;
    DummyData[553]=0;
    DummyData[554]=0;
    DummyData[555]=0;
    DummyData[556]=0;
    DummyData[557]=0;
    DummyData[558]=0;
    DummyData[559]=0;
    DummyData[560]=60;
    DummyData[561]=158;
    DummyData[562]=85;
    DummyData[563]=96;
    DummyData[564]=66;
    DummyData[565]=19;
    DummyData[566]=192;
    DummyData[567]=0;
    DummyData[568]=253;
    DummyData[569]=0;
    DummyData[570]=0;
    DummyData[571]=0;
    DummyData[572]=61;
    DummyData[573]=7;
    DummyData[574]=0;
    DummyData[575]=0;
    DummyData[576]=0;
    DummyData[577]=0;
    DummyData[578]=0;
    DummyData[579]=0;
    DummyData[580]=0;
    DummyData[581]=0;
    DummyData[582]=0;
    DummyData[583]=0;
    DummyData[584]=0;
    DummyData[585]=0;
    DummyData[586]=0;
    DummyData[587]=0;
    DummyData[588]=0;
    DummyData[589]=0;
    DummyData[590]=0;
    DummyData[591]=0;
    DummyData[592]=0;
    DummyData[593]=0;
    DummyData[594]=0;
    DummyData[595]=0;
    DummyData[596]=0;
    DummyData[597]=0;
    DummyData[598]=0;
    DummyData[599]=0;
    DummyData[600]=0;
    DummyData[601]=0;
    DummyData[602]=0;
    DummyData[603]=0;
    DummyData[604]=0;
    DummyData[605]=0;
    DummyData[606]=0;
    DummyData[607]=0;
    DummyData[608]=0;
    DummyData[609]=0;
    DummyData[610]=0;
    DummyData[611]=0;
    DummyData[612]=0;
    DummyData[613]=0;
    DummyData[614]=0;
    DummyData[615]=0;
    DummyData[616]=0;
    DummyData[617]=0;
    DummyData[618]=0;
    DummyData[619]=0;
    DummyData[620]=0;
    DummyData[621]=0;
    DummyData[622]=63;
    DummyData[623]=22;
    DummyData[624]=210;
    DummyData[625]=62;
    DummyData[626]=40;
    DummyData[627]=241;
    DummyData[628]=183;
    DummyData[629]=0;
    DummyData[630]=0;
    DummyData[631]=0;
    DummyData[632]=0;
    DummyData[633]=9;
    DummyData[634]=8;
    DummyData[635]=0;
    DummyData[636]=0;
    DummyData[637]=0;
    DummyData[638]=1;
    DummyData[639]=0;
    DummyData[640]=0;
    DummyData[641]=0;
    DummyData[642]=43;
    DummyData[643]=10;
    DummyData[644]=166;
    DummyData[645]=82;
    DummyData[646]=120;
    DummyData[647]=3;
    DummyData[648]=0;
    DummyData[649]=1;
    DummyData[650]=63;
    DummyData[651]=179;
    DummyData[652]=35;
    DummyData[653]=242;
    DummyData[654]=78;
    DummyData[655]=125;
    DummyData[656]=117;
    DummyData[657]=168;
    DummyData[658]=63;
    DummyData[659]=206;
    DummyData[660]=178;
    DummyData[661]=13;
    DummyData[662]=215;
    DummyData[663]=189;
    DummyData[664]=38;
    DummyData[665]=215;
    DummyData[666]=63;
    DummyData[667]=185;
    DummyData[668]=198;
    DummyData[669]=68;
    DummyData[670]=28;
    DummyData[671]=230;
    DummyData[672]=88;
    DummyData[673]=61;
    DummyData[674]=63;
    DummyData[675]=185;
    DummyData[676]=153;
    DummyData[677]=153;
    DummyData[678]=153;
    DummyData[679]=153;
    DummyData[680]=153;
    DummyData[681]=154;
    DummyData[682]=0;
    DummyData[683]=0;
    DummyData[684]=0;
    DummyData[685]=26;
    DummyData[686]=11;
    DummyData[687]=0;
    DummyData[688]=0;
    DummyData[689]=1;
    DummyData[690]=194;
    DummyData[691]=0;
    DummyData[692]=0;
    DummyData[693]=0;
    DummyData[694]=0;
    DummyData[695]=0;
    DummyData[696]=0;
    DummyData[697]=0;
    DummyData[698]=0;
    DummyData[699]=1;
    DummyData[700]=63;
    DummyData[701]=192;
    DummyData[702]=0;
    DummyData[703]=0;
    DummyData[704]=64;
    DummyData[705]=96;
    DummyData[706]=0;
    DummyData[707]=0;
    DummyData[708]=0;
    DummyData[709]=0;
    DummyData[710]=0;
    DummyData[711]=8;
    DummyData[712]=12;
    DummyData[713]=0;
    DummyData[714]=1;
    DummyData[715]=1;
}
