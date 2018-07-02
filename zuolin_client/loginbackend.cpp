#include "login_backend.h"
#ifdef DEBUG
#include <QDebug>
#endif
#include <QNetworkAccessManager>
#include "requestmaneger.h"
QString loginBackend::username()
{
    return _username;
}

loginBackend::loginBackend(QObject *parent) : QObject(parent)
{
    connect(&requestManeger, SIGNAL(get_response(QJsonObject)), this, SLOT(handle_login_response(QJsonObject)));
}

void loginBackend::set_username(const QString &username)
{
    if(_username == username) {
        return;
    }
    #ifdef DEBUG
    qDebug() << __func__ << ": username:" <<username;
    #endif
    this->_username = username;
    emit username_changed(username);
}

QString loginBackend::password()
{
    return _password;
}

void loginBackend::set_password(const QString &password)
{
    this->_password = password;
}

QString loginBackend::usertype()
{
    return _usertype;
}

void loginBackend::set_usertype(const QString &usertype)
{
    this->_usertype = usertype;
}

bool loginBackend::login_state()
{
    return _login_state;
}

void loginBackend::set_login_state(const bool state)
{
    if(_login_state == state) {
        return;
    }
    #ifdef DEBUG
    qDebug() << __func__ << ": state:" << state;
    #endif
    this->_login_state = state;
    emit login_state_changed(state);
}

bool loginBackend::login()
{
#ifdef DEBUG
    qDebug() << "Do login() and username is "
             << _username
             << " and pwd is "
             << _password << "\n";
#endif
    QJsonObject object
    {
        {"username", _username},
        {"password", _password}
    };
    requestManeger.send(object, "login");
    return true;
}

void loginBackend::handle_login_response(QJsonObject res_json)
{
    if (res_json.empty()){
        set_login_state(false);
    }
    if (res_json.contains("error")) {
        QString token =  res_json["token"].toString();
        if (res_json["error"].toInt() == 0 && token.length() != 0) {
               RequestManeger::token = token;
               RequestManeger::username = _username;
               set_login_state(true);
               return;
        }
    }
    set_login_state(false);
}
