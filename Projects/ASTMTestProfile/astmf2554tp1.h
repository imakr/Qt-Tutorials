#ifndef ASTMF2554TP1_H
#define ASTMF2554TP1_H

#include <QString>
#include "astmtestprofile.h"

class ASTMF2554TP1 : public ASTMTestProfile
{
    Q_OBJECT
public:
    explicit ASTMF2554TP1();
    virtual ~ASTMF2554TP1();
    virtual void onUndoLastAcquisitionData() override;
    virtual void updateTestProfileView() override;
    virtual void initializeTestProfileView() override;
    virtual void setTestProfileParams()override;
signals:

public slots:
};

#endif // ASTMF2554TP1_H
