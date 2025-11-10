#include "faceattendence.h"
#include "ui_faceattendence.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QByteArray>
#include "face_api.h"

QString global_token;
FaceAttendence* FaceAttendence::self = nullptr;

FaceAttendence::FaceAttendence(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceAttendence)
{
    ui->setupUi(this);
    self = this;
    ui->widget_2->hide();
    cap.open("/dev/video0");
    //startTimer(100);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FaceAttendence::updateFrame);
    timer->start(30); // 大约 30 FPS

    cap >> frame;
    if (frame.empty()) {
        qDebug() << "等待有效帧" ;
        return;
    }

    cascade.load("/home/hjy/Documents/office_meta/FaceAttendence/haarcascade_frontalface_default.xml");

    face_dect = new Work(this, &frame, &cascade);

    //face_dect->terminate();

    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);

    /* jpg转为base64 */

    QByteArray jpgData(reinterpret_cast<const char*>(buf.data()), buf.size());
    QString jpgBase64 = jpgData.toBase64();
    getAccessToken([this, jpgBase64](QString token){
        if(token.isEmpty()){
            qDebug() << "token 获取失败";
            return;
        }
        global_token = token;
        qDebug () << "token:\n" << token;
        face_dect->start();
        //faceSearch(jpgBase64, global_token);
    });
    connect(face_dect, &Work::sigFaceReady, this, [=](QString base64) {
            faceSearch(base64, global_token);
    });

    connect(this, &FaceAttendence::sigFaceVerified, this, [=](QString name){
        //ui->widget_2->setText("认证成功：" + name);
        ui->widget2->show();
        qDebug() << "uid:" << name << endl;
    });


}
void FaceAttendence::updateFrame()
{
    cap >> frame;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    QImage image(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
    ui->videoLb->setPixmap(QPixmap::fromImage(image));


}

FaceAttendence *FaceAttendence::getInstance()
{
    return self;
}

Work::Work(QWidget *parent, cv::Mat *frame, cv::CascadeClassifier *cascade) :
    QThread(parent),
    frame(frame),
    cascade(cascade)
{
    // 可以在这里进行初始化
}
FaceAttendence::~FaceAttendence()
{
    delete ui;
}

void Work::run()
{
    std::cout << "work start" << std::endl;
    cv::Mat gray;
    while (1) {

        cv::cvtColor(*this->frame, gray, cv::COLOR_BGR2GRAY);

        // --- 2) Haar 人脸检测 ---
        std::vector<cv::Rect> faces;
        this->cascade->detectMultiScale(gray, faces, 1.2, 3);
        if (faces.empty())
            continue;


        // --- 3) 给检测到的人脸画框 ---
        for (auto &face : faces) {
            qDebug() << "检测到人脸" << endl;
            cv::rectangle(*this->frame, face, cv::Scalar(0, 255, 0), 2);
        }

        // 保存为 JPG
        static qint64 last = 0;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - last < 2000) {  // 2000 ms = 2秒
            continue;
        }
        last = now;
        static int counter = 0;
        counter++;

        std::vector<uchar> buf;
        cv::imencode(".jpg", *this->frame, buf);

        /* jpg转为base64 */

        QByteArray jpgData(reinterpret_cast<const char*>(buf.data()), buf.size());
        QString jpgBase64 = jpgData.toBase64();
        qDebug() << "Base64 长度：" << jpgBase64.length();
        //faceSearch(jpgBase64, global_token);
        emit sigFaceReady(jpgBase64);
        // qDebug() << jpgBase64;



//        QString filename = QString("/home/hjy/Documents/linux-embed/qt/FaceAttendence/capture.jpg")
//              ;
//        QFile file(filename);
//        if (file.open(QIODevice::WriteOnly)) {
//            file.write((char*)buf.data(), buf.size());
//            file.close();
//            qDebug() << "保存图片:" << filename;

//            //✅ 下一步：把这个 JPG 文件传给你的人脸识别程序
//            system(QString("./face %1").arg(filename).toStdString().c_str());
//        }



    }
}

Work::~Work()
{
    std::cout << "work over" << std::endl;
}



