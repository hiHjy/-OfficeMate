#include "faceattendence.h"
#include "ui_faceattendence.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QByteArray>
#include "face_api.h"


FaceAttendence* FaceAttendence::self = nullptr;

FaceAttendence::FaceAttendence(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceAttendence)
{
    ui->setupUi(this);
    self = this;
    ui->widget_2->hide();
    old_x = ui->head_cap_img->x();
    old_y = ui->head_cap_img->y();

    DatabaseManager *db = DatabaseManager::getInstance();
    if (!db->initDataBase()) {
        qDebug() << "initDataBase error" << endl;
    }


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

    connect(this, &FaceAttendence::sigFaceVerified, this, [=](UserInfo user){
        //ui->widget_2->setText("认证成功：" + name);
        ui->widget_2->show();
        if (!user.valid) {
            qDebug() << "User not found. The user may not be registered" << endl;

        } else {


            qDebug() << "识别成功：";
            qDebug() << "姓名:" << user.name;
            qDebug() << "工号:" << user.workId;
            qDebug() << "身份:" << user.identity;
            qDebug() << "学院"  << user.dept;
            ui->LE_Name->setText(user.name);
            ui->LE_Work_ID->setText(user.workId);
            ui->LE_identity->setText(user.identity);
            ui->LE_Dept->setText(user.dept);

            //识别成功后存入数据库，记录访客时间
            if (!DatabaseManager::getInstance()->insertVisitRecord(user.workId)) {

                QMessageBox::warning(this, "错误", "记录访客导数据库失败！");
                return;
            }



        }

    });

    connect(face_dect, &Work::sigFaceTrace, this, [=](int x, int y, bool status) {

        if (status)
            ui->head_cap_img->move(x, y);
        else {
            ui->head_cap_img->move(old_x, old_y);
        }

    });


}
void FaceAttendence::updateFrame()
{
    cv::Mat tmp;
    cap >> frame;

    {
        QMutexLocker locker(&frameMutex);
        tmp = frame;
    }

    cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
    QImage image(tmp.data, tmp.cols, tmp.rows,  tmp.step1(),QImage::Format_RGB888);

    ui->videoLb->setPixmap(QPixmap::fromImage(image));
    //    ui->videoLb->setPixmap(QPixmap::fromImage(image).scaled(ui->videoLb->size(),
    //                                        Qt::KeepAspectRatio,
    //                                        Qt::SmoothTransformation));

}


FaceAttendence *FaceAttendence::getInstance()
{
    return self;
}

Work::Work(QWidget *parent, cv::Mat *frame, cv::CascadeClassifier *cascade, QMutex *frameMutex)
    :
      QThread(parent),
      frame(frame),
      cascade(cascade),
      frameMutex(frameMutex)
{
    // 可以在这里进行初始化
}
FaceAttendence::~FaceAttendence()
{
    if (face_dect) {
        face_dect->requestInterruption();
        face_dect->wait(); // 等待线程结束
        delete face_dect;
        face_dect = nullptr;
    }
    delete ui;

}

void Work::run()
{
    std::cout << "work start" << std::endl;
    cv::Mat gray;
    cv::Mat work_frame;
    while (!isInterruptionRequested()) {
        {
            QMutexLocker locker(frameMutex);
            work_frame = *frame;

        }
        cv::cvtColor(work_frame, gray, cv::COLOR_BGR2GRAY);

        // --- 2) Haar 人脸检测 ---
        std::vector<cv::Rect> faces;

        this->cascade->detectMultiScale(gray, faces, 1.2, 3);
        Rect rect;
        if (faces.empty() ) {
            emit sigFaceTrace(0, 0, false);
            continue;
        }

        rect = faces[0];
        // --- 3) 给检测到的人脸画框 ---
        if (faces.size() > 0) {
            qDebug() << "检测到人脸" << endl;

            //cv::rectangle(*this->frame, face, cv::Scalar(0, 255, 0), 2);
            emit sigFaceTrace(rect.x, rect.y, true);
        } else {

        }



        static qint64 last = 0;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - last < 2000) {  // 2000 ms = 2秒
            continue;
        }
        last = now;
        static int counter = 0;
        counter++;

        std::vector<uchar> buf;
        cv::imencode(".jpg", work_frame, buf);

        /* jpg转为base64 */

        QByteArray jpgData(reinterpret_cast<const char*>(buf.data()), buf.size());
        QString jpgBase64 = jpgData.toBase64();
        qDebug() << "Base64 长度：" << jpgBase64.length();
        //faceSearch(jpgBase64, global_token);
        emit sigFaceReady(jpgBase64);
        // qDebug() << jpgBase64;
        usleep(10000);


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



