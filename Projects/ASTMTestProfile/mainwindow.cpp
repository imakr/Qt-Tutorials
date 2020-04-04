#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_createtp1_clicked()
{
    if(nullptr == astmprofile)
    {
        astmprofile.reset(new ASTMF2554TP1());
        connect(astmprofile.get(),&ASTMF2554TP1::sigTestCompletion,this,&MainWindow::processTestCompletion,Qt::QueuedConnection);
    }
}
void MainWindow::on_createtp2_clicked()
{
    if(nullptr == astmprofile)
    {
        astmprofile.reset(new ASTMF2554TP2());
        connect(astmprofile.get(),&ASTMF2554TP2::sigTestCompletion,this,&MainWindow::processTestCompletion,Qt::QueuedConnection);
    }
}

void MainWindow::on_initialize_clicked()
{
    QString instructionString,statusString;
    astmprofile->setTotalMeasuresCount(10);
    astmprofile->initializeTestProfileView();
    astmprofile->getInstructionStatusString(instructionString,statusString);
    ui->instruction->setText(instructionString);
    ui->status->setText(statusString);
}

void MainWindow::on_update_clicked()
{
    QString instructionString,statusString;
    astmprofile->updateTestProfileView();
    astmprofile->getInstructionStatusString(instructionString,statusString);
    ui->instruction->setText(instructionString);
    ui->status->setText(statusString);
}

void MainWindow::on_undo_clicked()
{
    QString instructionString,statusString;
    astmprofile->onUndoLastAcquisitionData();
    astmprofile->getInstructionStatusString(instructionString,statusString);
    ui->instruction->setText(instructionString);
    ui->status->setText(statusString);
}

void MainWindow::on_exittp_clicked()
{
    astmprofile.reset();
}

void MainWindow::processTestCompletion(uint16_t p_value, QString p_text)
{
    QString p_message("All " + QString::number(p_value) + p_text + " acquisition completed!");
    QMessageBox::information(this, "Robony ASTM Application", p_message);
}
