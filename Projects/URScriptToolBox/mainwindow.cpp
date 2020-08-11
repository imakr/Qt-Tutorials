#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTransform>
static const float ANGULAR_MAX_THRESHOLD_VALUE_CAM      = 8.05f;
static const float ANGULAR_MIN_THRESHOLD_VALUE_CAM      = -8.05f;
static const float LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM  = 100.05f;
static const float LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM  = -100.05f;

static const double PiValue = 3.14159;
static const double Degree180 = 180.0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_CurrentTCP.ur3X_m= 0;
    m_CurrentTCP.ur3Y_m=0;
    m_CurrentTCP.ur3Z_m=0;
    m_CurrentTCP.ur3RX_rad=0;
    m_CurrentTCP.ur3RY_rad=0;
    m_CurrentTCP.ur3RZ_rad=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectButton_clicked()
{

    getNewCameraFile(m_CamfilePath);
    ui->filePath->setText(m_CamfilePath);
}
void MainWindow::updateURTCP()
{
    m_CurrentTCP.ur3X_m= ui->x_tcp->text().toDouble();
    m_CurrentTCP.ur3Y_m=ui->y_tcp->text().toDouble();
    m_CurrentTCP.ur3Z_m=ui->z_tcp->text().toDouble();
    m_CurrentTCP.ur3RX_rad=ui->rx_tcp->text().toDouble();
    m_CurrentTCP.ur3RY_rad=ui->ry_tcp->text().toDouble();
    m_CurrentTCP.ur3RZ_rad=ui->rz_tcp->text().toDouble();
}
void MainWindow::on_computeButton_clicked()
{
    if(!m_CamfilePath.isEmpty())
    {
        updateURTCP();
        ui->filePath->setText(m_CamfilePath);
        convertCamerLogToDBFile(m_CamfilePath);
    }
}
void MainWindow::getNewCameraFile(QString& filePath)
{
    QFileDialog fileDialog ;
    filePath = fileDialog.getOpenFileName(this,"Select File",QDir::currentPath(),"CSV(*.csv)");
    qDebug()<<filePath;
}
QMatrix4x4 MainWindow::getTranformationMatrixFromRotationVector(UR3eData p_ur3Data)
{
    float tcp_x = static_cast<float>(p_ur3Data.ur3X_m);
    float tcp_y = static_cast<float>(p_ur3Data.ur3Y_m);
    float tcp_z = static_cast<float>(p_ur3Data.ur3Z_m);
    float tcp_rx = static_cast<float>(p_ur3Data.ur3RX_rad);
    float tcp_ry = static_cast<float>(p_ur3Data.ur3RY_rad);
    float tcp_rz = static_cast<float>(p_ur3Data.ur3RZ_rad);

    qDebug()<<"************ URScriptManager : getTranformationMatrixFromRotationVector Called *****************";
    QVector3D rotVector(tcp_rx,tcp_ry,tcp_rz);
    qDebug()<<"1.RotVector:"<<rotVector.x()<<rotVector.y()<<rotVector.z();

    float angle = rotVector.length();
    rotVector.normalize();

    /*float fRx,fRy,fRz;
    fRx = rotVector.x();
    fRy = rotVector.y();
    fRz = rotVector.z();
    //qDebug()<<"1.2.xyz:"<<x<<y<<z;
    //qDebug()<<"1.RotVector(after normalize):Q"<<rotVector;
    //qDebug()<<"1.angle"<<angle;
    float fC = angle/2;
    float fSd = sin(fC);
    float fQx = fRx * fSd;
    float fQy = fRy * fSd;
    float fQz = fRz * fSd;
    float fQw = cos(fC);

    QQuaternion oQuaternion(fQw,fQx,fQy,fQz) ;*/

    QQuaternion oQuaternion = QQuaternion::fromAxisAndAngle(rotVector,(angle/3.14159f)*180);
    //qDebug()<<"1.Quaternion:Q"<<oQuaternion;
    //qDebug()<<oQuaternion.scalar();
    //qDebug()<<oQuaternion.x();
    //qDebug()<<oQuaternion.y();
    //qDebug()<<oQuaternion.z();
    // qDebug()<<"2.Quaternion:Q"<<oQuaternion;

    QMatrix3x3 rotMat = oQuaternion.toRotationMatrix();
    // qDebug()<<"3.RotMat:"<<rotMat;
    m_rotMatCurrent3x3 = rotMat;
    float *fpRotMat = rotMat.data();

    //Create the 4x4 Transformation Matrix from Rotation Matrix and XYZ Position
    QMatrix4x4 transMat(fpRotMat[0],fpRotMat[3],fpRotMat[6],tcp_x
            ,fpRotMat[1],fpRotMat[4],fpRotMat[7],tcp_y
            ,fpRotMat[2],fpRotMat[5],fpRotMat[8],tcp_z
            ,0 ,   0,   0,   1);
    qDebug()<<"4.Calculate trans_mat:"<<transMat;
    return transMat;
}

