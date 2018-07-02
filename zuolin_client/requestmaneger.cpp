#include "requestmaneger.h"
#include <QJsonValue>
#include <QJsonDocument>
#include <QNetworkReply>
QString RequestManeger::token = "";
QString RequestManeger::username = "";
void RequestManeger::send(QJsonObject req_json, QString method)
{
    request.setUrl(QUrl(url+QString("/")+method));
    QByteArray data = QJsonDocument(req_json).toJson();
    qDebug() << "Sync" << QString::fromUtf8(data.data(), data.size());
    manager->post(request, data);
}

void RequestManeger::set_url(QString url)
{
    this->url = url;
}

void RequestManeger::set_callback(void (*callback)(void*,  QJsonObject &), void *p_callback)
{
    this->callback = callback;
    this->callback_P = p_callback;
}

void RequestManeger::requestFinished(QNetworkReply *reply)
{
    QJsonObject res_json = QJsonDocument::fromJson(reply->readAll()).object();
    if (callback){
        (*callback)(this->callback_P, res_json);
    }
    emit get_response(res_json);

}
