#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}

void Form::on_tabWidget_currentChanged(int index)
{
    if (index == 0) {
        FaceAttendence::getInstance()->startCamera();
    } else {
        FaceAttendence::getInstance()->stopCamera();
    }
}
