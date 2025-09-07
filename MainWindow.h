#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QApplication>
#include <QLabel>
#include <QToolButton>
#include <QFrame>
#include "App.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(TaskManager& manager, QWidget* parent = nullptr);
protected:
#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray& type, void* msg, qintptr* result) override;
#endif
private:
    TaskManager& m_manager;

    QListWidget* m_active{};
    QListWidget* m_archived{};
    QFrame*      m_archiveArea{};
    QLineEdit*   m_input{};
    QPushButton* m_addBtn{};
    QFrame*      m_activeArea{};
    QFrame*      m_titleBar{};
    QToolButton* m_btnMin{};      // кнопка згортання
    QToolButton* m_btnClose{};    // кнопка закриття
    QFrame*      m_header{};   // закріплений хедер
    QFrame*      m_archiveFiller = nullptr;



    enum class Filter { All, Active, Done, Archived };

    void buildUi();
    void applyPalette();
    void reload();
    void addTaskUi(QListWidget* list, const Task& t);
    int listContentHeight(QListWidget* list) const;


private slots:
    void onAdd();
    void onItemToggled(size_t id, bool done);
    void onItemEdited(size_t id, const QString& txt);
    void onItemArchive(size_t id);
    void onItemDelete(size_t id);
};
