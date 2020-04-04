#include "astmf2554tp2.h"

ASTMF2554TP2::ASTMF2554TP2()
{
    m_profileName = "Rotation Analysis";
    m_profileId = static_cast<int>(ASTMF2554TestProfileCode::RotationAroundCenterPointProfile);
    m_astmProduct = "ASTMF2554";
    m_currentTestMeasurecount = 0;
    m_totalTestMeasureSize = 0;
    m_currentAcquiredPoint = 0;
    qDebug()<<"=== New ASTM Test Profile Contructed===";
    qDebug()<<"ASTM Product = "<<m_astmProduct;
    qDebug()<<"ASTM Profile Name = "<<m_profileName;
    qDebug()<<"ASTM Prodile ID = "<<m_profileId;
    qDebug()<<"=======================================";
}

ASTMF2554TP2::~ASTMF2554TP2()
{

}
void ASTMF2554TP2::initializeTestProfileView()
{
    m_currentTestMeasurecount = 0;
    m_currentAcquiredPoint = 19;
    //For rotation -> 0, +15, +30, +45, +60, +75, +90 -> 7 measurement size
    m_totalTestMeasureSize = 7;
    m_instructionString = "Please place the pointer at <b>Point 20</b> by ";
    m_instructionString += QString::number(m_currentTestMeasurecount * 15) + " degrees and Click on <b>Acquire Point</b> button";
    m_statusString = "Measures Acquired - ";
    m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_totalTestMeasureSize) + "</b>";
}

void ASTMF2554TP2::setTestProfileParams()
{

}
void ASTMF2554TP2::onUndoLastAcquisitionData()
{

}
void ASTMF2554TP2::updateTestProfileView()
{
    m_currentTestMeasurecount++;
    if (m_currentTestMeasurecount == m_totalTestMeasureSize)
    {
        m_instructionString = "All <b>" + QString::number(m_currentTestMeasurecount) + "</b> measures acquisition completed!";
        m_statusString = "Measures Acquired - ";
        m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_currentTestMeasurecount) + "</b>";
        m_statusString.append(m_sample);
        emit sigTestCompletion(m_currentTestMeasurecount, " measures");
        m_currentTestMeasurecount = 0;
        m_currentAcquiredPoint = 19;
    }
    else
    {
        m_instructionString = "Rotate the pointer about its axis at <b>Point 20</b> by ";
        m_instructionString += static_cast<bool>(1) ? "+" : "-";
        m_instructionString += QString::number(m_currentTestMeasurecount * 15) + " degrees and Click on <b>Acquire Point</b> button";
        m_statusString = "Measures Acquired - ";
        m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_totalTestMeasureSize) + "</b>";
        m_statusString.append(m_sample);
    }
}
