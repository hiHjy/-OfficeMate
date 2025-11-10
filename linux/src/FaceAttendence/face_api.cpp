#include "face_api.h"

// 百度AI开放平台的API认证信息
// client_id: 客户端ID，用于标识应用
QString client_id = "EuabRmosANnliPgnNxB32rDt";
// client_secret: 客户端密钥，用于验证应用身份
QString client_secret = "8GGFznpVx7azYYSCFampuULKW3UxgLvp";

// 获取百度AI访问令牌的函数
// 参数: callback - 回调函数，获取到token后会调用此函数
void getAccessToken(std::function<void(QString)> callback)
{
    // 创建网络访问管理器，用于发送HTTP请求
    QNetworkAccessManager* manager = new QNetworkAccessManager;

    // 设置请求的URL - 百度AI的令牌获取接口
    QUrl url("https://aip.baidubce.com/oauth/2.0/token");
    QNetworkRequest request(url);

    // 设置请求头，指定内容类型为表单数据
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // 构建请求参数
    QUrlQuery params;
    // 授权类型：客户端凭证模式
    params.addQueryItem("grant_type", "client_credentials");
    // 客户端ID
    params.addQueryItem("client_id", client_id);
    // 客户端密钥
    params.addQueryItem("client_secret", client_secret);

    // 连接网络请求完成信号到槽函数
    QObject::connect(manager, &QNetworkAccessManager::finished,
                     [callback](QNetworkReply* reply)
    {
        // 读取服务器返回的所有数据
        QByteArray response = reply->readAll();

        // 将JSON格式的响应数据解析为文档对象
        QJsonDocument doc = QJsonDocument::fromJson(response);

        // 从JSON文档中提取access_token字段的值
        QString token = doc["access_token"].toString();

        // 调用回调函数，将获取到的token传递给调用者
        callback(token);

        // 清理回复对象，防止内存泄漏
        reply->deleteLater();
    });

    // 发送POST请求，将参数转换为URL编码格式
    manager->post(request, params.toString(QUrl::FullyEncoded).toUtf8());
}

// 人脸搜索函数 - 在指定人脸库中搜索匹配的人脸
// 参数: base64Image - Base64编码的人脸图片数据
//        token - 访问令牌，用于API认证
void faceSearch(QString base64Image, QString token)
{
    // 创建网络访问管理器
    QNetworkAccessManager* manager = new QNetworkAccessManager;

    // 构建人脸搜索API的完整URL，包含访问令牌
    QString url = QString(
                "https://aip.baidubce.com/rest/2.0/face/v3/search?access_token=%1"
                ).arg(token);

    // 创建网络请求对象
    QNetworkRequest request(url);
    // 设置请求头，指定内容类型为JSON
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建JSON请求体
    QJsonObject json;
    // Base64编码的图片数据
    json["image"] = base64Image;
    // 图片类型：Base64编码
    json["image_type"] = "BASE64";
    // 要搜索的人脸库组ID列表，这里搜索组"11"
    json["group_id_list"] = "11";

    // 将JSON对象转换为文档
    QJsonDocument doc(json);

    // 连接网络请求完成信号
    QObject::connect(manager, &QNetworkAccessManager::finished,
                     [](QNetworkReply* reply)
    {
        // 读取返回的识别结果
        QString result = reply->readAll();


        // 输出识别结果到调试窗口
        qDebug() << "识别结果：" << result;

        // 这里可以添加JSON解析代码来提取具体的识别信息
        // 例如：QJsonDocument resultDoc = QJsonDocument::fromJson(result.toUtf8());
        // 然后从resultDoc中提取人脸匹配分数、用户信息等

        QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
        QJsonObject obj = doc.object();
        //FaceAttendence *mainWin = FaceAttendence :: getInstance();

        qDebug() << "obj[\"error_code\"].toInt() " << obj["error_code"].toInt() << endl;
        if (obj["error_code"].toInt() == 0) {
            auto user = obj["result"].toObject()["user_list"].toArray()[0].toObject();
            QString name = user["user_id"].toString();
            emit FaceAttendence :: getInstance()->sigFaceVerified(name);
        }


        // 清理回复对象
        reply->deleteLater();
    });

    // 发送POST请求，将JSON数据作为请求体
    manager->post(request, doc.toJson());

}
