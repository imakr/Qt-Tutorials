#ifndef ASTMF2554MAINWINDOW_H
#define ASTMF2554MAINWINDOW_H

#include <QMainWindow>
#include <astmfactorywindow.h>
#include <astmtestprofile.h>
#include <system_error>

class ASTMF2554MainWindow :public ASTMProduct
{
    Q_OBJECT

public:
    explicit ASTMF2554MainWindow(QWidget *parent = nullptr);
    ~ASTMF2554MainWindow();
    enum class TestProfileCode
    {
        TestProfileMenu = 0,					//!< 0.ProfileMenuPage
        SinglePointAccuracyProfile = 1,			//!< 1.Digitalization of single point for Accuracy
        RotationAroundCenterPointProfile = 2,   //!< 2.Rotation around the center point
        ArrayAngledParallelProfile = 3,			//!< 3.Navigation Array angled in parallel
        ArrayAngledPerpendicularProfile = 4,	//!< 4.Navigation Array angled perpendicular
        DigitalizeAllPointsProfile = 5			//!< 5.Digitalization of all individual points
    };
private slots:
    void on_loadTPButton_clicked();

private:
    Ui::ASTMF2554MainWindow *ui;
    std::vector<ASTMTestProfile*> m_astmf2554Profiles;
    std::error_code loadTestProfile();
};

#endif // ASTMF2554MAINWINDOW_H
