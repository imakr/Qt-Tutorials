#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <astmfactorywindow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:    
    void on_f2554button_clicked();

    void on_f3107button_clicked();

private:
    Ui::MainWindow *ui;
    ASTMFactoryWindow *w;
    ASTMProduct* astmproduct;
};

#endif // MAINWINDOW_H
