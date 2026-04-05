#ifndef SENDWIDGET_H
#define SENDWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTabWidget>
#include "protocol/protocolconfigwidget.h"
#include "protocol/protocolmanager.h"

class SendWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SendWidget(QWidget *parent = nullptr);

    QByteArray sendData() const;
    bool isHexMode() const;
    int autoSendInterval() const { return m_intervalSpin->value(); }

    ProtocolConfigWidget *protocolConfig() const { return m_protocolConfig; }
    ProtocolManager *protocolManager() const { return m_protocolMgr; }

signals:
    void sendRequested();
    void protocolSendRequested(const QByteArray &cmd, const QByteArray &payload);
    void autoSendToggled(bool enabled);

private slots:
    void onSendClicked();
    void onSaveProtocol();
    void onDeleteProtocol();
    void onLoadProtocol(int index);
    void onExportXlsx();
    void onImportXlsx();

private:
    void setupUI();
    void refreshProtocolList();

    // Tab 切换
    QTabWidget *m_tabs;

    // 左栏：普通发送
    QTextEdit *m_dataEdit;
    QCheckBox *m_hexCheck;
    QPushButton *m_btnSend;
    QCheckBox *m_autoSendCheck;
    QSpinBox *m_intervalSpin;

    // 右栏：协议发送
    QLineEdit *m_cmdEdit;
    QTextEdit *m_payloadEdit;
    QCheckBox *m_protocolHexCheck;
    QPushButton *m_btnProtocolSend;
    ProtocolConfigWidget *m_protocolConfig;

    // 协议管理
    ProtocolManager *m_protocolMgr;
    QComboBox *m_protocolCombo;
    QPushButton *m_btnSaveProtocol;
    QPushButton *m_btnDeleteProtocol;
    QPushButton *m_btnExport;
    QPushButton *m_btnImport;
};

#endif // SENDWIDGET_H
