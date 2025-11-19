#include "register.h"
#include "ui_register.h"
#include <QFileDialog>
#include <QtDebug>
#include <QMessageBox>
#include "face_api.h"
#include "databasemanager.h"

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);
    self = this;
    DatabaseManager::getInstance()->initDataBase();
}
Register* Register::self = nullptr;
Register::~Register()
{
    delete self;
    delete ui;
}

Register* Register::getInstance()
{
    if (!self)
        self = new Register();
    return self;
}

void Register::on_Btn_Clear_clicked()
{
    ui->Le_Name->clear();
    ui->Le_WorkID->clear();
    ui->Le_Identity->clear();
    ui->Le_Path->clear();
    ui->Le_Dept->clear();
    ui->Lb_Img->clear();

}

void Register::on_Btn_AddImg_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this);

    //qDebug() << filePath << endl;
    ui->Le_Path->setText(filePath);

    QPixmap mmp(filePath);
    mmp = mmp.scaled(384, 387);
    //    mmp = mmp.scaledToWidth(ui->Lb_Img->width());
    //    mmp = mmp.scaledToHeight(ui->Lb_Img->height());
    ui->Lb_Img->setPixmap(mmp);
}

void Register::on_Btn_Register_clicked()
{
    // 1. 获取用户输入
    QString name = ui->Le_Name->text().trimmed();
    QString workId = ui->Le_WorkID->text().trimmed();
    QString identity = ui->Le_Identity->text().trimmed();
    QString imgPath = ui->Le_Path->text().trimmed();
    QString dept = ui->Le_Dept->text().trimmed();


    qDebug() << name << endl;
    if (name.isEmpty() || workId.isEmpty() || identity.isEmpty() || imgPath.isEmpty()) {
        QMessageBox::warning(this, "提示", "信息填写不完整！");
        Register::on_Btn_Clear_clicked();
        return;
    }

    QFile file(imgPath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法读取图片文件！");
        return;
    }

    QByteArray imgData = file.readAll();
    QString imgBase64 = imgData.toBase64();
    getAccessToken([=](QString token){
        faceRegister(imgBase64, token, name, workId, identity, imgPath, dept);

    });





}

void Register::on_Btn_Cap_clicked()
{

    if (ui->Btn_Cap->text() == "打开摄像头") {
        ui->Btn_Cap->setText("关闭摄像头");

        if (!cap.open("/dev/video0")) {
            QMessageBox::warning(this, "错误", "摄像头打开失败");
            return;
        }

        //start timerEvent
        timerID = startTimer(100);

    } else {
        ui->Btn_Cap->setText("打开摄像头");
        ui->Lb_Img->clear();
        killTimer(timerID);
        cap.release();
    }
}

void Register::timerEvent(QTimerEvent *e)
{

    cap >> image;
    if (image.empty()) {
        QMessageBox::warning(this, "error", "摄像头异常");
        return;
    }

    // OpenCV 默认 BGR，需要转成 RGB 才能给 QImage
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);

    // 用 QImage 包装 Mat 的像素数据
    QImage qImage(
        image.data,
        image.cols,
        image.rows,
        image.step1(),
        QImage::Format_RGB888
    );

    // —— 关键逻辑 ——
    // 先根据 QLabel 的尺寸进行缩放
    QPixmap qPixmap = QPixmap::fromImage(qImage)
                           .scaled(ui->Lb_Img->size(),    // ← 完全按 QLabel 当前尺寸缩放
                                   Qt::KeepAspectRatio,   // ← 保持比例，不会拉伸变形
                                   Qt::SmoothTransformation);  // ← 平滑缩放，画质更好

    // 显示缩放后的图像
    ui->Lb_Img->setPixmap(qPixmap);
}

void Register::on_Btn_Shot_clicked()
{
    QString headFile = QString("./data/%1.jpg").arg(QString(ui->Le_WorkID->text().toUtf8().toBase64()));
    ui->Le_Path->setText(headFile);
    cv::imwrite(headFile.toUtf8().data(), image);
    ui->Btn_Cap->setText("打开摄像头");

    killTimer(timerID);
    cap.release();
}
