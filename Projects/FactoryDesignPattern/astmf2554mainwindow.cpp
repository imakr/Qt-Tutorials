#include "astmf2554mainwindow.h"
#include "ui_astmf2554mainwindow.h"

ASTMF2554MainWindow::ASTMF2554MainWindow(QWidget *parent) :
    ui(new Ui::ASTMF2554MainWindow)
{
    ui->setupUi(this);
    qDebug()<<"ASTM Product : ASTMF2554 Created";
    ASTMTestProfile *tp0 =  new ASTMTestProfile("ASTMF2554","0.Test Profile Menu",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::TestProfileMenu));
    ASTMTestProfile *tp1 =  new ASTMTestProfile("ASTMF2554","1.Single Point Digitization",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::SinglePointAccuracyProfile));
    ASTMTestProfile *tp2 =  new ASTMTestProfile("ASTMF2554","2.Rotation Analysis",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::RotationAroundCenterPointProfile));
    ASTMTestProfile *tp3 =  new ASTMTestProfile("ASTMF2554","3.Parallel Angulation to Camera",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::ArrayAngledParallelProfile));
    ASTMTestProfile *tp4 =  new ASTMTestProfile("ASTMF2554","4.Perpendicular Angulation to Camera",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::ArrayAngledPerpendicularProfile));
    ASTMTestProfile *tp5 =  new ASTMTestProfile("ASTMF2554","5.Digitization of all individual points",static_cast<int>(ASTMF2554MainWindow::TestProfileCode::DigitalizeAllPointsProfile));
    m_astmf2554Profiles.push_back(tp0);
    m_astmf2554Profiles.push_back(tp1);
    m_astmf2554Profiles.push_back(tp2);
    m_astmf2554Profiles.push_back(tp3);
    m_astmf2554Profiles.push_back(tp4);
    m_astmf2554Profiles.push_back(tp5);
    tp1->setLoadFunction(std::bind(&ASTMF2554MainWindow::loadTestProfile, this));
}

ASTMF2554MainWindow::~ASTMF2554MainWindow()
{
    for (auto a : m_astmf2554Profiles)
           delete  a;
    delete ui;
}
//-----------------------------------------------------------------------------
std::error_code ASTMF2554MainWindow::loadTestProfile()
//-----------------------------------------------------------------------------
{
    std::error_code err;

    QString instructionString;
    QString statusString;
    QString sample;
        // m_currentAcquiredPoint = 19;
          //  m_currentTestMeasurecount = 0;
           instructionString = "Please place the pointer at <b>Point 20</b>  and Click on <b>Acquire Point</b> button";
           qDebug()<< instructionString << statusString;
            //sample = "<b>" + QString::number(m_currentTestMeasurecount) + "/" + QString::number(m_testMeasureSize) + "</b>";
            //m_window->setCurrentTestProfileInstruction(instructionString, statusString.append(sample));
return  err;

}

void ASTMF2554MainWindow::on_loadTPButton_clicked()
{
    //std::error_code err = m_astmf2554Profiles.at(1)->doComputation1();
    //qDebug() <<" computation done :" << err.value();

}
