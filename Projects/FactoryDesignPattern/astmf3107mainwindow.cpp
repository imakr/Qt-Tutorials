#include "astmf3107mainwindow.h"
#include "ui_astmf3107mainwindow.h"

ASTMF3107MainWindow::ASTMF3107MainWindow(QWidget *parent) :
  ui(new Ui::ASTMF3107MainWindow)
{
    ui->setupUi(this);
     qDebug()<<"ASTM Product : ASTMF3107 Created";
     ASTMTestProfile *tp0 =  new ASTMTestProfile("ASTMF3017","0.Test Profile Menu",static_cast<int>(ASTMF3107MainWindow::TestProfileCode::TestProfileMenu));
     ASTMTestProfile *tp1 =  new ASTMTestProfile("ASTMF3017","1.Single Point Digitization",static_cast<int>(ASTMF3107MainWindow::TestProfileCode::SinglePointAccuracyProfile));
     ASTMTestProfile *tp2 =  new ASTMTestProfile("ASTMF3017","2.Reference Base Test - Detach and Reattach",static_cast<int>(ASTMF3107MainWindow::TestProfileCode::RefBaseAttachAndReattachProfile));
     ASTMTestProfile *tp3 =  new ASTMTestProfile("ASTMF3017","3.Reference Base Test - External Force",static_cast<int>(ASTMF3107MainWindow::TestProfileCode::RefBaseExtForceAppliedProfile));
     ASTMTestProfile *tp4 =  new ASTMTestProfile("ASTMF3017","4.Applied Clinical Task - Sagittal Saw Cut",static_cast<int>(ASTMF3107MainWindow::TestProfileCode::SagittalSawCutProfile));

     m_astmf3107Profiles.push_back(tp0);
     m_astmf3107Profiles.push_back(tp1);
     m_astmf3107Profiles.push_back(tp2);
     m_astmf3107Profiles.push_back(tp3);
     m_astmf3107Profiles.push_back(tp4);

}

ASTMF3107MainWindow::~ASTMF3107MainWindow()
{
    for (auto a : m_astmf3107Profiles)
           delete  a;
    delete ui;
}
