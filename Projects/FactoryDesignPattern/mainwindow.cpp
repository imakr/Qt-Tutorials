#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    w = nullptr;
    astmproduct = nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
    if(nullptr != w)delete w;
    if(nullptr != astmproduct)delete astmproduct;
}

void MainWindow::on_f2554button_clicked()
{
    if(nullptr == w)
    {
        w = new ASTMFactoryWindow();
        astmproduct = w->createASTMProduct(ASTMFactoryWindow::Product::ASTMF2554);
        this->hide();
        astmproduct->show();
    }
}

void MainWindow::on_f3107button_clicked()
{
    if(nullptr == w)
    {
        w = new ASTMFactoryWindow();
        astmproduct = w->createASTMProduct(ASTMFactoryWindow::Product::ASTMF3107);
        this->hide();
        astmproduct->show();
    }
}
