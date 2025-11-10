#ifndef FACE_API_H
#define FACE_API_H
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "faceattendence.h"
void faceSearch(QString base64Image, QString token);
void getAccessToken(std::function<void(QString)> callback);

#endif // FACE_API_H
