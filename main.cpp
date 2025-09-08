#include <QApplication>
#include "App.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    auto repo = std::make_unique<FileRepository>("tasks.json");
    TaskManager manager(std::move(repo));

    MainWindow w(manager);
    w.show();

    return a.exec();
}
