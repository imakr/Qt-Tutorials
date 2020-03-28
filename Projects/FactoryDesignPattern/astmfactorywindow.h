#ifndef ASTMFACTORYWINDOW_H
#define ASTMFACTORYWINDOW_H

#include <QMainWindow>
#include <QDebug>

namespace Ui {
class ASTMProduct;
class ASTMF2554MainWindow;
class ASTMF3107MainWindow;
}


class ASTMProduct : public QMainWindow
{
    Q_OBJECT

public:
    explicit ASTMProduct(QWidget *parent = nullptr);
    ~ASTMProduct();
    /*virtual void onUndoLastAcquisitionData();
    virtual bool validatePointAcqPrecursors(QString& p_errortext);
    virtual QString getTestProfileString();
    virtual void writeASTMTestReportHeader();
    virtual QString getAcqResultString();
    virtual void updateTestProfileView();
    virtual void loadTestProfileView();
    virtual void buildTestProfileLogFile();
    virtual void loadTestProfile();*/
private:   
    //!< The current test profile index
    uint16_t	m_currentTestProfileIndex;

    bool  m_bTestInProgress;

    bool m_bSkipTestStep;

    bool m_PointAcquired;
private slots:

};

class ASTMFactoryWindow
{

public:
    explicit ASTMFactoryWindow();
    ~ASTMFactoryWindow();
    enum class Product
    {
        ASTMF2554,ASTMF3107
    };
    ASTMProduct* createASTMProduct(Product p);

private:
    //Ui::ASTMProduct *ui;
};

#endif // ASTMFACTORYWINDOW_H
