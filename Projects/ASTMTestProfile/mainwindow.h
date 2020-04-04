#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "astmtestprofile.h"
#include "astmf2554tp2.h"
#include "astmf2554tp1.h"
#include <memory>
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
    void on_createtp1_clicked();

    void on_createtp2_clicked();

    void on_initialize_clicked();

    void on_update_clicked();

    void on_undo_clicked();

    void on_exittp_clicked();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<ASTMTestProfile> astmprofile;
     QString m_instructionString,m_statusString;
     void processTestCompletion(uint16_t p_value, QString p_text);
};
#endif // MAINWINDOW_H
