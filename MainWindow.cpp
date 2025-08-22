#include "MainWindow.h"
#include "TaskItemWidget.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QStatusBar>
#include <QListWidgetItem>
#include <QApplication>
#include <QStyleFactory>
#include <QScreen>
#include <QPainterPath>
#include <QRegion>
#ifdef Q_OS_WIN
#include <windows.h>
#endif


MainWindow::MainWindow(TaskManager& manager, QWidget* parent)
    : QMainWindow(parent), m_manager(manager) {
    setWindowFlags((windowFlags() | Qt::FramelessWindowHint) & ~Qt::WindowMaximizeButtonHint);
    buildUi();
    applyPalette();
    reload();
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    central->setObjectName("Root");                         // ← ДОДАНО
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    root->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // ── тонка верхня смужка (мін/клоуз) ─────────────────────────
    m_titleBar = new QFrame(central);
    m_titleBar->setObjectName("TitleBar");
    m_titleBar->setFixedHeight(24);
    auto* tLay = new QHBoxLayout(m_titleBar);
    tLay->setContentsMargins(8,0,6,0);
    tLay->setSpacing(6);

    m_btnMin   = new QToolButton(m_titleBar);
    m_btnClose = new QToolButton(m_titleBar);
    m_btnMin->setObjectName("TitleMin");
    m_btnClose->setObjectName("TitleClose");
    m_btnMin->setIcon(QIcon(":/icons/min.svg"));
    m_btnClose->setIcon(QIcon(":/icons/close.svg"));
    for (QToolButton* b : { m_btnMin, m_btnClose }) {
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        b->setFixedSize(22,22);
        b->setIconSize(QSize(14,14));
        b->setFocusPolicy(Qt::NoFocus);
        b->setCursor(Qt::PointingHandCursor);
    }
    m_btnMin->setStyleSheet("QToolButton{border:none;background:transparent;padding:0;} "
                            "QToolButton:hover{background:#eaeaea;border-radius:6px;}");
    m_btnClose->setStyleSheet("QToolButton{border:none;background:transparent;padding:0;} "
                              "QToolButton:hover{background:#e81123;color:white;border-radius:6px;}");
    tLay->addStretch();
    tLay->addWidget(m_btnMin);
    tLay->addWidget(m_btnClose);
    connect(m_btnMin,   &QToolButton::clicked, this, [this]{ showMinimized(); });
    connect(m_btnClose, &QToolButton::clicked, this, [this]{ close(); });
    root->addWidget(m_titleBar);

    // ── ХЕДЕР: окремо, фіксовано ПІД смужкою ────────────────────
    m_header = new QFrame(central);
    m_header->setObjectName("PinnedHeader");
    auto* hLay = new QHBoxLayout(m_header);
    hLay->setContentsMargins(10,10,10,10);
    hLay->setSpacing(5);
    m_header->setStyleSheet("QFrame { background: transparent; border: none; }");

    m_input = new QLineEdit(m_header);
    m_input->setPlaceholderText("+  Add a todo");
    m_addBtn = new QPushButton("Add", m_header);
    m_addBtn->setCursor(Qt::PointingHandCursor);

    hLay->addWidget(m_input, 1);
    hLay->addWidget(m_addBtn);

    root->addWidget(m_header, 0, Qt::AlignTop);   // ← прибиваємо до верху

    // ── АКТИВНІ: тільки список (без хедера всередині) ───────────
    m_activeArea = new QFrame(central);
    m_activeArea->setObjectName("ActiveArea");
    auto* actLay = new QVBoxLayout(m_activeArea);
    actLay->setContentsMargins(10,0,10,0);
    actLay->setSpacing(0);

    m_active = new QListWidget(m_activeArea);
    m_active->setObjectName("ActiveList");        // ← ДОДАНО
    m_active->setFrameShape(QFrame::NoFrame);
    m_active->setSpacing(0);
    m_active->setSelectionMode(QAbstractItemView::NoSelection);
    m_active->setFocusPolicy(Qt::NoFocus);
    m_active->setStyleSheet(
        "QListWidget{ background:transparent; border:none; }"
        "QListWidget::item:selected{ background: transparent; }"
        "QListWidget::item:hover{ background: transparent; }"
        );
    m_active->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_active->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_active->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    actLay->addWidget(m_active);
    root->addWidget(m_activeArea, 0, Qt::AlignTop);

    // ── АРХІВ ───────────────────────────────────────────────────
    m_archiveArea = new QFrame(central);
    m_archiveArea->setObjectName("ArchiveArea");
    auto* archLay = new QVBoxLayout(m_archiveArea);
    archLay->setContentsMargins(10,8,10,0);
    archLay->setSpacing(0);

    m_archived = new QListWidget(m_archiveArea);
    m_archived->setObjectName("ArchivedList");    // ← ДОДАНО
    m_archived->setFrameShape(QFrame::NoFrame);
    m_archived->setSpacing(0);
    m_archived->setSelectionMode(QAbstractItemView::NoSelection);
    m_archived->setFocusPolicy(Qt::NoFocus);
    m_archived->setStyleSheet(
        "QListWidget{ background:transparent; border:none; }"
        "QListWidget::item:selected{ background: transparent; }"
        "QListWidget::item:hover{ background: transparent; }"
        );
    m_archived->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_archived->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_archived->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* bottom = new QFrame(m_archiveArea);
    bottom->setObjectName("ArchiveBottom");
    auto* bLay = new QHBoxLayout(bottom);
    bLay->setContentsMargins(12,8,12,8);
    bLay->setSpacing(0);
    bLay->addStretch();

    archLay->addWidget(m_archived);
    archLay->addWidget(bottom);
    root->addWidget(m_archiveArea, 0, Qt::AlignTop);

    setCentralWidget(central);

    // дії
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(m_input,  &QLineEdit::returnPressed, this, &MainWindow::onAdd);
}


