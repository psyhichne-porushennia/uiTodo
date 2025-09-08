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

RESOURCES += icons.qrc

VCPKG_ROOT = C:/vcpkg
VCPKG_TRIPLET = x64-windows
INCLUDEPATH += $$VCPKG_ROOT/installed/$$VCPKG_TRIPLET/include

win32:msvc {
    QMAKE_CXXFLAGS += -Zc:__cplusplus /utf-8
}
