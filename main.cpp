#include "widget.h"
#include <QApplication>
#include "game.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);    

    Widget w;
    w.show();

    int result;
    result = a.exec();



    return result;
}
