# uiTodo.pro — qmake (Qt Widgets, C++20, nlohmann/json через vcpkg + SVG і ресурси)

QT       += widgets svg
CONFIG   += c++20
TEMPLATE = app
TARGET   = todo

SOURCES += \
    main.cpp \
    App.cpp \
    MainWindow.cpp \
    TaskItemWidget.cpp

HEADERS += \
    App.h \
    MainWindow.h \
    TaskItemWidget.h

# Ресурси з іконками для кнопок (мін/клоуз)
RESOURCES += icons.qrc

# ---- nlohmann/json із vcpkg (header-only) ----
VCPKG_ROOT = C:/vcpkg
VCPKG_TRIPLET = x64-windows
INCLUDEPATH += $$VCPKG_ROOT/installed/$$VCPKG_TRIPLET/include

# ---- MSVC quality-of-life ----
win32:msvc {
    QMAKE_CXXFLAGS += -Zc:__cplusplus /utf-8
}
