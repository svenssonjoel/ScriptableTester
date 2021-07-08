#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
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

private slots:
    void serialReadyRead();

    void on_devRefreshPushButton_clicked();

    void on_devConnectPushButton_clicked();

    void on_radioButton_toggled(bool checked);

private:
    void parsePacket();
    void updateSerialPorts();
    void initPlots();

    Ui::MainWindow *ui;

    QSerialPort *mSerial;
    QList<QSerialPortInfo> mSerialPorts;
    QByteArray mPacket; /* 130 byte packet in progress */
    bool mLogging;

    double mSample;

    QSharedPointer<QCPGraphDataContainer> mVoltData;
    QSharedPointer<QCPGraphDataContainer> mAmpData;
    QSharedPointer<QCPGraphDataContainer> mWattData;


};
#endif // MAINWINDOW_H
