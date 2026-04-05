#include "receivewidget.h"
#include "utils/hexutil.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

ReceiveWidget::ReceiveWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ReceiveWidget::setupUI()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 控制栏
    auto *ctrlRow = new QHBoxLayout;
    m_hexCheck = new QCheckBox("HEX 显示");
    ctrlRow->addWidget(m_hexCheck);
    m_timestampCheck = new QCheckBox("时间戳");
    m_timestampCheck->setChecked(true);
    ctrlRow->addWidget(m_timestampCheck);
    m_autoScrollCheck = new QCheckBox("自动滚动");
    m_autoScrollCheck->setChecked(true);
    ctrlRow->addWidget(m_autoScrollCheck);
    m_btnClear = new QPushButton("🗑️ 清空");
    connect(m_btnClear, &QPushButton::clicked, this, &ReceiveWidget::clear);
    ctrlRow->addWidget(m_btnClear);
    m_btnSave = new QPushButton("💾 保存");
    connect(m_btnSave, &QPushButton::clicked, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "保存接收数据", "", "文本文件 (*.txt);;所有文件 (*)");
        if (!path.isEmpty()) {
            QFile f(path);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&f);
                out << m_display->toPlainText();
            }
        }
    });
    ctrlRow->addWidget(m_btnSave);
    ctrlRow->addStretch();
    m_countLabel = new QLabel("收: 0 | 发: 0");
    ctrlRow->addWidget(m_countLabel);
    layout->addLayout(ctrlRow);

    // 显示区
    m_display = new QTextEdit;
    m_display->setReadOnly(true);
    m_display->setFontFamily("Consolas");
    m_display->setFontPointSize(10);
    layout->addWidget(m_display);
}

void ReceiveWidget::appendData(const QByteArray &data, bool incoming)
{
    QString prefix = incoming ? "📥 RX ← " : "📤 TX → ";
    QString timestamp = m_timestampCheck->isChecked()
        ? "[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] "
        : "";

    QString content;
    if (m_hexCheck->isChecked()) {
        content = HexUtil::toHexString(data);
    } else {
        content = QString::fromUtf8(data);
    }

    QString color = incoming ? "#00AA00" : "#0066CC";
    m_display->append(
        QString("<span style='color:#888888;'>%1</span>"
                "<span style='color:%2;'>%3%4</span>")
            .arg(timestamp, color, prefix, content.toHtmlEscaped())
    );

    if (incoming) m_rxCount += data.size();
    else m_txCount += data.size();
    updateCount();

    if (m_autoScrollCheck->isChecked()) {
        auto cursor = m_display->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_display->setTextCursor(cursor);
    }
}

void ReceiveWidget::appendProtocolFrame(const ParsedFrame &frame, bool incoming)
{
    QString prefix = incoming ? "📥 RX ← " : "📤 TX → ";
    QString timestamp = m_timestampCheck->isChecked()
        ? "[" + QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + "] "
        : "";

    QString color = incoming ? "#00AA00" : "#0066CC";

    // 拼接完整帧 HEX
    QString hexData;
    for (int i = 0; i < frame.fieldValues.size(); ++i) {
        if (i > 0) hexData += " ";
        hexData += frame.fieldHex(i);
    }

    QString html = QString("<span style='color:#888888;'>%1</span>"
                           "<span style='color:%2;font-weight:bold;'>%3[协议] </span>"
                           "<span style='color:%2;'>%4</span>")
                       .arg(timestamp, color, prefix, hexData);

    if (!frame.valid) {
        html += QString(" <span style='color:red;'>❌ %1</span>").arg(frame.error);
    } else {
        html += " <span style='color:green;'>✅</span>";
    }

    m_display->append(html);

    int totalLen = frame.templ.totalLength();
    if (incoming) m_rxCount += totalLen;
    else m_txCount += totalLen;
    updateCount();

    if (m_autoScrollCheck->isChecked()) {
        auto cursor = m_display->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_display->setTextCursor(cursor);
    }
}

void ReceiveWidget::clear()
{
    m_display->clear();
    m_rxCount = 0;
    m_txCount = 0;
    updateCount();
}

void ReceiveWidget::updateCount()
{
    m_countLabel->setText(QString("收: %1 | 发: %2").arg(m_rxCount).arg(m_txCount));
}
