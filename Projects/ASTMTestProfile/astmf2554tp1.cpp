#include "astmf2554tp1.h"

ASTMF2554TP1::ASTMF2554TP1()
{
    m_profileName = "Single Point Digitization";
    m_profileId = static_cast<int>(ASTMF2554TestProfileCode::SinglePointAccuracyProfile);
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

ASTMF2554TP1::~ASTMF2554TP1()
{

}
void ASTMF2554TP1::initializeTestProfileView()
{
    m_currentAcquiredPoint = 19;
    m_currentTestMeasurecount = 0;
    m_instructionString = "Please place the pointer at <b>Point 20</b>  and Click on <b>Acquire Point</b> button";
    m_statusString = "Measures Acquired - ";
    m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_totalTestMeasureSize) + "</b>";
    m_statusString.append(m_sample);
}

void ASTMF2554TP1::setTestProfileParams()
{

}
void ASTMF2554TP1::onUndoLastAcquisitionData()
{

}
void ASTMF2554TP1::updateTestProfileView()
{
    m_currentTestMeasurecount++;
    if (m_currentTestMeasurecount == m_totalTestMeasureSize)
    {
        m_instructionString = "All <b>" + QString::number(m_currentTestMeasurecount) + "</b> measures acquisition completed!";
        m_statusString = "Measures Acquired - ";
        m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_totalTestMeasureSize) + "</b>";
        m_statusString.append(m_sample);
        emit sigTestCompletion(m_currentTestMeasurecount, " measures");
        m_currentTestMeasurecount = 0;
        m_currentAcquiredPoint = 19;
    }
    else
    {
        m_instructionString = "Please place the pointer at <b>Point 20</b>  and Click on <b>Acquire Point</b> button";
        m_statusString = "Measures Acquired - ";
        m_sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_totalTestMeasureSize) + "</b>";
        m_statusString.append(m_sample);
    }
}
