#include "mainwindow.h"
#include <QSplitter>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>
#include "themes/thememanager.h"
#include "tools/bertestdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_serial(new QSerialPort(this))
    , m_autoSendTimer(new QTimer(this))
    , m_speedTimer(new QTimer(this))
{
    // 初始化主题系统
    ThemeManager::instance()->refresh();

    setupUI();
    setupMenuBar();

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::onSerialReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::onSerialError);

    // 设置默认协议模板
    auto tmpl = ProtocolConfigWidget::customFrameTemplate();
    m_parser.setTemplate(tmpl);
    m_sendWidget->protocolConfig()->setTemplate(tmpl);

    connect(m_sendWidget->protocolConfig(), &ProtocolConfigWidget::templateChanged, [this]() {
        m_parser.setTemplate(m_sendWidget->protocolConfig()->getTemplate());
    });

    // 速度刷新定时器（每秒更新）
    connect(m_speedTimer, &QTimer::timeout, this, &MainWindow::updateSpeedStats);
    m_speedTimer->start(1000);

    updateStatus();
}

MainWindow::~MainWindow()
{
    if (m_serial->isOpen()) m_serial->close();
}

// 格式化字节数
static QString formatBytes(qint64 bytes)
{
    if (bytes < 1024) return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
}

// 格式化速度
static QString formatSpeed(qint64 bytesPerSec)
{
    if (bytesPerSec < 1024) return QString("%1 B/s").arg(bytesPerSec);
    if (bytesPerSec < 1024 * 1024) return QString("%1 KB/s").arg(bytesPerSec / 1024.0, 0, 'f', 1);
    return QString("%1 MB/s").arg(bytesPerSec / (1024.0 * 1024.0), 0, 'f', 2);
}

void MainWindow::setupUI()
{
    auto *central = new QWidget;
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);

    // ===== 上半部分：左（串口配置）右（发送） =====
    auto *topSplitter = new QSplitter(Qt::Horizontal);

    // 左上：串口配置（一列紧凑，带边框）
    auto *configGroup = new QGroupBox("串口配置");
    auto *configLayout = new QVBoxLayout(configGroup);
    configLayout->setContentsMargins(4, 8, 4, 4);
    m_configWidget = new SerialConfigWidget;
    configLayout->addWidget(m_configWidget);
    topSplitter->addWidget(configGroup);

    // 右上：发送（带边框）
    auto *sendGroup = new QGroupBox("发送");
    auto *sendLayout = new QVBoxLayout(sendGroup);
    sendLayout->setContentsMargins(4, 8, 4, 4);
    m_sendWidget = new SendWidget;
    sendLayout->addWidget(m_sendWidget);
    topSplitter->addWidget(sendGroup);

    // 串口配置窄，发送宽
    topSplitter->setStretchFactor(0, 1);
    topSplitter->setStretchFactor(1, 5);

    // ===== 下半部分：接收（展开） =====
    auto *recvGroup = new QGroupBox("接收");
    auto *recvLayout = new QVBoxLayout(recvGroup);
    recvLayout->setContentsMargins(4, 8, 4, 4);
    m_receiveWidget = new ReceiveWidget;
    recvLayout->addWidget(m_receiveWidget);

    // ===== 主布局：上下分栏 =====
    auto *vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(topSplitter);
    vSplitter->addWidget(recvGroup);
    vSplitter->setStretchFactor(0, 1);
    vSplitter->setStretchFactor(1, 3);

    mainLayout->addWidget(vSplitter);

    connect(m_configWidget, &SerialConfigWidget::connectClicked, this, &MainWindow::openSerialPort);
    connect(m_configWidget, &SerialConfigWidget::disconnectClicked, this, &MainWindow::closeSerialPort);
    connect(m_sendWidget, &SendWidget::sendRequested, this, &MainWindow::sendNormalData);
    connect(m_sendWidget, &SendWidget::protocolSendRequested, this, &MainWindow::sendProtocolData);
    connect(m_sendWidget, &SendWidget::autoSendToggled, this, &MainWindow::onAutoSendToggled);

    // 自动发送定时器
    connect(m_autoSendTimer, &QTimer::timeout, this, &MainWindow::sendNormalData);

    // ===== 状态栏 =====
    m_statusLabel = new QLabel("未连接");
    statusBar()->addWidget(m_statusLabel, 1);

    // 收发统计 + 速度
    m_speedLabel = new QLabel("收: 0 B | 发: 0 B");
    m_speedLabel->setMinimumWidth(300);
    statusBar()->addPermanentWidget(m_speedLabel);

    setWindowTitle("串口助手 v1.0");
    resize(1100, 700);
}