int16_t MainWindow::convertCamerLogToDBFile(QString p_fileNameWithPath)
{
    memset(&m_OldcamData,0,sizeof(m_OldcamData));
    memset(&m_NewcamData,0,sizeof(m_NewcamData));
    QFileInfo fileInfo;
    fileInfo.setFile(m_CamfilePath);
    QString fileName = fileInfo.fileName();
    fileName.chop(4);
    QString camFilePath = QDir::currentPath().append(DeltaMatrixFileName);
    camFilePath.chop(4);
    camFilePath.append("_").append(fileName).append(".csv");
    QFile outCamFile(camFilePath);
    ui->deltaOutputFile->setText(camFilePath);
    outCamFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outCamFile);
    QFile inputCamFile(p_fileNameWithPath);
    inputCamFile.open(QIODevice::ReadOnly|QFile::Text);
    QTextStream inData(&inputCamFile);
    QString getTempData=inData.readLine();
    qDebug()<<"getTempData:ReadLine:"<<getTempData;
    int xcount = 0,ycount=0,zcount=0,xyzcount=0,linecount=0;
    if( getTempData.compare("Camera Timestamp (ms);r1c1;r1c2;r1c3;r1c4;r2c1;r2c2;r2c3;r2c4;r3c1;r3c2;r3c3;r3c4",Qt::CaseSensitive) != 0)
    {
        qDebug()<<"FileManager:INVALID CAMERA FILE FORMAT DETECTED..";
        QMessageBox::information(this,"UR Script Utility","INVALID CAMERA FILE FORMAT DETECTED.");
        return FAILURE;
    }
    else
    {
        qDebug()<<"FileManager:CAMERA FILE FORMAT OK..";
    }
    //read the second line.
    if(!inData.atEnd())
    {
        linecount++;
        getTempData=inData.readLine();
        dataSeparation(getTempData,m_OldcamData);
        // pd.setValue(count++);
        QMatrix4x4 FirstTFMat(m_OldcamData.row1Col1,m_OldcamData.row1Col2,m_OldcamData.row1Col3,m_OldcamData.row1Col4,
                              m_OldcamData.row2Col1,m_OldcamData.row2Col2,m_OldcamData.row2Col3,m_OldcamData.row2Col4,
                              m_OldcamData.row3Col1,m_OldcamData.row3Col2,m_OldcamData.row3Col3,m_OldcamData.row3Col4,
                              0 ,   0,   0,   1);
        m_FirstTFInCamCSV = FirstTFMat;
        qDebug()<<"convertCamerLogToDBFile:m_FirstTFInCamCSV :" <<m_FirstTFInCamCSV;
    }
    outData.setRealNumberPrecision(10);
    getTempData.clear();
    while (!inData.atEnd())
    {
        linecount++;
        getTempData=inData.readLine();
        dataSeparation(getTempData,m_NewcamData);
        qDebug()<<"m_NewcamData "<<m_NewcamData.timeStamp_ms<<m_NewcamData.row1Col1<<m_NewcamData.row1Col2<<m_NewcamData.row1Col3
               <<m_NewcamData.row1Col4<<m_NewcamData.row2Col1<<m_NewcamData.row2Col2<<m_NewcamData.row2Col3<<m_NewcamData.row2Col4
              <<m_NewcamData.row3Col1<<m_NewcamData.row3Col2<<m_NewcamData.row3Col3<<m_NewcamData.row3Col4;
        qDebug()<<"previousCamData "<<m_OldcamData.timeStamp_ms<<m_OldcamData.row1Col1<<m_OldcamData.row1Col2
               <<m_OldcamData.row1Col3<<m_OldcamData.row1Col4<<m_OldcamData.row2Col1<<m_OldcamData.row2Col2
              <<m_OldcamData.row2Col3<<m_OldcamData.row2Col4<<m_OldcamData.row3Col1<<m_OldcamData.row3Col2
             <<m_OldcamData.row3Col3<<m_OldcamData.row3Col4;

        QMatrix4x4 NewtransMat(m_NewcamData.row1Col1,m_NewcamData.row1Col2,m_NewcamData.row1Col3,m_NewcamData.row1Col4,
                               m_NewcamData.row2Col1,m_NewcamData.row2Col2,m_NewcamData.row2Col3,m_NewcamData.row2Col4,
                               m_NewcamData.row3Col1,m_NewcamData.row3Col2,m_NewcamData.row3Col3,m_NewcamData.row3Col4,
                               0 ,   0,   0,   1);
        QMatrix4x4 OldtransMat(m_OldcamData.row1Col1,m_OldcamData.row1Col2,m_OldcamData.row1Col3,m_OldcamData.row1Col4,
                               m_OldcamData.row2Col1,m_OldcamData.row2Col2,m_OldcamData.row2Col3,m_OldcamData.row2Col4,
                               m_OldcamData.row3Col1,m_OldcamData.row3Col2,m_OldcamData.row3Col3,m_OldcamData.row3Col4,
                               0 ,   0,   0,   1);
        //qDebug() <<"Calculate NewtransMat ="<< NewtransMat;
        //qDebug() <<"Calculate OldtransMat ="<< OldtransMat;
        QMatrix4x4 deltransMat = OldtransMat.inverted() * NewtransMat;         
        float *fpDeltaMat = deltransMat.data();
        //qDebug() <<"Calculate deltransMat ="<< deltransMat;
        //qDebug() << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<","<<
        //static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<","<<
        //static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14]);
        fpDeltaMat[12]/=1000; //Convert the displacement in X from mm to m
        fpDeltaMat[13]/=1000; //Convert the displacement in Y from mm to m
        fpDeltaMat[14]/=1000; //Convert the displacement in Z from mm to m
        if(fpDeltaMat[12] < float(0.00005))      xcount++;
        if(fpDeltaMat[13] < float(0.00005))      ycount++;
        if(fpDeltaMat[14] < float(0.00005))      zcount++;

        if((fpDeltaMat[12] < float(0.00005))&&(fpDeltaMat[13] < float(0.00005))&&(fpDeltaMat[14] < float(0.00005)))
            xyzcount++;
        outData << fixed << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<","<<
                                                                                                                                                                                            static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                            static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14])<<","<<
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             static_cast<double>(fpDeltaMat[3])<<","<<static_cast<double>(fpDeltaMat[7])<<","<<static_cast<double>(fpDeltaMat[11])<<","<<static_cast<double>(fpDeltaMat[15])<<'\n';
        memcpy(&m_OldcamData,&m_NewcamData,sizeof(m_NewcamData));
        memset(&m_NewcamData,0,sizeof(m_NewcamData));
    }
    outCamFile.close();
    inputCamFile.close();
    getTempData.clear();
    qDebug()<<"LINE COUNT VALUES = " << linecount;
    qDebug()<<"INVALID X VALUES = " << xcount;
    qDebug()<<"INVALID Y VALUES = " << ycount;
    qDebug()<<"INVALID Z VALUES = " << zcount;
    qDebug()<<"INVALID XYZ VALUES = " << xyzcount;
    QMessageBox::information(this,"UR Script Utility","Delta Transformation Matrix computation done!!");
    return SUCCESS;
}

