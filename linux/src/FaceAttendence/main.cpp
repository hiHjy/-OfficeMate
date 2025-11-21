#include "faceattendence.h"
#include "register.h"
#include <QApplication>
#include "form.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    FaceAttendence w;
//    w.show();
//    Register reg;
//    reg.show();
    Form f;
    f.show();


    return a.exec();
}
