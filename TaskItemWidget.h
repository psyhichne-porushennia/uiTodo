#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QToolButton>
#include <QMenu>
#include <QEvent>
#include <QSpacerItem>
#include "App.h"

class TaskItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit TaskItemWidget(const Task& task, QWidget* parent = nullptr);
    size_t taskId() const { return m_taskId; }
    void syncFrom(const Task& t); // оновити віджет із моделі

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void toggledDone(size_t taskId, bool done);
    void textEdited(size_t taskId, QString text);
    void archiveRequested(size_t taskId);
    void deleteRequested(size_t taskId);

private:
    size_t       m_taskId{};
    QCheckBox*   m_check{};
    QLineEdit*   m_edit{};
    QToolButton* m_menuBtn{};
    QMenu*       m_menu{};
    bool         m_isArchived{false};
    QSpacerItem* m_cbSpacer{};

    void buildUi(const Task& task);
    void applyStyle();

    void styleMenu();      // стилізація QMenu (округлення + тінь)
    void updateMenuMask(); // оновлення маски меню після Show/Resize
};