int16_t MainWindow::dataSeparation(QString& p_data,CameraData& p_camData)
{
    QStringList csplitString=p_data.split(";");
    qDebug()<<"SplitString:"<<csplitString;
    if(csplitString.isEmpty())
    {
        qDebug()<<"Empty";
        return FAILURE;
    }

    p_camData.timeStamp_ms = csplitString[CAM_DATA_SEP_DB_TIME].toInt();
    p_camData.row1Col1 = csplitString[CAM_DATA_ROW1COL1].toFloat();
    p_camData.row1Col2 = csplitString[CAM_DATA_ROW1COL2].toFloat();
    p_camData.row1Col3 = csplitString[CAM_DATA_ROW1COL3].toFloat();
    p_camData.row1Col4 = csplitString[CAM_DATA_ROW1COL4].toFloat();// Row 1 for X values
    p_camData.row2Col1 = csplitString[CAM_DATA_ROW2COL1].toFloat();
    p_camData.row2Col2 = csplitString[CAM_DATA_ROW2COL2].toFloat();
    p_camData.row2Col3 = csplitString[CAM_DATA_ROW2COL3].toFloat();
    p_camData.row2Col4 = csplitString[CAM_DATA_ROW2COL4].toFloat();//Row 2 for Y values
    p_camData.row3Col1 = csplitString[CAM_DATA_ROW3COL1].toFloat();
    p_camData.row3Col2 = csplitString[CAM_DATA_ROW3COL2].toFloat();
    p_camData.row3Col3 = csplitString[CAM_DATA_ROW3COL3].toFloat();
    p_camData.row3Col4 = csplitString[CAM_DATA_ROW3COL4].toFloat();//Row 3 for Z values

    return SUCCESS;
}
uint16_t MainWindow::getURDataAfterTransformation(UR3eData& p_ur3Data)
{

    m_rotMatCurrent3x3 = m_CurrentTransform.normalMatrix();
    //qDebug()<<"RotMat_Expected_normal:"<<m_rotMatCurrent3x3;

    QQuaternion expectedQuaternion = QQuaternion::fromRotationMatrix(m_rotMatCurrent3x3);
    float x, y,z,angle;
    expectedQuaternion.getAxisAndAngle(&x,&y,&z,&angle);
    angle *= 3.14159f/180;
    p_ur3Data.ur3RX_rad = static_cast<double>(x)*angle;
    p_ur3Data.ur3RY_rad = static_cast<double>(y)*angle;
    p_ur3Data.ur3RZ_rad = static_cast<double>(z)*angle;

    /*expectedQuaternion.normalize();
        float fAxis[3]={};
        float fQw = expectedQuaternion.scalar();
        float fQx = expectedQuaternion.x();
        float fQy = expectedQuaternion.y();
        float fQz = expectedQuaternion.z();
        //qDebug()<<"Expected_Quaternion:"<<expectedQuaternion;

        float angle = 2 * acos(fQw);
        double exp = static_cast<double>(1 - (fQw * fQw));
        if (exp < 0.000001)
        {
            fAxis[0] = fQx;
            fAxis[1] = fQy;
            fAxis[2] = fQz;
        }
        else
        {

            float fSqr = sqrt(1 - (fQw * fQw));
            fAxis[0] = fQx / fSqr;
            fAxis[1] = fQy / fSqr;
            fAxis[2] = fQz / fSqr;
        }
        fAxis[0] *= angle;
        fAxis[1] *= angle;
        fAxis[2] *= angle;
        p_ur3Data.ur3RX_rad = static_cast<double>(fAxis[0]);
        p_ur3Data.ur3RY_rad = static_cast<double>(fAxis[1]);
        p_ur3Data.ur3RZ_rad = static_cast<double>(fAxis[2]);*/
    float *pTransfMat = m_CurrentTransform.data();
    p_ur3Data.ur3X_m = static_cast<double>(pTransfMat[12]);
    p_ur3Data.ur3Y_m = static_cast<double>(pTransfMat[13]);
    p_ur3Data.ur3Z_m = static_cast<double>(pTransfMat[14]);
    return 0;
}

void MainWindow::on_genScriptBtn_clicked()
{
    QMatrix4x4 first,second;
    qDebug() << " ********** genScriptBtn - Called ********" <<endl;
    QString camFilePath = QDir::currentPath().append(DeltaMatrixFileName);
    QFile inData(camFilePath);
    inData.open(QIODevice::ReadOnly | QFile::Text);

    QString getTempData;
    qDebug()<<"getTempData:ReadLine:"<<getTempData;
    QStringList csplitString;

    QString URScriptFilePath = QDir::currentPath().append(URScriptFileName);
    ui->URSrciptOutputFile->setText(URScriptFilePath);
    QFile outCamFile(URScriptFilePath);
    outCamFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outCamFile);
    outData.setRealNumberPrecision(10);
    first = getTranformationMatrixFromRotationVector(m_CurrentTCP);
    //float *fpfirstMat = first.data();
    qDebug() <<"Calculated first ="<< first;
    while (!inData.atEnd())
    {
        getTempData=inData.readLine();
        csplitString=getTempData.split(",");

        QMatrix4x4 deltransMat(csplitString[0].toFloat(),csplitString[1].toFloat(),csplitString[2].toFloat(),csplitString[3].toFloat(),
                csplitString[4].toFloat(),csplitString[5].toFloat(),csplitString[6].toFloat(),csplitString[7].toFloat(),
                csplitString[8].toFloat(),csplitString[9].toFloat(),csplitString[10].toFloat(),csplitString[11].toFloat(),
                0 ,   0,   0,   1);
        /*QMatrix4x4 deltransMat(1,0,0,csplitString[3].toFloat(),
                               0,1,0,csplitString[7].toFloat(),
                               0,0,1,csplitString[11].toFloat(),
                               0 ,   0,   0,   1);*/
        /*QMatrix4x4 deltransMat(1,0,0,0,
                               0,1,0,0,
                               0,0,1,0,
                               0,0,0,1);*/
        qDebug() <<"Existing m_FirstTFInCamCSV ="<< first;
        qDebug() <<"Read csplitString ="<< csplitString;
        qDebug() <<"Read deltransMat ="<< deltransMat;
        second = first * deltransMat;
        float *fpSecondMat = second.data();
        qDebug() <<"Calculated second ="<< second;
        m_CurrentTransform = second;
        getURDataAfterTransformation(m_CurrentTCP);
        outData << m_CurrentTCP.ur3X_m << "," <<m_CurrentTCP.ur3Y_m <<","<<m_CurrentTCP.ur3Z_m<<","<<m_CurrentTCP.ur3RX_rad<<","
                <<m_CurrentTCP.ur3RY_rad<<","<<m_CurrentTCP.ur3RZ_rad<<"\n";
        first = second;
    }
    inData.close();
    outCamFile.close();
    QMessageBox::information(this,"UR Script Utility","UR3e Script Generation done!!");
}

