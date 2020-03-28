#ifndef ASTMF3107MAINWINDOW_H
#define ASTMF3107MAINWINDOW_H

#include <QMainWindow>
#include <astmfactorywindow.h>

class ASTMF3107MainWindow : public ASTMProduct
{
    Q_OBJECT

public:
    explicit ASTMF3107MainWindow(QWidget *parent = nullptr);
    ~ASTMF3107MainWindow();
    enum class TestProfileCode
    {
        TestProfileMenu = 0,					//!< 0.ProfileMenuPage
        SinglePointAccuracyProfile = 1,			//!< 1.Digitalization of single point for Accuracy
        RefBaseAttachAndReattachProfile = 2,	//!< 2.Reference Base Test - Detach and Reattach
        RefBaseExtForceAppliedProfile = 3,	    //!< 3.Reference Base Test - External Force Application
        SagittalSawCutProfile = 4				//!< 4.Applied Clinical Task - Sagittal Saw Cut
    };
private:
    Ui::ASTMF3107MainWindow *ui;
};

#endif // ASTMF3107MAINWINDOW_H