void MainWindow::setupMenuBar()
{
    auto *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction("退出", qApp, &QApplication::quit);

    auto *toolMenu = menuBar()->addMenu("工具(&T)");
    toolMenu->addAction("⟳ 刷新串口列表", m_configWidget, &SerialConfigWidget::refreshPorts);
    toolMenu->addSeparator();

    // 主题子菜单
    auto *themeMenu = toolMenu->addMenu("🎨 主题切换");
    struct ThemeInfo { QString key; QString display; };
    QList<ThemeInfo> themes = {
        {"darkblue", "深邃蓝调"}, {"moonlight", "月白极光"},
    };
    for (const auto &t : themes) {
        themeMenu->addAction(t.display, [t]() {
            ThemeManager::instance()->applyTheme(t.key);
        });
    }
    themeMenu->addSeparator();
    themeMenu->addAction("🌙☀️ 日间/夜间切换", []() {
        ThemeManager::instance()->toggleDayNight();
    });

    toolMenu->addSeparator();
    toolMenu->addAction("📊 误码率测试", [this]() {
        if (!m_berDialog) {
            m_berDialog = new BERTestDialog(m_serial, this);
            connect(m_berDialog, &QDialog::finished, this, [this]() {
                m_berDialog = nullptr;
            });
            connect(m_berDialog, &BERTestDialog::testStarted, this, [this]() {
                m_sendWidget->setEnabled(false);
                m_autoSendTimer->stop();
            });
            connect(m_berDialog, &BERTestDialog::testStopped, this, [this]() {
                m_sendWidget->setEnabled(true);
            });
        }
        m_berDialog->show();
        m_berDialog->raise();
        m_berDialog->activateWindow();
    });

    auto *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction("关于", [this]() {
        QMessageBox::about(this, "关于",
            "串口助手 v1.0\n\n"
            "功能特性:\n"
            "• 支持常用串口参数配置\n"
            "• HEX/ASCII 双模式收发\n"
            "• 自定义协议帧解析\n"
            "• 定时发送\n"
            "• 接收数据保存\n"
            "• 实时收发统计与速度\n\n"
            "基于 Qt + QSerialPort 开发");
    });
}

void MainWindow::openSerialPort()
{
    m_serial->setPortName(m_configWidget->portName());
    m_serial->setBaudRate(m_configWidget->baudRate());
    m_serial->setDataBits(m_configWidget->dataBits());
    m_serial->setStopBits(m_configWidget->stopBits());
    m_serial->setParity(m_configWidget->parity());
    m_serial->setFlowControl(m_configWidget->flowControl());

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_configWidget->setConnected(true);

        // 重置统计
        m_rxTotal = 0;
        m_txTotal = 0;
        m_rxLastSec = 0;
        m_txLastSec = 0;
        m_rxThisSec = 0;
        m_txThisSec = 0;

        updateStatus();
        updateSpeedStats();

        // 如果定时发送已勾选但串口之前未打开，现在启动
        if (m_autoSendPending) {
            m_autoSendPending = false;
            m_autoSendTimer->start(m_sendWidget->autoSendInterval());
        }
    } else {
        QMessageBox::warning(this, "错误", "无法打开串口: " + m_serial->errorString());
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
    m_configWidget->setConnected(false);
    m_autoSendTimer->stop();
    updateStatus();
}

void MainWindow::onSerialReadyRead()
{
    QByteArray data = m_serial->readAll();
    if (data.isEmpty()) return;

    // 更新统计
    m_rxTotal += data.size();
    m_rxThisSec += data.size();

    // 转发给误码率测试窗口
    if (m_berDialog && m_berDialog->isRunning()) {
        m_berDialog->feedReceivedData(data);
    }

    // 尝试协议解析
    auto frames = m_parser.decode(data);
    if (!frames.isEmpty()) {
        for (const auto &frame : frames) {
            m_receiveWidget->appendProtocolFrame(frame, true);
        }
    } else {
        // 原始数据显示
        m_receiveWidget->appendData(data, true);
    }
}

void MainWindow::onSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, "串口错误", m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::sendNormalData()
{
    if (!m_serial->isOpen()) {
        QMessageBox::warning(this, "提示", "请先打开串口");
        return;
    }

    QByteArray data = m_sendWidget->sendData();
    if (data.isEmpty()) return;

    m_serial->write(data);

    // 更新统计
    m_txTotal += data.size();
    m_txThisSec += data.size();

    m_receiveWidget->appendData(data, false);
}

void MainWindow::sendProtocolData(const QByteArray &cmd, const QByteArray &payload)
{
    if (!m_serial->isOpen()) {
        QMessageBox::warning(this, "提示", "请先打开串口");
        return;
    }

    QByteArray frame = m_parser.buildFrame(cmd, payload);
    m_serial->write(frame);

    // 更新统计
    m_txTotal += frame.size();
    m_txThisSec += frame.size();

    // 尝试解析显示
    ParsedFrame pf;
    pf.templ = m_parser.templateRef();
    int offset = 0;
    for (const auto &field : pf.templ.fields) {
        pf.fieldValues.append(frame.mid(offset, field.length));
        offset += field.length;
    }
    pf.valid = true;
    m_receiveWidget->appendProtocolFrame(pf, false);
}

void MainWindow::onAutoSendToggled(bool enabled)
{
    if (enabled) {
        if (m_serial->isOpen()) {
            m_autoSendTimer->start(m_sendWidget->autoSendInterval());
        } else {
            // 标记等待，串口打开后自动启动
            m_autoSendPending = true;
        }
    } else {
        m_autoSendTimer->stop();
        m_autoSendPending = false;
    }
}

void MainWindow::updateSpeedStats()
{
    // 上一秒的值就是速度
    m_rxLastSec = m_rxThisSec;
    m_txLastSec = m_txThisSec;
    m_rxThisSec = 0;
    m_txThisSec = 0;

    m_speedLabel->setText(
        QString("收: %1 (%2) | 发: %3 (%4)")
            .arg(formatBytes(m_rxTotal))
            .arg(formatSpeed(m_rxLastSec))
            .arg(formatBytes(m_txTotal))
            .arg(formatSpeed(m_txLastSec))
    );
}

void MainWindow::updateStatus()
{
    if (m_serial->isOpen()) {
        m_statusLabel->setText(QString("已连接: %1 | %2 | %3-%4-%5")
            .arg(m_configWidget->portName())
            .arg(m_configWidget->baudRate())
            .arg(m_configWidget->dataBits())
            .arg(m_configWidget->stopBits() == QSerialPort::OneStop ? "1" : "2")
            .arg(m_configWidget->parity() == QSerialPort::NoParity ? "N" : "E"));
    } else {
        m_statusLabel->setText("未连接");
    }
}
