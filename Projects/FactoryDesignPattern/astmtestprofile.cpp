#include "astmtestprofile.h"

ASTMTestProfile::ASTMTestProfile(QString p_astmProduct,QString p_profileName,int p_profileID):
    m_profileName(p_profileName),
    m_astmProduct(p_astmProduct),
    m_profileId(p_profileID)
{
    qDebug()<<"=== New ASTM Test Profile Contructed===";
    qDebug()<<"ASTM Product = "<<m_astmProduct;
    qDebug()<<"ASTM Profile Name = "<<m_profileName;
    qDebug()<<"ASTM Prodile ID = "<<m_profileId;
    qDebug()<<"=======================================";
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

void ASTMTestProfile::setFunction(const ASTMTestProfile::FunctionType &p_function)
{
    m_function  =  p_function;
}

void ASTMTestProfile::setLoadFunction(const ASTMTestProfile::FunctionType1 &p_function)
{
     m_function1  =  p_function;
}
int ASTMTestProfile::doComputation()
{
    int err;
    if (m_function)
    {
        qDebug() << "starting computation";
        err = m_function();
        qDebug() << "computation finished";
    }
    else
    {
        qDebug() << "no function specified";
        err = -1;
    }
    return err;
}

std::error_code ASTMTestProfile::doComputation1()
{

    std::error_code err;
    if (m_function1)
    {
        qDebug() << "starting computation";
        err = m_function1();
        qDebug() << "computation finished";
    }
    else
    {
        qDebug() << "no function specified";
        //err = cappapp::AppErrorCode::SystemSetupFail;
    }
    return err;
}
