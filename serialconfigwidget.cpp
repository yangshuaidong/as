#include "serialconfigwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSerialPortInfo>

// 给 QComboBox 添加选中对号功能
static void addComboWithCheckmark(QComboBox *combo, const QStringList &items, const QString &current = QString())
{
    combo->blockSignals(true);
    combo->clear();
    for (const auto &item : items) {
        combo->addItem("  " + item, item);  // displayText带前缀空格，data存原始值
    }
    // 标记选中项
    int curIdx = 0;
    if (!current.isEmpty()) {
        for (int i = 0; i < combo->count(); ++i) {
            if (combo->itemData(i).toString() == current) { curIdx = i; break; }
        }
    }
    combo->setItemText(curIdx, "✓ " + combo->itemData(curIdx).toString());
    combo->setCurrentIndex(curIdx);
    combo->blockSignals(false);
}

// 更新对号位置
static void updateCheckmark(QComboBox *combo, int newIndex)
{
    for (int i = 0; i < combo->count(); ++i) {
        QString val = combo->itemData(i).toString();
        combo->setItemText(i, (i == newIndex ? "✓ " : "  ") + val);
    }
}

SerialConfigWidget::SerialConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SerialConfigWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    // 串口 + 刷新按钮（一行）
    auto *portRow = new QHBoxLayout;
    portRow->addWidget(new QLabel("串口:"));
    m_portCombo = new QComboBox;
    m_portCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    portRow->addWidget(m_portCombo);
    m_btnRefresh = new QPushButton("⟳");
    m_btnRefresh->setFixedSize(26, 26);
    m_btnRefresh->setToolTip("刷新串口列表");
    connect(m_btnRefresh, &QPushButton::clicked, this, &SerialConfigWidget::refreshPorts);
    portRow->addWidget(m_btnRefresh);
    layout->addLayout(portRow);

    // 波特率
    auto *baudRow = new QHBoxLayout;
    baudRow->addWidget(new QLabel("波特率:"));
    m_baudCombo = new QComboBox;
    m_baudCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addComboWithCheckmark(m_baudCombo, {"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"}, "115200");
    baudRow->addWidget(m_baudCombo);
    layout->addLayout(baudRow);

    // 数据位
    auto *dataRow = new QHBoxLayout;
    dataRow->addWidget(new QLabel("数据位:"));
    m_dataBitsCombo = new QComboBox;
    m_dataBitsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addComboWithCheckmark(m_dataBitsCombo, {"5", "6", "7", "8"}, "8");
    dataRow->addWidget(m_dataBitsCombo);
    layout->addLayout(dataRow);

    // 停止位
    auto *stopRow = new QHBoxLayout;
    stopRow->addWidget(new QLabel("停止位:"));
    m_stopBitsCombo = new QComboBox;
    m_stopBitsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addComboWithCheckmark(m_stopBitsCombo, {"1", "1.5", "2"}, "1");
    stopRow->addWidget(m_stopBitsCombo);
    layout->addLayout(stopRow);

    // 校验
    auto *parityRow = new QHBoxLayout;
    parityRow->addWidget(new QLabel("校验位:"));
    m_parityCombo = new QComboBox;
    m_parityCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addComboWithCheckmark(m_parityCombo, {"无", "奇校验", "偶校验"}, "无");
    parityRow->addWidget(m_parityCombo);
    layout->addLayout(parityRow);

    // 流控
    auto *flowRow = new QHBoxLayout;
    flowRow->addWidget(new QLabel("流控:"));
    m_flowCombo = new QComboBox;
    m_flowCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    addComboWithCheckmark(m_flowCombo, {"无", "硬件", "软件"}, "无");
    flowRow->addWidget(m_flowCombo);
    layout->addLayout(flowRow);

    layout->addStretch();

    // 连接按钮
    m_btnConnect = new QPushButton("🔌 打开串口");
    m_btnConnect->setMinimumHeight(32);
    connect(m_btnConnect, &QPushButton::clicked, [this]() {
        if (m_btnConnect->text().contains("打开")) emit connectClicked();
        else emit disconnectClicked();
    });
    layout->addWidget(m_btnConnect);

    // 连接对号更新
    auto connectCheckmark = [this](QComboBox *combo) {
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [combo](int i) {
            updateCheckmark(combo, i);
        });
    };
    connectCheckmark(m_baudCombo);
    connectCheckmark(m_dataBitsCombo);
    connectCheckmark(m_stopBitsCombo);
    connectCheckmark(m_parityCombo);
    connectCheckmark(m_flowCombo);

    refreshPorts();
}

void SerialConfigWidget::refreshPorts()
{
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &info : ports) {
        m_portCombo->addItem(info.portName() + " - " + info.description(), info.portName());
    }
}

QString SerialConfigWidget::portName() const
{
    return m_portCombo->currentData().toString();
}

qint32 SerialConfigWidget::baudRate() const
{
    return m_baudCombo->currentData().toString().toInt();
}

QSerialPort::DataBits SerialConfigWidget::dataBits() const
{
    return static_cast<QSerialPort::DataBits>(m_dataBitsCombo->currentData().toString().toInt());
}

QSerialPort::StopBits SerialConfigWidget::stopBits() const
{
    QString text = m_stopBitsCombo->currentData().toString();
    if (text == "1.5") return QSerialPort::OneAndHalfStop;
    if (text == "2") return QSerialPort::TwoStop;
    return QSerialPort::OneStop;
}

QSerialPort::Parity SerialConfigWidget::parity() const
{
    int idx = m_parityCombo->currentIndex();
    switch (idx) {
    case 1: return QSerialPort::OddParity;
    case 2: return QSerialPort::EvenParity;
    default: return QSerialPort::NoParity;
    }
}

QSerialPort::FlowControl SerialConfigWidget::flowControl() const
{
    int idx = m_flowCombo->currentIndex();
    switch (idx) {
    case 1: return QSerialPort::HardwareControl;
    case 2: return QSerialPort::SoftwareControl;
    default: return QSerialPort::NoFlowControl;
    }
}

void SerialConfigWidget::setConnected(bool connected)
{
    m_btnConnect->setText(connected ? "🔴 关闭串口" : "🔌 打开串口");
    m_portCombo->setEnabled(!connected);
    m_baudCombo->setEnabled(!connected);
    m_dataBitsCombo->setEnabled(!connected);
    m_stopBitsCombo->setEnabled(!connected);
    m_parityCombo->setEnabled(!connected);
    m_flowCombo->setEnabled(!connected);
    m_btnRefresh->setEnabled(!connected);
}
