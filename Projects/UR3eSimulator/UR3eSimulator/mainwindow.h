#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <URLowSpeedport.hpp>
#include <URHighSpeedport.hpp>

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

private:
    Ui::MainWindow *ui;
     URLowSpeedport *s1;
     URHighSpeedport *s2;

};

#endif // MAINWINDOW_H
