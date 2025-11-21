#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "faceattendence.h"
namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

private slots:
    void on_tabWidget_currentChanged(int index);

private:
    Ui::Form *ui;
};

#endif // FORM_H
