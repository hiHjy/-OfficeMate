#ifndef FACEATTENDENCE_H
#define FACEATTENDENCE_H
#include <QMainWindow>
#include <opencv.hpp>
#include <QThread>
#include "databasemannager.h"
#include <QMessageBox>
#include <QMutex>
using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui { class FaceAttendence; }
QT_END_NAMESPACE
class Work : public QThread {
    Q_OBJECT

public:
    Work(QWidget *parent = nullptr, cv::Mat *frame = nullptr, cv::CascadeClassifier *cascade = nullptr, QMutex* frameMutex = nullptr);
    ~Work();

    void run() override;
    cv::Mat *frame;
    cv::CascadeClassifier *cascade;
    QMutex  *frameMutex;
signals:
    void sigFaceReady(QString base64);
    void sigFaceTrace(int x, int y, bool status);
    void sigFaceCrop( QImage img);


};



class FaceAttendence : public QMainWindow
{
    Q_OBJECT

public:
    FaceAttendence(QWidget *parent = nullptr);
    ~FaceAttendence();
    //定时器事件
    //void timerEvent(QTimerEvent *e);
    void updateFrame();
    static FaceAttendence *getInstance();
    void startCamera();
    void stopCamera();
signals:
    void sigFaceVerified(UserInfo user);

private:
    Ui::FaceAttendence *ui;
    VideoCapture cap;
    QTimer *timer;
    Work *face_dect;
    cv::CascadeClassifier cascade;//一级级联分类器
    cv::Mat frame;
    static FaceAttendence* self;
    int old_x ;
    int old_y ;
    QMutex frameMutex;

};



#endif // FACEATTENDENCE_H
