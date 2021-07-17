#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct {
    uint64_t timestamp;
    double volt;
    double amp;
    double watt;
} sample_t;

typedef struct {
    uint64_t start_time;
    QVector<sample_t> samples;
} script_data_t;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

private slots:
    void serialReadyRead();

    void testerSerialReadyRead();

    void on_devRefreshPushButton_clicked();

    void on_devConnectPushButton_clicked();

    void on_testerConnectPushButton_clicked();

    void on_testerRefreshPushButton_clicked();

    void on_runScriptPushButton_clicked();

    void on_loadScriptPushButton_clicked();

    void on_saveScriptPushButton_clicked();

    void scriptTimerTimeout();

    void on_logRadioButton_toggled(bool checked);

    void on_sampleRadioButton_toggled(bool checked);

    void on_clearGraphsPushButton_clicked();

private:
    void parsePacket();
    void updateSerialPorts();
    void initPlots();
    void finishScript();

    Ui::MainWindow *ui;

    QSerialPort *mSerial;
    QSerialPort *mTesterSerial;

    QList<QSerialPortInfo> mSerialPorts;
    QByteArray mPacket; /* 130 byte packet in progress */
    bool mLogging;

    double mSample;
    bool mSampling;
    script_data_t mScriptData;

    QSharedPointer<QCPGraphDataContainer> mVoltData;
    QSharedPointer<QCPGraphDataContainer> mAmpData;
    QSharedPointer<QCPGraphDataContainer> mWattData;

    bool mScriptRunning;
    QTimer mScriptTimer;
    QStringList mScript;

};
#endif // MAINWINDOW_H
