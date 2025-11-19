#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include <opencv.hpp>
namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr);
    ~Register();
   static Register* getInstance();

public slots:
    void on_Btn_Clear_clicked();

    void on_Btn_AddImg_clicked();

    void on_Btn_Register_clicked();

    void on_Btn_Cap_clicked();
    void timerEvent(QTimerEvent *e) override;
private slots:
    void on_Btn_Shot_clicked();

private:
    Ui::Register *ui;
    static Register* self;
    int timerID;
    cv::VideoCapture cap;
    cv::Mat image;



};

#endif // REGISTER_H
