#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QLabel>
#include <QElapsedTimer>
#include "serialconfigwidget.h"
#include "sendwidget.h"
#include "receivewidget.h"
#include "protocol/protocolparser.h"

class BERTestDialog;  // 前向声明

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void onSerialReadyRead();
    void onSerialError(QSerialPort::SerialPortError error);

    void sendNormalData();
    void sendProtocolData(const QByteArray &cmd, const QByteArray &payload);
    void onAutoSendToggled(bool enabled);
    void updateSpeedStats();

private:
    void setupUI();
    void setupMenuBar();
    void updateStatus();

    QSerialPort *m_serial;
    SerialConfigWidget *m_configWidget;
    SendWidget *m_sendWidget;
    ReceiveWidget *m_receiveWidget;
    ProtocolParser m_parser;
    QTimer *m_autoSendTimer;
    QLabel *m_statusLabel;

    // 收发统计
    qint64 m_rxTotal = 0;
    qint64 m_txTotal = 0;
    qint64 m_rxLastSec = 0;   // 上一秒接收字节数
    qint64 m_txLastSec = 0;   // 上一秒发送字节数
    qint64 m_rxThisSec = 0;   // 本秒累计接收
    qint64 m_txThisSec = 0;   // 本秒累计发送
    QTimer *m_speedTimer;     // 速度刷新定时器
    QLabel *m_speedLabel;     // 状态栏速度显示

    // 误码率测试
    BERTestDialog *m_berDialog = nullptr;
    bool m_autoSendPending = false;  // 定时发送等待串口打开
};

#endif // MAINWINDOW_H
