#ifndef ASTMTESTPROFILE_H
#define ASTMTESTPROFILE_H
#pragma once
#include <QString>
#include <QDebug>

class ASTMTestProfile
{
    using FunctionType = std::function<int()>;
    using FunctionType1 = std::function<std::error_code()>;
public:
    ASTMTestProfile(QString p_astmProduct,QString p_profileName,int p_profileId);
    ~ASTMTestProfile();
    bool getASTMProductInfo(QString& p_astmProduct) const;
    bool getASTMProfileInfo(QString& p_profileName,int& p_profileId) const;
    QString getASTMTestProfileName() const;
    /*virtual void onUndoLastAcquisitionData();
    virtual bool validatePointAcqPrecursors(QString& p_errortext);
    virtual QString getTestProfileString();
    virtual void writeASTMTestReportHeader();
    virtual QString getAcqResultString();
    virtual void updateTestProfileView();
    virtual void loadTestProfileView();
    virtual void buildTestProfileLogFile();
    virtual void loadTestProfile();*/
    void setFunction(const FunctionType& p_function);
    void setLoadFunction(const FunctionType1& p_function);
    int doComputation();
    std::error_code doComputation1();
private :
   uint16_t m_currentTestMeasurecount;
   uint16_t m_totalTestMeasureSize;
   uint16_t	m_currentAcquiredPoint;
protected:
    QString m_profileName;
    QString m_astmProduct;
    int m_profileId;
    FunctionType                 m_function;
     FunctionType1                 m_function1;
};

#endif // ASTMTESTPROFILE_H
