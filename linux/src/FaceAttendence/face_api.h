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
int faceSearch(QString base64Image, QString token);
void faceRegister(QString base64Image, QString token,
                  QString name, QString workId,
                  QString identity, QString imgPath, QString dept);
void getAccessToken(std::function<void(QString)> callback);
extern QString global_token;

#endif // FACE_API_H
