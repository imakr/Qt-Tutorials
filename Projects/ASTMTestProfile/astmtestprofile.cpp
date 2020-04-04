#include "astmtestprofile.h"

ASTMTestProfile::ASTMTestProfile()
{  
}

ASTMTestProfile::~ASTMTestProfile()
{
    qDebug()<<"=== New ASTM Test Profile Destructed===";
    qDebug()<<"ASTM Product = "<<m_astmProduct;
    qDebug()<<"ASTM Profile Name = "<<m_profileName;
    qDebug()<<"ASTM Prodile ID = "<<m_profileId;
    qDebug()<<"=======================================";
}

bool ASTMTestProfile::getASTMProductInfo(QString &p_astmProduct) const
{
    p_astmProduct = m_astmProduct;
    return true;
}

bool ASTMTestProfile::getASTMProfileInfo(QString &p_profileName, int &p_profileId) const
{
    p_profileName = m_profileName;
    p_profileId = m_profileId;
    return  true;
}

QString ASTMTestProfile::getASTMTestProfileName() const
{
    return m_profileName;
}

void ASTMTestProfile::getInstructionStatusString(QString &p_instructionString, QString &p_statusString)
{
    p_instructionString = m_instructionString;
    p_statusString = m_statusString;
}

void ASTMTestProfile::setTotalMeasuresCount(uint16_t p_totalTestMeasureSize)
{
    m_totalTestMeasureSize = p_totalTestMeasureSize;
}
