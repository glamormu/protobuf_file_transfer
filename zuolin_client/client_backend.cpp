#include "client_backend.h"
#include "requestmaneger.h"
#include <QJsonArray>

ClientBackend::ClientBackend(QObject *parent) : QObject(parent)
{
}

/*
 * curl -H "Content-Type: application/json" -X POST
 * --data '{
 *      "username":"vergil","token":"..",
 *      "command":"ls",
 *      "dir":"zhang"}'
 * https://cvzhang.cn/files
 */
void ClientBackend::file_ls(QString path)
{
    RequestManeger *requestManeger = new RequestManeger();
    QJsonObject object
    {
        {"username", RequestManeger::username},
        {"token", RequestManeger::token},
        {"command", "ls"},
        {"dir", "zhang"}
    };
    typedef void (*CALLBACK_FUNC)(void*, QJsonObject& obj);
    requestManeger->set_callback((CALLBACK_FUNC)&ClientBackend::handle_file_ls, this);
    requestManeger->send(object, "files");
}

void ClientBackend::handle_file_ls(QJsonObject &res_json)
{
    if(res_json.isEmpty()|| !res_json.contains("error")) {
        return;
    }
    int err = res_json["error"].toInt();
    if(err!=0) {
        fprintf(stderr, "Error code is %d\n", err);
    }
    QJsonArray filename_list = res_json["data"].toArray();
    QVariantList names;
    QVariantList authors;
    QVariantList filesizes;
    for(auto filename : filename_list) {
        names << filename.toString();
        authors << "zsr";
        filesizes << "1000";
    }
    emit fileLoaded(names, authors, filesizes);
}


void ClientBackend::file_history()
{
    RequestManeger* requestManeger = new RequestManeger();
    QJsonObject object {
        {"username", RequestManeger::username},
        {"token", RequestManeger::token},
        {"command", "history"}
    };
    typedef void (*CALLBACK_FUNC)(void*, QJsonObject& obj);
    requestManeger->set_callback((CALLBACK_FUNC)&ClientBackend::handle_file_history, this);
    requestManeger->send(object, "files");
}

void ClientBackend::file_upload(QString filename)
{

}

void ClientBackend::file_download(QString filename)
{

}

void ClientBackend::file_rm(QString filename)
{

}

void ClientBackend::file_mv(QString filename)
{

}

void ClientBackend::file_detail(QString filename)
{

}

void ClientBackend::handle_file_history(QJsonObject &res_json)
{
    if(res_json.isEmpty()|| !res_json.contains("error")) {
        return;
    }
    int err = res_json["error"].toInt();
    if(err!=0) {
        fprintf(stderr, "Error code is %d\n", err);
        return;
    }
    QJsonArray filename_list = res_json["files"].toArray();
    QVariantList names;
    QVariantList authors;
    QVariantList filesizes;
    for(auto filename : filename_list) {
        names << filename.toString();
        authors << "zsr";
        filesizes << "1000";
    }
    emit historyLoaded(names, authors, filesizes);
}

void ClientBackend::handle_file_upload(QJsonObject &res_json)
{

}

void ClientBackend::handle_file_download(QJsonObject &res_json)
{

}

void ClientBackend::handle_file_rm(QJsonObject &res_json)
{

}

void ClientBackend::handle_file_mv(QJsonObject &res_json)
{

}

void ClientBackend::handle_file_detail(QJsonObject &res_json)
{

}
