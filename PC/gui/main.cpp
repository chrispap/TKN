#include <QtGui/QApplication>
#include "TKN_Window.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TKN_Window w;
    w.show();
    int retVal = a.exec();
    return retVal;

}
