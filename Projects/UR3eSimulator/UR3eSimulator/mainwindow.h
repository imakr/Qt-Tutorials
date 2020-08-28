#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <URLowSpeedport.hpp>
#include <URHighSpeedport.hpp>
#include <URDashboardServer.hpp>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void setTCP(URLowSpeedport::UR3eTCP newTCP);
private slots:
    void on_URInputPort1_clicked();

    void on_URInputPort2_clicked();

    void on_sendNewTCP_clicked();

    void on_URDashboardPort_clicked();

    void handleDashboardCommand(QString p_command);

private:
    Ui::MainWindow *ui;
    URLowSpeedport *m_lowSpeedServer;
    URHighSpeedport *m_highSpeedServer;
    URDashboardServer *m_dashboardServer;
};

#endif // MAINWINDOW_H