int MainWindow::listContentHeight(QListWidget* list) const {
    if (!list) return 0;
    const int n = list->count();
    if (n == 0) return 0;
    int h = list->contentsMargins().top() + list->contentsMargins().bottom();
    const int spacing = list->spacing();
    for (int i = 0; i < n; ++i) {
        auto* it = list->item(i);
        h += it->sizeHint().height();
        if (i + 1 < n) h += spacing;
    }
    return h;
}

void MainWindow::applyPalette() {
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette p = qApp->palette();
    p.setColor(QPalette::Window, QColor("#f6f7fb"));      // фон вікна (видно у верхній смужці)
    p.setColor(QPalette::Base,   QColor("#ffffff"));      // фон полів/редакторів
    p.setColor(QPalette::WindowText, Qt::black);
    p.setColor(QPalette::Text,       Qt::black);
    p.setColor(QPalette::ButtonText, Qt::black);
    p.setColor(QPalette::Button, QColor("#ffffff"));
    p.setColor(QPalette::Highlight,  QColor("#bde0ff"));
    p.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(p);

    setStyleSheet(R"(
        /* глобально */
        QWidget { color: black; }

        /* верхня смужка лишається ніжно-сірою */
        #TitleBar { background: #f6f7fb; }

        /* хедер + активні задачі — білі площини */
        #PinnedHeader { background: #ffffff; border: none; }
        #ActiveArea   { background: #ffffff; border: none; }
        QListWidget#ActiveList {
            background: #ffffff;
            border: none;
        }
        QListWidget#ActiveList::item:selected,
        QListWidget#ActiveList::item:hover { background: transparent; }

        /* архів — сіра площина */
        #ArchiveArea   { background: #eeeeee; border: none; }
        #ArchiveBottom { background: #eeeeee; border: none; }
        QListWidget#ArchivedList {
            background: transparent;  /* щоб просвічував сірий ArchiveArea */
            border: none;
        }
        QListWidget#ArchivedList::item:selected,
        QListWidget#ArchivedList::item:hover { background: transparent; }

        /* білі округлі контролі в хедері (і глобально) */
        QPushButton {
            background:#ffffff; color:black;
            border:1px solid #e7e7ea; border-radius:10px;
            padding:8px 14px;
        }
        QPushButton:hover  { background:#f2f2f2; }
        QPushButton:pressed{ background:#eaeaea; }

        QLineEdit {
            background:#ffffff; color:black;
            border:1px solid #e7e7ea; border-radius:10px;
            padding:8px 10px;
        }
        QLineEdit:focus { border:1px solid #d0d0ff; }

        /* кнопки смужки заголовка */
        #TitleMin, #TitleClose { border:none; background:transparent; padding:0; }
        #TitleMin:hover   { background:#eaeaea; border-radius:6px; }
        #TitleClose:hover { background:#e81123; color:white; border-radius:6px; }

        /* нульові зовнішні відступи секцій, щоб не було «смуг» між ними */
        QFrame#PinnedHeader, QFrame#ActiveArea, QFrame#ArchiveArea { margin:0; }
    )");
}


void MainWindow::reload() {
    m_active->clear();
    m_archived->clear();

    const auto all = m_manager.getTasks();
    int archivedCount = 0;

    for (const auto& t : all) {
        if (t.getStatus() == archived) {
            addTaskUi(m_archived, t);
            ++archivedCount;
        } else {
            addTaskUi(m_active, t);   // показуємо всі неархівні (toDO і done)
        }
    }

    m_archiveArea->setVisible(archivedCount > 0);

    m_active->setFixedHeight(listContentHeight(m_active));
    m_archived->setFixedHeight(listContentHeight(m_archived));

    const int cap = QGuiApplication::primaryScreen()->availableGeometry().height() * 85 / 100;
    adjustSize();
    if (height() > cap) resize(width(), cap);
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray& type, void* msg, qintptr* result) {
    MSG* m = static_cast<MSG*>(msg);
    if (m->message == WM_NCHITTEST) {
        const int bw = 6; // товщина активної зони для resize

        const int x = static_cast<int>(static_cast<short>(LOWORD(m->lParam)));
        const int y = static_cast<int>(static_cast<short>(HIWORD(m->lParam)));

        const QRect fr = frameGeometry();
        const int L = fr.left(), R = fr.right(), T = fr.top(), B = fr.bottom();

        const bool canW = minimumWidth()  != maximumWidth();
        const bool canH = minimumHeight() != maximumHeight();

        // --- ресайз по краях/кутам ---
        if (canW && x >= L && x < L + bw) {
            if (canH && y >= T && y < T + bw)   { *result = HTTOPLEFT;     return true; }
            if (canH && y <= B && y > B - bw)   { *result = HTBOTTOMLEFT;  return true; }
            *result = HTLEFT;  return true;
        }
        if (canW && x <= R && x > R - bw) {
            if (canH && y >= T && y < T + bw)   { *result = HTTOPRIGHT;    return true; }
            if (canH && y <= B && y > B - bw)   { *result = HTBOTTOMRIGHT; return true; }
            *result = HTRIGHT; return true;
        }
        if (canH && y >= T && y < T + bw)       { *result = HTTOP;    return true; }
        if (canH && y <= B && y > B - bw)       { *result = HTBOTTOM; return true; }

        // --- перетягування за смужку, але НЕ по кнопках ---
        const QPoint local = mapFromGlobal(QPoint(x, y));

        auto containsOnMain = [&](QWidget* w)->bool {
            if (!w) return false;
            const QRect r(QPoint(w->mapTo(this, QPoint(0,0))), w->size());
            return r.contains(local);
        };

        if (m_titleBar && m_titleBar->geometry().contains(local)) {
            // якщо під курсором одна з кнопок — віддаємо в клієнтську зону (клік спрацює)
            if (containsOnMain(m_btnMin) || containsOnMain(m_btnClose)) {
                *result = HTCLIENT;
                return true;
            }
            *result = HTCAPTION;
            return true;
        }
    }
    return QMainWindow::nativeEvent(type, msg, result);
}
#endif



    void MainWindow::addTaskUi(QListWidget* list, const Task& t) {
        auto* item = new QListWidgetItem();
        auto* w = new TaskItemWidget(t, list);
        item->setSizeHint(w->sizeHint());
        list->addItem(item);
        list->setItemWidget(item, w);

        connect(w, &TaskItemWidget::toggledDone,  this, &MainWindow::onItemToggled);
        connect(w, &TaskItemWidget::textEdited,   this, &MainWindow::onItemEdited);
        connect(w, &TaskItemWidget::archiveRequested, this, &MainWindow::onItemArchive);
        connect(w, &TaskItemWidget::deleteRequested,  this, &MainWindow::onItemDelete);
    }



void MainWindow::onAdd() {
    const auto text = m_input->text().trimmed();
    if (text.isEmpty()) return;
    m_manager.addtask(text.toStdString());
    m_manager.saveTasks();
    m_input->clear();
    reload();
}

void MainWindow::onItemToggled(size_t id, bool doneChecked) {
    m_manager.setStatus(id, doneChecked ? Status::done : Status::toDO);
    m_manager.saveTasks();
    reload();
}


void MainWindow::onItemEdited(size_t id, const QString& txt) {
    m_manager.setText(id, txt.trimmed().toStdString());
    m_manager.saveTasks();
    // без reload — локально теж ок, але для узгодженості:
    reload();
}

void MainWindow::onItemArchive(size_t id) {
    const auto all = m_manager.getTasks();
    auto it = std::find_if(all.begin(), all.end(),
                           [id](const Task& t){ return t.getTaskID() == id; });
    if (it == all.end()) return;

    Status newSt = (it->getStatus() == archived) ? toDO : archived;
    m_manager.setStatus(id, newSt);
    m_manager.saveTasks();
    reload(); // миттєво піде в інший список/зону
}


void MainWindow::onItemDelete(size_t id) {
    m_manager.deleteTask(id);
    m_manager.saveTasks();
    reload();
}

