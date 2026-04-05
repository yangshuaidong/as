#include "sendwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "utils/hexutil.h"

SendWidget::SendWidget(QWidget *parent)
    : QWidget(parent)
    , m_protocolMgr(new ProtocolManager(this))
{
    setupUI();

    // 加载已保存的协议
    m_protocolMgr->loadFromFile(ProtocolManager::defaultSavePath());
    refreshProtocolList();
}

void SendWidget::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    m_tabs = new QTabWidget;

    // ===== Tab 1: 普通发送 =====
    auto *normalPage = new QWidget;
    auto *normalLayout = new QVBoxLayout(normalPage);

    auto *optRow = new QHBoxLayout;
    m_hexCheck = new QCheckBox("HEX 发送");
    optRow->addWidget(m_hexCheck);
    optRow->addSpacing(8);
    m_autoSendCheck = new QCheckBox("定时发送");
    optRow->addWidget(m_autoSendCheck);
    optRow->addWidget(new QLabel("间隔(ms):"));
    m_intervalSpin = new QSpinBox;
    m_intervalSpin->setRange(10, 60000);
    m_intervalSpin->setValue(1000);
    m_intervalSpin->setMinimumWidth(90);
    m_intervalSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    optRow->addWidget(m_intervalSpin);
    optRow->addStretch();
    normalLayout->addLayout(optRow);

    // 定时发送信号
    connect(m_autoSendCheck, &QCheckBox::toggled, this, &SendWidget::autoSendToggled);
    connect(m_intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int) {
        if (m_autoSendCheck->isChecked()) {
            // 重新发射信号以更新定时器间隔
            emit autoSendToggled(true);
        }
    });

    m_dataEdit = new QTextEdit;
    m_dataEdit->setPlaceholderText("输入数据...\nHEX: 01 02 FF AB\n文本直接输入");
    normalLayout->addWidget(m_dataEdit);

    m_btnSend = new QPushButton("📤 发送");
    m_btnSend->setObjectName("btnSend");
    m_btnSend->setMinimumHeight(36);
    connect(m_btnSend, &QPushButton::clicked, this, &SendWidget::onSendClicked);
    normalLayout->addWidget(m_btnSend);

    m_tabs->addTab(normalPage, "📝 普通发送");

    // ===== Tab 2: 协议发送（两栏布局） =====
    auto *protoPage = new QWidget;
    auto *protoLayout = new QVBoxLayout(protoPage);

    // 顶部：协议选择 + 管理按钮
    auto *topRow = new QHBoxLayout;
    topRow->addWidget(new QLabel("当前协议:"));
    m_protocolCombo = new QComboBox;
    m_protocolCombo->setMinimumWidth(150);
    connect(m_protocolCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SendWidget::onLoadProtocol);
    topRow->addWidget(m_protocolCombo);

    m_btnSaveProtocol = new QPushButton("💾 保存");
    m_btnSaveProtocol->setToolTip("保存当前协议配置");
    connect(m_btnSaveProtocol, &QPushButton::clicked, this, &SendWidget::onSaveProtocol);
    topRow->addWidget(m_btnSaveProtocol);

    m_btnDeleteProtocol = new QPushButton("🗑️ 删除");
    m_btnDeleteProtocol->setToolTip("删除当前协议");
    connect(m_btnDeleteProtocol, &QPushButton::clicked, this, &SendWidget::onDeleteProtocol);
    topRow->addWidget(m_btnDeleteProtocol);

    m_btnExport = new QPushButton("📤 导出XLSX");
    m_btnExport->setToolTip("导出协议模板为Excel文件");
    connect(m_btnExport, &QPushButton::clicked, this, &SendWidget::onExportXlsx);
    topRow->addWidget(m_btnExport);

    m_btnImport = new QPushButton("📥 导入XLSX");
    m_btnImport->setToolTip("从Excel文件导入协议模板");
    connect(m_btnImport, &QPushButton::clicked, this, &SendWidget::onImportXlsx);
    topRow->addWidget(m_btnImport);

    protoLayout->addLayout(topRow);

    // 协议配置 + 命令/载荷（左右两栏）
    auto *contentRow = new QHBoxLayout;

    // 左：协议字段配置
    m_protocolConfig = new ProtocolConfigWidget;
    contentRow->addWidget(m_protocolConfig, 3);

    // 右：命令 + 载荷 + 发送
    auto *cmdLayout = new QVBoxLayout;

    cmdLayout->addWidget(new QLabel("命令字节(HEX):"));
    m_cmdEdit = new QLineEdit("01");
    cmdLayout->addWidget(m_cmdEdit);

    auto *hexRow = new QHBoxLayout;
    m_protocolHexCheck = new QCheckBox("HEX 载荷");
    m_protocolHexCheck->setChecked(true);
    hexRow->addWidget(m_protocolHexCheck);
    hexRow->addStretch();
    cmdLayout->addLayout(hexRow);

    cmdLayout->addWidget(new QLabel("数据载荷:"));
    m_payloadEdit = new QTextEdit;
    m_payloadEdit->setPlaceholderText("HEX: 01 02 03\nASCII: Hello");
    m_payloadEdit->setMaximumHeight(100);
    cmdLayout->addWidget(m_payloadEdit);

    m_btnProtocolSend = new QPushButton("📦 组装发送");
    m_btnProtocolSend->setObjectName("btnProtocolSend");
    m_btnProtocolSend->setMinimumHeight(40);
    connect(m_btnProtocolSend, &QPushButton::clicked, [this]() {
        QByteArray cmd = HexUtil::fromHexString(m_cmdEdit->text());
        QByteArray payload;
        if (m_protocolHexCheck->isChecked()) {
            payload = HexUtil::fromHexString(m_payloadEdit->toPlainText());
        } else {
            payload = m_payloadEdit->toPlainText().toUtf8();
        }
        emit protocolSendRequested(cmd, payload);
    });
    cmdLayout->addWidget(m_btnProtocolSend);

    contentRow->addLayout(cmdLayout, 1);
    protoLayout->addLayout(contentRow);

    m_tabs->addTab(protoPage, "🔧 协议发送");

    mainLayout->addWidget(m_tabs);
}

