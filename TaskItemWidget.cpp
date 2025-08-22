#include "TaskItemWidget.h"
#include <QHBoxLayout>
#include <QAction>
#include "QStyle"

TaskItemWidget::TaskItemWidget(const Task& task, QWidget* parent)
    : QWidget(parent) {
    buildUi(task);
    applyStyle();
}

void TaskItemWidget::buildUi(const Task& task) {
    m_taskId = task.getTaskID();

    m_check = new QCheckBox(this);
    m_check->setChecked(task.getStatus() == done);

    m_edit = new QLineEdit(QString::fromStdString(task.getText()), this);
    m_edit->setPlaceholderText("I need to…");
    m_edit->setReadOnly(true);
    m_edit->setFocusPolicy(Qt::NoFocus);
    m_edit->setCursor(Qt::ArrowCursor);
    m_edit->setContextMenuPolicy(Qt::NoContextMenu);

    m_menuBtn = new QToolButton(this);
    m_menuBtn->setIcon(QIcon(":/icons/menu.svg"));
    m_menuBtn->setCursor(Qt::PointingHandCursor);
    m_menuBtn->setPopupMode(QToolButton::InstantPopup);

    m_isArchived = (task.getStatus() == archived);

    // ЛЕЙАУТ спочатку
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 0, 0, 0);
    lay->setSpacing(0);

    // Для архіву: замість чекбокса — фіксований спейсер ТОЇ Ж ширини
    if (m_isArchived) {
        m_check->hide();
        const int w = m_check->sizeHint().width(); // точна ширина чекбокса в цій темі
        m_cbSpacer = new QSpacerItem(w, 1, QSizePolicy::Fixed, QSizePolicy::Minimum);
        lay->addItem(m_cbSpacer);
    } else {
        lay->addWidget(m_check);
        connect(m_check, &QCheckBox::toggled, this, [this](bool on){
            emit toggledDone(m_taskId, on);
        });
    }

    lay->addWidget(m_edit, 1);
    lay->addWidget(m_menuBtn);

    // Меню
    m_menu = new QMenu(this);
    QAction* actArchive = m_isArchived
                              ? m_menu->addAction("Restore from Archive")
                              : m_menu->addAction("Move to Archive");
    auto* actDelete = m_menu->addAction("Delete");
    m_menuBtn->setMenu(m_menu);

    connect(actArchive, &QAction::triggered, this, [this]{ emit archiveRequested(m_taskId); });
    connect(actDelete,  &QAction::triggered, this, [this]{ emit deleteRequested(m_taskId);  });

    // (сигнал editingFinished можна лишити — readOnly не дасть редагувати)
    connect(m_edit, &QLineEdit::editingFinished, this, [this]{
        emit textEdited(m_taskId, m_edit->text());
    });
}


void TaskItemWidget::syncFrom(const Task& t) {
    if (t.getTaskID() != m_taskId) return;
    m_check->setChecked(t.getStatus() == done);
    if (m_edit->text() != QString::fromStdString(t.getText()))
        m_edit->setText(QString::fromStdString(t.getText()));
}

void TaskItemWidget::applyStyle() {
    setObjectName("TaskItem");
    setStyleSheet(R"(
    #TaskItem {
        background:transparent;
        border:1px solid #e7e7ea;
        border-radius:12px;
    }
    QLineEdit {
        color:black;
        border:none; padding:4px 6px; background:transparent;
        selection-background-color:#dfe8ff; selection-color:black;
    }
    QToolButton {
        color:black;
        border:none; font-size:16px; padding:2px 6px;
    }
    QToolButton::menu-indicator { image:none; }
    QCheckBox { color:#7ae0a0; }
    )");
}

