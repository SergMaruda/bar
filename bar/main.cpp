#include "stdafx.h"
#include "bar.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    bar w;

    w.show();


    return a.exec();
}
