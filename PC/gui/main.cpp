#include <QtGui/QApplication>
#include "qtknwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TKNWindow w;
    w.show();
    int retVal = a.exec();
    return retVal;

}
