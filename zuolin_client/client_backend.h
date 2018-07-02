#ifndef CLIENT_BACKEND_H
#define CLIENT_BACKEND_H

#include <QObject>
#include "requestmaneger.h"
class ClientBackend : public QObject
{
    Q_OBJECT
public:
    explicit ClientBackend(QObject *parent = nullptr);

signals:
    void fileLoaded(const QVariantList _file_names,const QVariantList _file_authors,const QVariantList _file_sizes);
    void historyLoaded(const QVariantList _file_names,const QVariantList _file_authors,const QVariantList _file_sizes);
public slots:
    void file_ls(QString path);
    void file_history();
    void file_upload(QString filename);
    void file_download(QString filename);
    void file_rm(QString filename);
    void file_mv(QString filename);
    void file_detail(QString filename);
private:
    void handle_file_ls(QJsonObject& res_json);
    void handle_file_history(QJsonObject& res_json);
    void handle_file_upload(QJsonObject& res_json);
    void handle_file_download(QJsonObject& res_json);
    void handle_file_rm(QJsonObject& res_json);
    void handle_file_mv(QJsonObject& res_json);
    void handle_file_detail(QJsonObject& res_json);
};

#endif // CLIENT_BACKEND_H
