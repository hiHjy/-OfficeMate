#include "faceattendence.h"
#include "register.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FaceAttendence w;
    w.show();
//    Register reg;
//    reg.show();


    return a.exec();
}
