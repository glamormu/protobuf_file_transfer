#ifndef REQUESTMANEGER_H
#define REQUESTMANEGER_H

#include <QObject>
#include <QJsonObject>
#include <QNetworkAccessManager>
class RequestManeger : public QObject
{
    Q_OBJECT
public:
    explicit RequestManeger(QObject *parent = nullptr){
        url = "https://cvzhang.cn";
        manager = new QNetworkAccessManager();
        reply = NULL;
        callback = NULL;
        config = QSslConfiguration::defaultConfiguration();
        config.setProtocol(QSsl::TlsV1_2OrLater);
        request.setSslConfiguration(config);
        request.setUrl(QUrl(url));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
    }

private:
    static RequestManeger* requestManeger;
    QNetworkAccessManager *manager;
    QNetworkRequest request;
    QNetworkReply *reply;
    QSslConfiguration config;
    QString url;
    void (*callback)(void *, QJsonObject &);
    void *callback_P;

signals:
    void get_response(QJsonObject res_json);
public:
    void send(QJsonObject req_json, QString method);
    void set_url(QString url);
    void set_callback(void (*callback)(void *, QJsonObject &), void *p_callback = nullptr);
    static QString token;
    static QString username;
private slots:
    void requestFinished(QNetworkReply* reply);
};


#endif // REQUESTMANEGER_H
