#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QVector3D>
#include <QGenericMatrix>
#include <QQuaternion>
#include <QMatrix4x4>
#include <iostream>
#include <QMessageBox>
#include <iostream>
#include <sstream>
#include <QProgressDialog>
#include <QThread>
const int16_t FAILURE = -1;
const int16_t SUCCESS = 0;
// Camera data cells
const uint8_t CAM_DATA_SEP_DB_TIME= 0;
const uint8_t CAM_DATA_ROW1COL1 =1;
const uint8_t CAM_DATA_ROW1COL2 =2;
const uint8_t CAM_DATA_ROW1COL3 =3;
const uint8_t CAM_DATA_ROW1COL4 =4;
const uint8_t CAM_DATA_ROW2COL1 =5;
const uint8_t CAM_DATA_ROW2COL2 =6;
const uint8_t CAM_DATA_ROW2COL3 =7;
const uint8_t CAM_DATA_ROW2COL4 =8;
const uint8_t CAM_DATA_ROW3COL1 =9;
const uint8_t CAM_DATA_ROW3COL2 =10;
const uint8_t CAM_DATA_ROW3COL3 =11;
const uint8_t CAM_DATA_ROW3COL4 =12;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    struct UR3eData
    {
        double ur3X_m;      // Linear displacement by UR Robot in X axis- current TCP
        double ur3Y_m;      // Linear displacement by UR Robot in Y axis- current TCP
        double ur3Z_m;      // Linear displacement by UR Robot in Z axis- current TCP
        double ur3RX_rad;   // Angular displacement by UR Robot in X axis- current TCP
        double ur3RY_rad;   // Angular displacement by UR Robot in Y axis- current TCP
        double ur3RZ_rad;   // Angular displacement by UR Robot in Z axis- current TCP        
    };
    struct CameraData
    {
    int timeStamp_ms; //Timestamp of data captured in camera

    float row1Col1;  // Row 1 Column 1 data of camera data
    float row1Col2;  // Row 1 Column 2 data of camera data
    float row1Col3;  // Row 1 Column 3 data of camera data
    float row1Col4;  // Row 1 Column 4 data of camera data

    float row2Col1;  // Row 2 Column 1 data of camera data
    float row2Col2;  // Row 2 Column 2 data of camera data
    float row2Col3;  // Row 2 Column 3 data of camera data
    float row2Col4;  // Row 2 Column 4 data of camera data

    float row3Col1;  // Row 3 Column 1 data of camera data
    float row3Col2;  // Row 3 Column 2 data of camera data
    float row3Col3;  // Row 3 Column 3 data of camera data
    float row3Col4;  // Row 3 Column 4 data of camera data
    };
    const QString URScriptFileName = "/UR_CAM2DBSCRIPT_OUTPUT.txt";
    const QString ExpectedTFFileName = "/EXPECTED_TF_MATRIX_OUTPUT.txt";
    const QString DeltaMatrixFileName = "/CAM_DELTA_MATRIX_OUTPUT.csv";
private slots:
    void on_selectButton_clicked();

    void on_computeButton_clicked();

    void on_genScriptBtn_clicked();

    void on_testButton_clicked();

    void on_computeExpTF_clicked();

    void on_resetTCPButton_clicked();

    void on_getTMButton_clicked();

    void on_getRVButton_clicked();

    void on_splitcamfile_clicked();

private:
    Ui::MainWindow *ui;
    QMatrix4x4   m_CurrentTransform;
    QMatrix4x4 m_FirstTFInCamCSV;
    QMatrix3x3 m_rotMatCurrent3x3;
    QString m_CamfilePath;
    UR3eData m_CurrentTCP;
    struct CameraData m_OldcamData,m_NewcamData;
    void getNewCameraFile(QString &filePath);
    QMatrix4x4 getTranformationMatrixFromRotationVector(UR3eData p_ur3Data);
    int16_t convertCamerLogToDBFile(QString p_fileNameWithPath);
    int16_t dataSeparation(QString& p_data,CameraData& p_camData);
    uint16_t getURDataAfterTransformation(UR3eData &p_ur3Data);
    void updateURTCP();
};

#endif // MAINWINDOW_H
