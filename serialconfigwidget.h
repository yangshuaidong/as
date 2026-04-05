#ifndef SERIALCONFIGWIDGET_H
#define SERIALCONFIGWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QSerialPort>

class SerialConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SerialConfigWidget(QWidget *parent = nullptr);

    // 获取配置
    QString portName() const;
    qint32 baudRate() const;
    QSerialPort::DataBits dataBits() const;
    QSerialPort::StopBits stopBits() const;
    QSerialPort::Parity parity() const;
    QSerialPort::FlowControl flowControl() const;

    void setConnected(bool connected);

signals:
    void connectClicked();
    void disconnectClicked();

public slots:
    void refreshPorts();

private:
    void setupUI();

    QComboBox *m_portCombo;
    QComboBox *m_baudCombo;
    QComboBox *m_dataBitsCombo;
    QComboBox *m_stopBitsCombo;
    QComboBox *m_parityCombo;
    QComboBox *m_flowCombo;
    QPushButton *m_btnConnect;
    QPushButton *m_btnRefresh;
};

#endif // SERIALCONFIGWIDGET_H
