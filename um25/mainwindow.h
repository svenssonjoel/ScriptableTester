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

    void on_radioButton_toggled(bool checked);

    void on_testerConnectPushButton_clicked();

    void on_testerRefreshPushButton_clicked();

    void on_runScriptPushButton_clicked();

    void on_loadScriptPushButton_clicked();

    void on_saveScriptPushButton_clicked();

    void scriptTimerTimeout();
private:
    void parsePacket();
    void updateSerialPorts();
    void initPlots();

    Ui::MainWindow *ui;

    QSerialPort *mSerial;
    QSerialPort *mTesterSerial;

    QList<QSerialPortInfo> mSerialPorts;
    QByteArray mPacket; /* 130 byte packet in progress */
    bool mLogging;

    double mSample;

    QSharedPointer<QCPGraphDataContainer> mVoltData;
    QSharedPointer<QCPGraphDataContainer> mAmpData;
    QSharedPointer<QCPGraphDataContainer> mWattData;

    QTimer mScriptTimer;
    QStringList mScript;
};
#endif // MAINWINDOW_H