QByteArray SendWidget::sendData() const
{
    QString text = m_dataEdit->toPlainText();
    if (m_hexCheck->isChecked()) {
        return HexUtil::fromHexString(text);
    }
    return text.toUtf8();
}

bool SendWidget::isHexMode() const
{
    return m_hexCheck->isChecked();
}

void SendWidget::onSendClicked()
{
    emit sendRequested();
}

void SendWidget::refreshProtocolList()
{
    m_protocolCombo->blockSignals(true);
    m_protocolCombo->clear();
    m_protocolCombo->addItems(m_protocolMgr->names());
    if (m_protocolCombo->count() == 0) {
        m_protocolCombo->addItem("（无已保存协议）");
    }
    m_protocolCombo->blockSignals(false);
}

void SendWidget::onSaveProtocol()
{
    bool ok;
    QString name = QInputDialog::getText(this, "保存协议", "协议名称:",
                                          QLineEdit::Normal, m_protocolConfig->getTemplate().name, &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    ProtocolTemplate tmpl = m_protocolConfig->getTemplate();
    tmpl.name = name.trimmed();

    // 检查是否同名，有的话更新
    int idx = m_protocolMgr->indexOf(name.trimmed());
    if (idx >= 0) {
        m_protocolMgr->update(idx, tmpl);
    } else {
        m_protocolMgr->add(tmpl);
    }

    m_protocolMgr->saveToFile(ProtocolManager::defaultSavePath());
    refreshProtocolList();
    m_protocolCombo->setCurrentText(name.trimmed());
    QMessageBox::information(this, "成功", "协议 '" + name + "' 已保存");
}

void SendWidget::onDeleteProtocol()
{
    int idx = m_protocolCombo->currentIndex();
    if (idx < 0 || idx >= m_protocolMgr->count()) return;

    QString name = m_protocolMgr->at(idx).name;
    auto ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除协议 '%1' 吗？").arg(name));
    if (ret != QMessageBox::Yes) return;

    m_protocolMgr->remove(idx);
    m_protocolMgr->saveToFile(ProtocolManager::defaultSavePath());
    refreshProtocolList();
}

void SendWidget::onLoadProtocol(int index)
{
    if (index < 0 || index >= m_protocolMgr->count()) return;
    ProtocolTemplate tmpl = m_protocolMgr->at(index);
    m_protocolConfig->setTemplate(tmpl);
}

void SendWidget::onExportXlsx()
{
    // 导出当前配置的协议
    ProtocolTemplate tmpl = m_protocolConfig->getTemplate();

    QString path = QFileDialog::getSaveFileName(this, "导出协议模板",
        tmpl.name + ".xlsx", "Excel 文件 (*.xlsx)");
    if (path.isEmpty()) return;

    // 临时添加到 manager 来导出
    int tempIdx = m_protocolMgr->count();
    m_protocolMgr->add(tmpl);
    bool ok = m_protocolMgr->exportToXlsx(path, tempIdx);
    m_protocolMgr->remove(tempIdx);

    if (ok) {
        QMessageBox::information(this, "成功", "协议模板已导出到:\n" + path);
    } else {
        QMessageBox::warning(this, "失败", "导出失败，请检查文件路径和权限");
    }
}

void SendWidget::onImportXlsx()
{
    QString path = QFileDialog::getOpenFileName(this, "导入协议模板",
        "", "Excel 文件 (*.xlsx)");
    if (path.isEmpty()) return;

    bool ok;
    ProtocolTemplate tmpl = m_protocolMgr->importFromXlsx(path, &ok);

    if (!ok || tmpl.fields.isEmpty()) {
        QMessageBox::warning(this, "失败", "导入失败或文件中没有有效字段");
        return;
    }

    // 应用到配置界面
    m_protocolConfig->setTemplate(tmpl);

    QMessageBox::information(this, "成功",
        QString("已导入协议 '%1'，共 %2 个字段\n点击 [💾 保存] 可持久化保存")
            .arg(tmpl.name).arg(tmpl.fields.size()));
}