void MainWindow::on_testButton_clicked()
{
    /*QMatrix4x4 first(-0.0287631999,-0.939378791,0.341672597,62.58829645,
                     -0.034520182,0.34254362,0.9388675,88.718773,
                     -0.9989900,0.015210234778,-0.042280175,1825.2824,
                     0,0,0,1);
    QMatrix4x4 second(-0.029173549,-0.939344,0.34173092,62.5977856,
                      -0.03428407,0.34261560,0.938849906,88.715789,
                      -0.9989862319,0.0156736561,-0.04219988,1825.31272,
                      0,0,0,1);*/
    /*QMatrix4x4 second(-0.029173549,-0.939344,0.34173092,62.5977856,
                      -0.03428407,0.34261560,0.938849906,88.715789,
                      -0.9989862319,0.0156736561,-0.04219988,1825.31272,
                      0,0,0,1);
        QMatrix4x4 first(-0.028837592462113093,-0.9393776002370053,0.34166960273625985,62.592388691967585,
                          -0.03429016379658825,0.34254025125406595,0.9388771809654359,88.70317723076408,
                          -0.9989957847397518,0.015359050874517235,-0.042089447946507486,1825.248030778022,
                          0,0,0,1);*/

    QMatrix4x4 first(-0.029173549,-0.939344,0.34173092,62.5977856,
                     -0.03428407,0.34261560,0.938849906,88.715789,
                     -0.9989862319,0.0156736561,-0.04219988,1825.31272,
                     0,0,0,1);
    QMatrix4x4 second(-0.028837592462113093,-0.9393776002370053,0.34166960273625985,62.592388691967585,
                      -0.03429016379658825,0.34254025125406595,0.9388771809654359,88.70317723076408,
                      -0.9989957847397518,0.015359050874517235,-0.042089447946507486,1825.248030778022,
                      0,0,0,1);
    QMatrix4x4 deltransMat = first.inverted() * second;
    qDebug()<<"testing Tranformation logic - deltransMat:" <<deltransMat;
    QMatrix4x4 expected_second = first*deltransMat;
    qDebug()<<"testing Tranformation logic - expected_second:" <<expected_second;

    //Below Generated from python script.
    /*Calculated Delta TF Matrix:
     [ 1.000000 -0.000713  0.000058 -0.081450]
      [ 0.000713  1.000000 -0.000132 -0.007811]
      [-0.000058  0.000132  1.000000  0.016852]
      [ 0.000000  0.000000  0.000000  1.000000]]
     Calculated New TF Matrix:
     [[-0.018918 -0.938368  0.345119  60.760274]
      [ 0.071129  0.343043  0.936623  88.931784]
      [-0.997288  0.042267  0.060256  1780.082975]
      [ 0.000000  0.000000  0.000000  1.000000]]
     Expected New TF Matrix:
     [[-0.018918 -0.938368  0.345119  60.760274]
      [ 0.071129  0.343043  0.936623  88.931784]
      [-0.997288  0.042267  0.060256  1780.082975]
      [ 0.000000  0.000000  0.000000  1.000000]]
*/
}

