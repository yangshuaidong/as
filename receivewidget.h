#ifndef RECEIVEWIDGET_H
#define RECEIVEWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "protocol/protocolframe.h"

class ReceiveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReceiveWidget(QWidget *parent = nullptr);

    // 追加接收数据
    void appendData(const QByteArray &data, bool incoming);

    // 追加协议解析结果
    void appendProtocolFrame(const ParsedFrame &frame, bool incoming);

    // 清空
    void clear();

    bool isHexMode() const { return m_hexCheck->isChecked(); }
    bool showTimestamp() const { return m_timestampCheck->isChecked(); }
    bool autoScroll() const { return m_autoScrollCheck->isChecked(); }

private:
    void setupUI();

    QTextEdit *m_display;
    QCheckBox *m_hexCheck;
    QCheckBox *m_timestampCheck;
    QCheckBox *m_autoScrollCheck;
    QPushButton *m_btnClear;
    QPushButton *m_btnSave;
    QLabel *m_countLabel;

    int m_rxCount = 0;
    int m_txCount = 0;
    void updateCount();
};

#endif // RECEIVEWIDGET_H
