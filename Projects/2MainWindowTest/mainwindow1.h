#ifndef MAINWINDOW1_H
#define MAINWINDOW1_H

#include <QMainWindow>
#include <QDebug>
#include "mainwindow2.h"

namespace Ui {
class MainWindow1;
}

class MainWindow1 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow1(QWidget *parent = nullptr);
    ~MainWindow1();

private slots:
    void on_nextmainwindow_clicked();

private:
    Ui::MainWindow1 *ui;
    MainWindow2 mw2;

};

#endif // MAINWINDOW1_H
