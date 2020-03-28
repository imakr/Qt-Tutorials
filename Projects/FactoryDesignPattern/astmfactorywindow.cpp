#include "astmfactorywindow.h"
#include "astmf2554mainwindow.cpp"
#include "astmf3107mainwindow.cpp"

ASTMProduct::ASTMProduct(QWidget *parent) :
 QMainWindow(parent)
{

}

ASTMProduct::~ASTMProduct()
{

}

ASTMFactoryWindow::ASTMFactoryWindow()
{

}

ASTMFactoryWindow::~ASTMFactoryWindow()
{

}

ASTMProduct* ASTMFactoryWindow::createASTMProduct(ASTMFactoryWindow::Product p)
{
    switch(p)
    {
        case ASTMFactoryWindow::Product::ASTMF2554:
        {
            return new ASTMF2554MainWindow();
        }
            break;
        case ASTMFactoryWindow::Product::ASTMF3107:
        {
             return new ASTMF3107MainWindow();
             break;
        }
    }
    return nullptr;
}