void MainWindow::on_computeExpTF_clicked()
{
    QMatrix4x4 first,second;
    qDebug() << " ********** computeExpectedTFMatrix - Called ********" <<endl;

    QString camFilePath = QDir::currentPath().append(DeltaMatrixFileName);
    QFile inData(camFilePath);
    inData.open(QIODevice::ReadOnly | QFile::Text);

    QString getTempData;
    qDebug()<<"getTempData:ReadLine:"<<getTempData;
    QStringList csplitString;

    QString URScriptFilePath = QDir::currentPath().append(ExpectedTFFileName);
    ui->expTransformOutput->setText(URScriptFilePath);
    QFile outCamFile(URScriptFilePath);
    outCamFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outCamFile);
    outData.setRealNumberPrecision(10);
    first= m_FirstTFInCamCSV ;
    float *fpfirstMat = first.data();
    qDebug() <<"Calculated first ="<< first;
    outData << fixed << static_cast<double>(fpfirstMat[0])<<","<<static_cast<double>(fpfirstMat[4])<<","<<static_cast<double>(fpfirstMat[8])<<","<<static_cast<double>(fpfirstMat[12])<<","<<
                                                                                                                                                                                        static_cast<double>(fpfirstMat[1])<<","<<static_cast<double>(fpfirstMat[5])<<","<<static_cast<double>(fpfirstMat[9])<<","<<static_cast<double>(fpfirstMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                        static_cast<double>(fpfirstMat[2])<<","<<static_cast<double>(fpfirstMat[6])<<","<<static_cast<double>(fpfirstMat[10])<<","<<static_cast<double>(fpfirstMat[14])<<","<<"\n";
    while (!inData.atEnd())
    {
        getTempData=inData.readLine();
        csplitString=getTempData.split(",");
        QMatrix4x4 deltransMat(csplitString[0].toFloat(),csplitString[1].toFloat(),csplitString[2].toFloat(),csplitString[3].toFloat(),
                csplitString[4].toFloat(),csplitString[5].toFloat(),csplitString[6].toFloat(),csplitString[7].toFloat(),
                csplitString[8].toFloat(),csplitString[9].toFloat(),csplitString[10].toFloat(),csplitString[11].toFloat(),
                0 ,   0,   0,   1);
        qDebug() <<"Existing m_FirstTFInCamCSV ="<< first;
        qDebug() <<"Read csplitString ="<< csplitString;
        qDebug() <<"Read deltransMat ="<< deltransMat;
        second = first * deltransMat;
        float *fpSecondMat = second.data();
        qDebug() <<"Calculated second ="<< second;
        outData << fixed << static_cast<double>(fpSecondMat[0])<<","<<static_cast<double>(fpSecondMat[4])<<","<<static_cast<double>(fpSecondMat[8])<<","<<static_cast<double>(fpSecondMat[12])<<","<<
                                                                                                                                                                                                static_cast<double>(fpSecondMat[1])<<","<<static_cast<double>(fpSecondMat[5])<<","<<static_cast<double>(fpSecondMat[9])<<","<<static_cast<double>(fpSecondMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                                    static_cast<double>(fpSecondMat[2])<<","<<static_cast<double>(fpSecondMat[6])<<","<<static_cast<double>(fpSecondMat[10])<<","<<static_cast<double>(fpSecondMat[14])<<","<<"\n";
        first = second;
    }
    inData.close();
    outCamFile.close();
    QMessageBox::information(this,"UR Script Utility","Expected Transformation Matrix computation done!!");
}

void MainWindow::on_resetTCPButton_clicked()
{
    m_CurrentTCP.ur3X_m= 0.323368912;
    m_CurrentTCP.ur3Y_m=-0.004230606;
    m_CurrentTCP.ur3Z_m=0.299153483;
    m_CurrentTCP.ur3RX_rad=-1.209355217;
    m_CurrentTCP.ur3RY_rad=1.209209110;
    m_CurrentTCP.ur3RZ_rad=-1.209292167;
    ui->x_tcp->setText(QString::number(m_CurrentTCP.ur3X_m));
    ui->y_tcp->setText(QString::number(m_CurrentTCP.ur3Y_m));
    ui->z_tcp->setText(QString::number(m_CurrentTCP.ur3Z_m));
    ui->rx_tcp->setText(QString::number(m_CurrentTCP.ur3RX_rad));
    ui->ry_tcp->setText(QString::number(m_CurrentTCP.ur3RY_rad));
    ui->rz_tcp->setText(QString::number(m_CurrentTCP.ur3RZ_rad));
    qDebug()<<"*************** on_resetTCPButton_clicked ************ ";
    qDebug()<<ui->x_tcp->text();
    qDebug()<<ui->y_tcp->text();
    qDebug()<<ui->z_tcp->text();
    qDebug()<<ui->rx_tcp->text();
    qDebug()<<ui->ry_tcp->text();
    qDebug()<<ui->rz_tcp->text();

}

void MainWindow::on_getTMButton_clicked()
{
    updateURTCP();
    getTranformationMatrixFromRotationVector(m_CurrentTCP);
}

void MainWindow::on_getRVButton_clicked()
{
    qDebug()<<"*************** on_getRVButton_clicked ************ ";
    updateURTCP();
    m_CurrentTransform = getTranformationMatrixFromRotationVector(m_CurrentTCP);
    qDebug() << "Original TCP : "<< m_CurrentTCP.ur3X_m << "," <<m_CurrentTCP.ur3Y_m <<","<<m_CurrentTCP.ur3Z_m<<","<<m_CurrentTCP.ur3RX_rad<<","
             <<m_CurrentTCP.ur3RY_rad<<","<<m_CurrentTCP.ur3RZ_rad<<"\n";
    qDebug() << "Original m_CurrentTransform :";//m_CurrentTransform;
    float *fpSecondMat = m_CurrentTransform.data();

    qDebug() << fixed << static_cast<double>(fpSecondMat[0])<<","<<static_cast<double>(fpSecondMat[4])<<","<<static_cast<double>(fpSecondMat[8])<<","<<static_cast<double>(fpSecondMat[12])<<","<<
                                                                                                                                                                                             static_cast<double>(fpSecondMat[1])<<","<<static_cast<double>(fpSecondMat[5])<<","<<static_cast<double>(fpSecondMat[9])<<","<<static_cast<double>(fpSecondMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                                 static_cast<double>(fpSecondMat[2])<<","<<static_cast<double>(fpSecondMat[6])<<","<<static_cast<double>(fpSecondMat[10])<<","<<static_cast<double>(fpSecondMat[14])<<","<<"\n";

    getURDataAfterTransformation(m_CurrentTCP);
    qDebug() << "Re-Calculated TCP : "<< m_CurrentTCP.ur3X_m << "," <<m_CurrentTCP.ur3Y_m <<","<<m_CurrentTCP.ur3Z_m<<","<<m_CurrentTCP.ur3RX_rad<<","
             <<m_CurrentTCP.ur3RY_rad<<","<<m_CurrentTCP.ur3RZ_rad<<"\n";
    m_CurrentTransform = getTranformationMatrixFromRotationVector(m_CurrentTCP);
    qDebug() << "Re-Calculated m_CurrentTransform : "<<m_CurrentTransform;

}

void MainWindow::on_splitcamfile_clicked()
{
    QFile inputCamFile(m_CamfilePath);
    inputCamFile.open(QIODevice::ReadOnly|QFile::Text);
    QTextStream inData(&inputCamFile);
    int filenum = 1;
    int maxloc = ui->locLineEdit->text().toInt();
    int lineswrote = 0;
    QString getTempData;
    getTempData=inData.readLine();
    qDebug()<<"getTempData:ReadLine:"<<getTempData;

    if( getTempData.compare("Camera Timestamp (ms);r1c1;r1c2;r1c3;r1c4;r2c1;r2c2;r2c3;r2c4;r3c1;r3c2;r3c3;r3c4",Qt::CaseSensitive) != 0)
    {
        qDebug()<<"FileManager:INVALID CAMERA FILE FORMAT DETECTED..";
        QMessageBox::information(this,"UR Script Utility","INVALID CAMERA FILE FORMAT DETECTED..");
        return;
    }
    else
    {
        qDebug()<<"FileManager:CAMERA FILE FORMAT OK..";
    }
    while(!inData.atEnd())
    {
        QString camSplitFilePath(m_CamfilePath);
        camSplitFilePath.chop(4);
        camSplitFilePath.append("_");
        camSplitFilePath.append((QString::number(filenum)+".csv"));
        QFile outCamFile(camSplitFilePath);
        outCamFile.open(QIODevice::WriteOnly | QFile::Text);
        QTextStream outData(&outCamFile);
        outData << "Camera Timestamp (ms);r1c1;r1c2;r1c3;r1c4;r2c1;r2c2;r2c3;r2c4;r3c1;r3c2;r3c3;r3c4"<<"\n";
        while(lineswrote <= maxloc)
        {
            if(!inData.atEnd())
            {
                getTempData=inData.readLine();
                outData << getTempData <<"\n";
                lineswrote++;
            }else
            {
                //Input File EOF detected before writing maxloc LOC
                outCamFile.close();
                break;
            }
        }
        outCamFile.close();
        lineswrote=0;
        filenum++;
    }
    inputCamFile.close();
    QMessageBox::information(this,"UR Script Utility","Splitting of Camera File is done!!");
}

void MainWindow::on_applyTransformation_clicked()
{
    updateURTCP();
    QMatrix4x4 currentTranformMatrix = getTranformationMatrixFromRotationVector(m_CurrentTCP);
    float dx_mm = ui->dX_mm->text().toFloat();
    float dy_mm = ui->dY_mm->text().toFloat();
    float dz_mm = ui->dZ_mm->text().toFloat();
    float rx_rad = (ui->rX_deg->text().toFloat()*3.14159f)/180;
    float ry_rad = (ui->rY_deg->text().toFloat()*3.14159f)/180;
    float rz_rad = (ui->rZ_deg->text().toFloat()*3.14159f)/180;
    QMatrix4x4 beforeTranformMatrix = currentTranformMatrix;
    currentTranformMatrix.translate(dx_mm,dy_mm,dz_mm);
    QVector3D rotVector(rx_rad,ry_rad,rz_rad);
    qDebug()<<"1.RotVector:"<<rotVector.x()<<rotVector.y()<<rotVector.z();

    float angle = rotVector.length();
    rotVector.normalize();

    QQuaternion oQuaternion = QQuaternion::fromAxisAndAngle(rotVector,(angle/3.14159f)*180);
    currentTranformMatrix.rotate(oQuaternion);
    qDebug()<<"Newly Transformed Matrix :" <<currentTranformMatrix;

    QMatrix4x4 deltaTranformMatrix = beforeTranformMatrix.inverted()*currentTranformMatrix;
    qDebug()<<"Delta Transformed Matrix :" <<deltaTranformMatrix;


}

void MainWindow::on_applyMatrixTransform_clicked()
{
    float deltaX_mm = 0.0 ;
    float deltaY_mm = 0.0;
    float deltaZ_mm = 0.0;
    float deltaXvec_deg = 0.0;
    float deltaYvec_deg = 0.0;
    float deltaZvec_deg = 0.0;
    QMatrix4x4 inputMatrix(ui->in_r1c1->text().toFloat(),ui->in_r1c2->text().toFloat(),ui->in_r1c3->text().toFloat(),ui->in_r1c4->text().toFloat(),
                           ui->in_r2c1->text().toFloat(),ui->in_r2c2->text().toFloat(),ui->in_r2c3->text().toFloat(),ui->in_r2c4->text().toFloat(),
                           ui->in_r3c1->text().toFloat(),ui->in_r3c2->text().toFloat(),ui->in_r3c3->text().toFloat(),ui->in_r3c4->text().toFloat(),
                           0 ,   0,   0,   1);

    float dx_mm = ui->mat_dX_mm->text().toFloat();
    float dy_mm = ui->mat_dY_mm->text().toFloat();
    float dz_mm = ui->mat_dZ_mm->text().toFloat();
    float rx_rad = (ui->mat_rX_deg->text().toFloat()*3.14159f)/180;
    float ry_rad = (ui->mat_rY_deg->text().toFloat()*3.14159f)/180;
    float rz_rad = (ui->mat_rZ_deg->text().toFloat()*3.14159f)/180;
    QMatrix4x4 beforeTranformMatrix = inputMatrix;
    inputMatrix.translate(dx_mm,dy_mm,dz_mm);
    QVector3D rotVector(rx_rad,ry_rad,rz_rad);
    qDebug()<<"1.RotVector:"<<rotVector.x()<<rotVector.y()<<rotVector.z();

    float angle = rotVector.length();
    rotVector.normalize();

    QQuaternion oQuaternion = QQuaternion::fromAxisAndAngle(rotVector,(angle/3.14159f)*180);
    inputMatrix.rotate(oQuaternion);
    qDebug()<<"Newly Transformed Matrix :" <<inputMatrix;

    QMatrix4x4 deltaTranformMatrix = beforeTranformMatrix.inverted()*inputMatrix;
    qDebug()<<"Delta Transformed Matrix :" <<deltaTranformMatrix;

    float *fpDeltaMatPtr = deltaTranformMatrix.data();
    //qDebug() <<"Calculate deltransMat ="<< deltransMat;
    //qDebug() << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<","<<
    //static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<","<<
    //static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14]);
    deltaX_mm = fpDeltaMatPtr[12]; //dX in mm
    deltaY_mm = fpDeltaMatPtr[13]; //dY in mm
    deltaZ_mm = fpDeltaMatPtr[14]; //dZ in mm
    if (deltaX_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MAX";
    if (deltaX_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MIN";
    if (deltaY_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MAX";
    if (deltaY_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MIN";
    if (deltaZ_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MAX";
    if (deltaZ_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MIN";


    if (deltaXvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MAX";
    if (deltaXvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MIN";
    if (deltaYvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MAX";
    if (deltaYvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MIN";
    if (deltaZvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MAX";
    if (deltaZvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MIN";
    /*qDebug()<< "deltaX_mm:"<<deltaX_mm <<"deltaY_mm:"<<deltaY_mm<<"deltaY_mm:"<<deltaZ_mm<<
    "deltaXvec_deg:"<<deltaXvec_deg<<"deltaYvec_deg:"<<deltaYvec_deg<<"deltaZvec_deg:"<<deltaZvec_deg ;*/

    QString NewMatrixFilePath = QDir::currentPath().append(TransformedMatrixFileName);
    QFile outCamFile(NewMatrixFilePath);
    outCamFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outCamFile);
    outData.setRealNumberPrecision(10);
    float *fpDeltaMat = inputMatrix.data();
    outData << fixed << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<","<<
                                                                                                                                                                                        static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                        static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14])<<","<<
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         static_cast<double>(fpDeltaMat[3])<<","<<static_cast<double>(fpDeltaMat[7])<<","<<static_cast<double>(fpDeltaMat[11])<<","<<static_cast<double>(fpDeltaMat[15])<<'\n';

    outCamFile.close();
    QString getTempData;
    QStringList csplitString;

    QFile inFile(NewMatrixFilePath);
    inFile.open(QIODevice::ReadOnly | QFile::Text);
    QTextStream inData(&inFile);
    getTempData=inData.readLine();
    csplitString=getTempData.split(",");
    QMatrix4x4 deltransMat(csplitString[0].toFloat(),csplitString[1].toFloat(),csplitString[2].toFloat(),csplitString[3].toFloat(),
            csplitString[4].toFloat(),csplitString[5].toFloat(),csplitString[6].toFloat(),csplitString[7].toFloat(),
            csplitString[8].toFloat(),csplitString[9].toFloat(),csplitString[10].toFloat(),csplitString[11].toFloat(),
            0 ,   0,   0,   1);
    ui->out_r1c1->setText(csplitString[0]);
    ui->out_r1c2->setText(csplitString[1]);
    ui->out_r1c3->setText(csplitString[2]);
    ui->out_r1c4->setText(csplitString[3]);
    ui->out_r2c1->setText(csplitString[4]);
    ui->out_r2c2->setText(csplitString[5]);
    ui->out_r2c3->setText(csplitString[6]);
    ui->out_r2c4->setText(csplitString[7]);
    ui->out_r3c1->setText(csplitString[8]);
    ui->out_r3c2->setText(csplitString[9]);
    ui->out_r3c3->setText(csplitString[10]);
    ui->out_r3c4->setText(csplitString[11]);
    //Remove angle between vector as this doesnt imply the real displacement.
    /*//Delta Angle between 2 consecutive X vector's
    deltaXvec_deg = (acos(ui->out_r1c1->text().toFloat()*ui->in_r1c1->text().toFloat()+ui->out_r2c1->text().toFloat()*ui->in_r2c1->text().toFloat()+ ui->out_r3c1->text().toFloat()*ui->in_r3c1->text().toFloat()))*(Degree180/PiValue);
    //Delta Angle between 2 consecutive Y vector's
    deltaYvec_deg = (acos(ui->out_r1c2->text().toFloat()*ui->in_r1c2->text().toFloat()+ui->out_r2c2->text().toFloat()*ui->in_r2c2->text().toFloat()+ ui->out_r3c2->text().toFloat()*ui->in_r3c2->text().toFloat()))*(Degree180/PiValue);
    //Delta Angle between 2 consecutive Z vector's
    deltaZvec_deg = (acos(ui->out_r1c3->text().toFloat()*ui->in_r1c3->text().toFloat()+ui->out_r2c3->text().toFloat()*ui->in_r2c3->text().toFloat()+ ui->out_r3c3->text().toFloat()*ui->in_r3c3->text().toFloat()))*(Degree180/PiValue);
    QString errorLog(QString ("dX:%0,dY:%1,dZ:%2,dXvec:%3,dYvec:%4,dZvec:%5").arg(deltaX_mm).arg(deltaY_mm).arg(deltaZ_mm).
                     arg(deltaXvec_deg).arg(deltaYvec_deg).arg(deltaZvec_deg));
    qDebug()<<"DELTA-VALUES:"<<fixed<<errorLog;
    if (deltaX_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MAX";
    if (deltaX_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MIN";
    if (deltaY_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MAX";
    if (deltaY_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MIN";
    if (deltaZ_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MAX";
    if (deltaZ_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MIN";


    if (deltaXvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MAX";
    if (deltaXvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MIN";
    if (deltaYvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MAX";
    if (deltaYvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MIN";
    if (deltaZvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MAX";
    if (deltaZvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MIN";
    if ( (deltaX_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaX_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaY_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaY_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaZ_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaZ_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaXvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaXvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaYvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaYvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaZvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaZvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  )

    {
        qDebug("Calculated Del value Exceed Min/Max Limit Addition to Database-1.");

        QMessageBox::information(this,"Delta Min-Max Error-1",errorLog);
    }*/
    m_rotMatCurrent3x3 = deltaTranformMatrix.normalMatrix();
    //qDebug()<<"RotMat_Expected_normal:"<<m_rotMatCurrent3x3;

    QQuaternion expectedQuaternion = QQuaternion::fromRotationMatrix(m_rotMatCurrent3x3);
    float x, y,z,ang;
    expectedQuaternion.getAxisAndAngle(&x,&y,&z,&ang);
    //ang *= 3.14159f/180;
    deltaXvec_deg = x*ang;
    deltaYvec_deg = y*ang;
    deltaZvec_deg = z*ang;
    qDebug()<<"DELTA-LINEAR-VALUES:"<<fixed<<"dX="<<deltaX_mm<<"mm;dY="<<deltaY_mm<<"mm;dZ="<<deltaZ_mm<<"mm.";
    qDebug()<<"DELTA-ANGULAR-VALUES:"<<fixed<<"rX="<<deltaXvec_deg<<"deg;rY="<<deltaYvec_deg<<"deg;rZ="<<deltaZvec_deg<<"deg";
    if (deltaX_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MAX";
    if (deltaX_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delX MIN";
    if (deltaY_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MAX";
    if (deltaY_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delY MIN";
    if (deltaZ_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MAX";
    if (deltaZ_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delZ MIN";
    QString errorLog1(QString ("dX:%0,dY:%1,dZ:%2,dXvec:%3,dYvec:%4,dZvec:%5").arg(deltaX_mm).arg(deltaY_mm).arg(deltaZ_mm).
                      arg(deltaXvec_deg).arg(deltaYvec_deg).arg(deltaZvec_deg));

    if (deltaXvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MAX";
    if (deltaXvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRX MIN";
    if (deltaYvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MAX";
    if (deltaYvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRY MIN";
    if (deltaZvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MAX";
    if (deltaZvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM) qDebug()<< "Failed delRZ MIN";
    if ( (deltaX_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaX_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaY_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaY_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaZ_mm>=LINEAR_AXIS_MAX_THRESHOLD_VALUE_CAM) || (deltaZ_mm<=LINEAR_AXIS_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaXvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaXvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaYvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaYvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  ||
         (deltaZvec_deg>=ANGULAR_MAX_THRESHOLD_VALUE_CAM) || (deltaZvec_deg<=ANGULAR_MIN_THRESHOLD_VALUE_CAM)  )

    {
        qDebug("Calculated Del value Exceed Min/Max Limit Addition to Database-2.");

        QMessageBox::information(this,"Delta Min-Max Error-2",errorLog1);
    }else {
        qDebug("Calculated Del within Min/Max Limit Addition to Database-2.");
    }
}

void MainWindow::on_createShortDeltaTransform_clicked()
{
    if(!m_CamfilePath.isEmpty())
    {
        updateURTCP();
        ui->filePath->setText(m_CamfilePath);
        createShortDeltaTransform(m_CamfilePath);
    }
}
int16_t MainWindow::createShortDeltaTransform(QString p_fileNameWithPath)
{
    memset(&m_OldcamData,0,sizeof(m_OldcamData));
    memset(&m_NewcamData,0,sizeof(m_NewcamData));
    QString camFilePath = QDir::currentPath().append(ShortDeltaMatrixFileName);
    QFile outCamFile(camFilePath);
    ui->shortDeltaTranform->setText(camFilePath);
    outCamFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outCamFile);

    QFile inputCamFile(p_fileNameWithPath);
    inputCamFile.open(QIODevice::ReadOnly|QFile::Text);
    QTextStream inData(&inputCamFile);
    QString getTempData=inData.readLine();
    qDebug()<<"getTempData:ReadLine:"<<getTempData;
    int validmovementcount = 0,invalidmovementcount=0,linecount=0;
    float thresholdValue = 0;
    thresholdValue = ui->filterThreshold->text().toFloat();
    if( getTempData.compare("Camera Timestamp (ms);r1c1;r1c2;r1c3;r1c4;r2c1;r2c2;r2c3;r2c4;r3c1;r3c2;r3c3;r3c4",Qt::CaseSensitive) != 0)
    {
        qDebug()<<"FileManager:INVALID CAMERA FILE FORMAT DETECTED..";
        return FAILURE;
    }
    else
    {
        qDebug()<<"FileManager:CAMERA FILE FORMAT OK..";
    }
    //read the second line.
    if(!inData.atEnd())
    {
        linecount++;
        getTempData=inData.readLine();
        dataSeparation(getTempData,m_OldcamData);
    }
    outData.setRealNumberPrecision(10);
    getTempData.clear();
    while (!inData.atEnd())
    {
        linecount++;
        bool foundValidDisplacement = false;
        getTempData.clear();
        getTempData=inData.readLine();
        dataSeparation(getTempData,m_NewcamData);
        /*qDebug()<<"m_NewcamData "<<m_NewcamData.timeStamp_ms<<m_NewcamData.row1Col1<<m_NewcamData.row1Col2<<m_NewcamData.row1Col3
               <<m_NewcamData.row1Col4<<m_NewcamData.row2Col1<<m_NewcamData.row2Col2<<m_NewcamData.row2Col3<<m_NewcamData.row2Col4
              <<m_NewcamData.row3Col1<<m_NewcamData.row3Col2<<m_NewcamData.row3Col3<<m_NewcamData.row3Col4;*/
        QMatrix4x4 NewtransMat(m_NewcamData.row1Col1,m_NewcamData.row1Col2,m_NewcamData.row1Col3,m_NewcamData.row1Col4,
                               m_NewcamData.row2Col1,m_NewcamData.row2Col2,m_NewcamData.row2Col3,m_NewcamData.row2Col4,
                               m_NewcamData.row3Col1,m_NewcamData.row3Col2,m_NewcamData.row3Col3,m_NewcamData.row3Col4,
                               0 ,   0,   0,   1);
        while((!foundValidDisplacement)&&(!inData.atEnd()))
        {
            /*qDebug()<<"previousCamData "<<m_OldcamData.timeStamp_ms<<m_OldcamData.row1Col1<<m_OldcamData.row1Col2
               <<m_OldcamData.row1Col3<<m_OldcamData.row1Col4<<m_OldcamData.row2Col1<<m_OldcamData.row2Col2
              <<m_OldcamData.row2Col3<<m_OldcamData.row2Col4<<m_OldcamData.row3Col1<<m_OldcamData.row3Col2
             <<m_OldcamData.row3Col3<<m_OldcamData.row3Col4;*/
            QMatrix4x4 OldtransMat(m_OldcamData.row1Col1,m_OldcamData.row1Col2,m_OldcamData.row1Col3,m_OldcamData.row1Col4,
                                   m_OldcamData.row2Col1,m_OldcamData.row2Col2,m_OldcamData.row2Col3,m_OldcamData.row2Col4,
                                   m_OldcamData.row3Col1,m_OldcamData.row3Col2,m_OldcamData.row3Col3,m_OldcamData.row3Col4,
                                   0 ,   0,   0,   1);
            QMatrix4x4 deltransMat = OldtransMat.inverted() * NewtransMat;
            // qDebug()<<"DELTA-MATRIX-VALUES:"<<deltransMat;
            float *fpDeltaMat = deltransMat.data();
            //qDebug()<<"DELTA-LINEAR-VALUES:"<<fixed<<"dX="<<fpDeltaMat[12]<<"mm;dY="<<fpDeltaMat[13]<<"mm;dZ="<<fpDeltaMat[14]<<"mm.";

            if((fpDeltaMat[12] > thresholdValue)||(fpDeltaMat[13] > thresholdValue)||(fpDeltaMat[14] > thresholdValue))
            {
                validmovementcount++;
                foundValidDisplacement = true;
                fpDeltaMat[12]/=1000; //Convert the displacement in X from mm to m
                fpDeltaMat[13]/=1000; //Convert the displacement in Y from mm to m
                fpDeltaMat[14]/=1000; //Convert the displacement in Z from mm to m
                outData << fixed << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<","<<
                                                                                                                                                                                                    static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<","<<
                                                                                                                                                                                                                                                                                                                                                                    static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14])<<","<<
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     static_cast<double>(fpDeltaMat[3])<<","<<static_cast<double>(fpDeltaMat[7])<<","<<static_cast<double>(fpDeltaMat[11])<<","<<static_cast<double>(fpDeltaMat[15])<<'\n';
            }else
            {
                invalidmovementcount++;
                linecount++;
                getTempData.clear();
                getTempData=inData.readLine();
                dataSeparation(getTempData,m_OldcamData);
            }
        }
        memcpy(&m_OldcamData,&m_NewcamData,sizeof(m_NewcamData));
        memset(&m_NewcamData,0,sizeof(m_NewcamData));
    }
    outCamFile.close();
    inputCamFile.close();
    getTempData.clear();
    qDebug()<<"LINE COUNT VALUES = " << linecount;
    qDebug()<<"VALID XYZ VALUES = " << validmovementcount;
    qDebug()<<"INVALID XYZ VALUES = " << invalidmovementcount;
    QMessageBox::information(this,"UR Script Utility","Delta Transformation Matrix computation done!!");
    return SUCCESS;
}

void MainWindow::on_createStripDeltaTransform_clicked()
{
    QString deltamatrixfilePath =  ui->deltaOutputFile->text();
    QFileInfo fileInfo;
    fileInfo.setFile(deltamatrixfilePath);
    QString fileName = fileInfo.fileName();
    fileName.chop(4);
    QString strippedDeltaMatFilePath = QDir::currentPath().append(StrippedDeltaMatrixFileName);
    strippedDeltaMatFilePath.chop(4);
    strippedDeltaMatFilePath.append("_").append(fileName).append(".csv");
    QFile outDeltaFile(strippedDeltaMatFilePath);
    ui->stripDeltaOutput->setText(strippedDeltaMatFilePath);
    outDeltaFile.open(QIODevice::WriteOnly | QFile::Text);
    QTextStream outData(&outDeltaFile);


    QFile inputDeltaMatrixFile(deltamatrixfilePath);
    inputDeltaMatrixFile.open(QIODevice::ReadOnly|QFile::Text);
    QTextStream inData(&inputDeltaMatrixFile);
    QString getTempData=inData.readLine();

    int validmovementcount = 0,invalidmovementcount=0,linecount=0;
    float thresholdValue = 0;
    thresholdValue = ui->linearThreshold_mm->text().toFloat();
    thresholdValue /= 1000; // mm to m
    outData.setRealNumberPrecision(10);
    getTempData.clear();

    while (!inData.atEnd())
    {
        linecount++;
        getTempData.clear();
        getTempData=inData.readLine();
        QStringList csplitString=getTempData.split(",");
        QMatrix4x4 NewtransMat(csplitString[0].toFloat(),csplitString[1].toFloat(),csplitString[2].toFloat(),csplitString[3].toFloat(),
                               csplitString[4].toFloat(),csplitString[5].toFloat(),csplitString[6].toFloat(),csplitString[7].toFloat(),
                               csplitString[8].toFloat(),csplitString[9].toFloat(),csplitString[10].toFloat(),csplitString[11].toFloat(),
                               0 ,   0,   0,   1);

        float *fpDeltaMat = NewtransMat.data();
        if((std::abs(fpDeltaMat[12]) > thresholdValue)||(std::abs(fpDeltaMat[13]) > thresholdValue)||(std::abs(fpDeltaMat[14]) > thresholdValue))
        {
            validmovementcount++;
            outData << fixed << static_cast<double>(fpDeltaMat[0])<<","<<static_cast<double>(fpDeltaMat[4])<<","<<static_cast<double>(fpDeltaMat[8])<<","<<static_cast<double>(fpDeltaMat[12])<<",";
            outData << fixed << static_cast<double>(fpDeltaMat[1])<<","<<static_cast<double>(fpDeltaMat[5])<<","<<static_cast<double>(fpDeltaMat[9])<<","<<static_cast<double>(fpDeltaMat[13])<<",";
            outData << fixed << static_cast<double>(fpDeltaMat[2])<<","<<static_cast<double>(fpDeltaMat[6])<<","<<static_cast<double>(fpDeltaMat[10])<<","<<static_cast<double>(fpDeltaMat[14])<<",";
            outData << fixed << static_cast<double>(fpDeltaMat[3])<<","<<static_cast<double>(fpDeltaMat[7])<<","<<static_cast<double>(fpDeltaMat[11])<<","<<static_cast<double>(fpDeltaMat[15])<<'\n';

        }else
        {
            invalidmovementcount++;
        }
    }
    outDeltaFile.close();
    inputDeltaMatrixFile.close();
    getTempData.clear();
    qDebug()<<"******* CREATE STRIPPED DELTA MATRIX FILE METRICS ********  " ;
    qDebug()<<"LINE COUNT VALUE = " << linecount;
    qDebug()<<"VALID XYZ VALUES = " << validmovementcount;
    qDebug()<<"INVALID XYZ VALUES = " << invalidmovementcount;
    qDebug()<<"INPUT THERSHOLD (in Meters) = " << thresholdValue;
    QMessageBox::information(this,"UR Script Utility","Stripped Delta Transformation Matrix computation done!!");
}
