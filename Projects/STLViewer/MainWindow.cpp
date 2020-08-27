#include <QMessageBox>
#include <QFileDialog>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "STLParser.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowTitle("STLViewer");
    ui->setupUi(this);

    // 3D tab widgets
    QGridLayout *layout3DView = new QGridLayout(ui->tab3D);
    glView = new AppGLWidget(ui->tab3D);

    layout3DView->addWidget(glView);
    ui->tab3D->setLayout(layout3DView);

    // Histogram tab widget
    QGridLayout *layoutAnalysisView = new QGridLayout(ui->tabAnalysis);
    analysisWidget = new HistWidget(ui->tabAnalysis);

    layoutAnalysisView->addWidget(analysisWidget);
    ui->tabAnalysis->setLayout(layoutAnalysisView);

    // test plot
    analysisWidget->replot();

    // statusbar
    statusLabel = new QLabel(ui->statusbar);
    statusLabel->setText("Default model");
    ui->statusbar->addWidget(statusLabel);

    connectSignalsSlots();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignalsSlots()
{
    QObject::connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::LoadFile);
}

void MainWindow::LoadFile()
{
    STLParser parser;
    QString filename = QFileDialog::getOpenFileName(this, "Load ASCII .stl model", QString(), "STL Files (*.stl)");
    // Opening the file
    QFile fileSTL(filename);
    if (false == fileSTL.open(QIODevice::ReadOnly))
    {
        return;
    }

    // Check the STL file is in the ASCII or Binary format
    bool isASCII = true;
    // Header
    char   header[80] = { 0 };
    qint64 headerData = fileSTL.read(header, 80);
    if (headerData < 80)
    {
        // STL Files header size is exactly 80 bytes
        qDebug() << "STL Files header size is not exactly 80 bytes ";
        return ;
    }
    // ASCII STL files starts with 'solid' (not the case of binary STL files in general)
    if (false == QString(header).trimmed().toUpper().startsWith("SOLID"))
    {
        isASCII = false;
    }
    else // Some Binary files do start with SOLID
    {
        // Go back to the beginning of the file
        fileSTL.seek(0);

        QTextStream stream(&fileSTL);
        // Skip first line
        stream.readLine();
        // Check if the second line starts with 'facet'
        QString line = stream.readLine();
        isASCII      = true;
        if ((line.isEmpty())
            || (fileSTL.error() != QFile::NoError)
            || (!QString(line).trimmed().toUpper().startsWith("FACET")))
        {
            isASCII = false;
        }
    }
    // Go back to the beginning of the file
    fileSTL.seek(0);
    qDebug() << "Detected format: " << (isASCII ? "ASCII" : "BINARY");
    fileSTL.close();
    if (!filename.isNull())
    {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Cannot open file - Error number = " << file.error();
            return;
        }
        Model m = parser.parse(file);
        if (m.isInitialized()) {
            glView->SetModel(m);
            glView->show();
            // FIXME: what if filename is veery long?
            statusLabel->setText(filename + ": " + m.GetName());

            // update the histogram
            int nTriangles = m.GetNTriangles();
            QVector<double> x(nTriangles), y(nTriangles);
            for(int i=0; i<nTriangles; ++i) {
                x[i] = m.GetTriangle(i).GetTheta();
                y[i] = m.GetTriangle(i).GetArea();
            }
            analysisWidget->SetHistData(x, y);
            analysisWidget->SetNBins(100);
            analysisWidget->SetBinWidth(1.);
            analysisWidget->SetUserDispRange(-100., 100);
            analysisWidget->Plot();
        } else {
            qDebug() << "Cannot read  ASCII .stl file - Error number = " << file.error();
        }
    } else {
        qDebug() << "file does not exists";
        return;
    }
}
