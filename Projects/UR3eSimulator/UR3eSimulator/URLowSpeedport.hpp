/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QTimer>
#include <QTcpSocket>
QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QTcpServer;
class QNetworkSession;
QT_END_NAMESPACE

//! [0]
class URLowSpeedport : public QDialog
{
    Q_OBJECT

public:
    explicit URLowSpeedport(QWidget *parent = Q_NULLPTR);
    struct UR3eTCP
    {
        double ur3X_m;      // Linear displacement by UR Robot in X axis- current TCP
        double ur3Y_m;      // Linear displacement by UR Robot in Y axis- current TCP
        double ur3Z_m;      // Linear displacement by UR Robot in Z axis- current TCP
        double ur3RX_rad;   // Angular displacement by UR Robot in X axis- current TCP
        double ur3RY_rad;   // Angular displacement by UR Robot in Y axis- current TCP
        double ur3RZ_rad;   // Angular displacement by UR Robot in Z axis- current TCP
    };

    typedef union
    {
        unsigned char byteAccess[sizeof(uint64_t)]; // Byte Access
        double packedDoubleValue;                 // Packed data value
    }IEEE754PackedDoubleData;

    typedef union
    {
        unsigned char byteAccess[sizeof(uint64_t)]; // Byte Access
        uint64_t packed64BitValue;                 // Packed data value
    }IEEE754Packed64BitData;

    void initDummyData();
    void setTCP(UR3eTCP p_newTCP);
    void getPackedValue(double p_value, IEEE754Packed64BitData &p_packedvalue);
public slots:
    void sessionOpened();
    void handleNewConnection();
    void sendRealTimeData();
    void disconnectFromServer();
    void handleProgrammingStatusChange(bool p_status);
private:
    QLabel *statusLabel;
    QTcpServer *tcpServer;
    QStringList fortunes;
    QNetworkSession *networkSession;
    quint16 m_port;
    QTimer *timer;
    QTcpSocket *clientConnection;
    uint8_t DummyData[716];
    uint64_t encodeDouble(long double f, unsigned bits, unsigned expbits);
};

//! [0]

#endif
