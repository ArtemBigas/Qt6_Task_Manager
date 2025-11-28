#include "mainwindow.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //приложение при закрытии не выключается
    a.setQuitOnLastWindowClosed(false);
    MainWindow mainwindow;
    mainwindow.setWindowTitle("Qt6_Task_Manager");
    mainwindow.setWindowFlags(mainwindow.windowFlags() | Qt::WindowMaximizeButtonHint);
    mainwindow.show();
    return a.exec();
}
