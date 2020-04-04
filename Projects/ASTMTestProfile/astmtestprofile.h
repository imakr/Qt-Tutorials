#ifndef ASTMTESTPROFILE_H
#define ASTMTESTPROFILE_H
#pragma once
#include <QString>
#include <QDebug>

class ASTMTestProfile : public QObject
{
    using FunctionType = std::function<int()>;
    using FunctionType1 = std::function<std::error_code()>;
    Q_OBJECT
public:
    enum class ASTMF2554TestProfileCode
    {
        TestProfileMenu = 0,					//!< 0.ProfileMenuPage
        SinglePointAccuracyProfile = 1,			//!< 1.Digitalization of single point for Accuracy
        RotationAroundCenterPointProfile = 2,   //!< 2.Rotation around the center point
        ArrayAngledParallelProfile = 3,			//!< 3.Navigation Array angled in parallel
        ArrayAngledPerpendicularProfile = 4,	//!< 4.Navigation Array angled perpendicular
        DigitalizeAllPointsProfile = 5			//!< 5.Digitalization of all individual points
    };
    enum class ASTMF3107TestProfileCode
    {
        TestProfileMenu = 0,					//!< 0.ProfileMenuPage
        SinglePointAccuracyProfile = 1,			//!< 1.Digitalization of single point for Accuracy
        RefBaseAttachAndReattachProfile = 2,	    //!< 2.Reference Base Test - Detach and Reattach
        RefBaseExtForceAppliedProfile = 3,	    //!< 3.Reference Base Test - External Force Application
        SagittalSawCutProfile = 4				//!< 4.Applied Clinical Task - Sagittal Saw Cut
    };
    ASTMTestProfile();
    virtual ~ASTMTestProfile();
    bool getASTMProductInfo(QString& p_astmProduct) const;
    bool getASTMProfileInfo(QString& p_profileName,int& p_profileId) const;
    QString getASTMTestProfileName() const;
    void getInstructionStatusString(QString& p_instructionString,QString& p_statusString);
    void setTotalMeasuresCount(uint16_t p_totalTestMeasureSize);
    virtual void onUndoLastAcquisitionData() = 0;
    virtual void updateTestProfileView() = 0;
    virtual void initializeTestProfileView() = 0;
    virtual void setTestProfileParams() = 0;
signals:
    void sigTestCompletion(uint16_t p_value,QString p_text);
private :

protected:
    QString m_profileName;
    QString m_astmProduct;
    int m_profileId;
    uint16_t m_currentTestMeasurecount;
    uint16_t m_totalTestMeasureSize;
    uint16_t m_currentAcquiredPoint;
    QString m_instructionString;
    QString m_statusString;
    QString m_sample;

};

#endif // ASTMTESTPROFILE_H
