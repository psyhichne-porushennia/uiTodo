#include "TaskItemWidget.h"
#include <QHBoxLayout>
#include <QAction>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QPainterPath>
#include <QTimer>
#include <QEvent>

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

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 0, 0, 0);
    lay->setSpacing(0);

    if (m_isArchived) {
        m_check->hide();
        const int w = m_check->sizeHint().width();
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

    m_menu = new QMenu(this);
    styleMenu();

    QAction* actArchive = m_isArchived
                              ? m_menu->addAction(tr("Restore from Archive"))
                              : m_menu->addAction(tr("Move to Archive"));
    auto* actDelete = m_menu->addAction(tr("Delete"));
    m_menuBtn->setMenu(m_menu);

    connect(actArchive, &QAction::triggered, this, [this]{ emit archiveRequested(m_taskId); });
    connect(actDelete,  &QAction::triggered, this, [this]{ emit deleteRequested(m_taskId);  });

    connect(m_edit, &QLineEdit::editingFinished, this, [this]{
        emit textEdited(m_taskId, m_edit->text());
    });
}

void TaskItemWidget::styleMenu() {
    m_menu->setWindowFlags(Qt::Popup
                           | Qt::FramelessWindowHint
                           | Qt::NoDropShadowWindowHint);
    m_menu->setAttribute(Qt::WA_TranslucentBackground, true);

    m_menu->setStyleSheet(R"(
        QMenu { background:#ffffff; border:1px solid #e7e7ea; border-radius:10px; padding:6px; }
        QMenu::item { background:transparent; padding:8px 14px; border-radius:6px; color:black; }
        QMenu::item:selected { background:#f2f2f6; color:black; }
        QMenu::separator { height:1px; background:#e7e7ea; margin:6px 10px; }
        QMenu:focus { outline:none; }
    )");

    auto *fx = new QGraphicsDropShadowEffect(m_menu);
    fx->setBlurRadius(16);
    fx->setOffset(0, 3);
    fx->setColor(QColor(0,0,0,55));
    m_menu->setGraphicsEffect(fx);
    m_menu->installEventFilter(this);
}

void TaskItemWidget::updateMenuMask() {
    const int radius = 10;
    QRect r = m_menu->rect().adjusted(0,0,-1,-1);
    QPainterPath path; path.addRoundedRect(r, radius, radius);
    m_menu->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

bool TaskItemWidget::eventFilter(QObject* watched, QEvent* e) {
    if (watched == m_menu && (e->type() == QEvent::Show || e->type() == QEvent::Resize)) {
        QTimer::singleShot(0, this, [this]{ updateMenuMask(); });
    }
    return QWidget::eventFilter(watched, e);
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
        QToolButton { color:black; border:none; font-size:16px; padding:2px 6px; }
        QToolButton::menu-indicator { image:none; }
        QCheckBox { color:#7ae0a0; }
    )");
}
