#ifndef ASTMF2554TP2_H
#define ASTMF2554TP2_H

#include <QString>
#include "astmtestprofile.h"

class ASTMF2554TP2 : public ASTMTestProfile
{
    Q_OBJECT
public:
    explicit ASTMF2554TP2();
    virtual void onUndoLastAcquisitionData() override;
    virtual void updateTestProfileView() override;
    virtual void initializeTestProfileView() override;
    virtual void setTestProfileParams()override;
    ~ASTMF2554TP2();
signals:

public slots:
};

#endif // ASTMF2554TP2_H
