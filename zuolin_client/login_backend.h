#ifndef LOGIN_BACKEND_H
#define LOGIN_BACKEND_H
#include <QQuickItem>
#include <QObject>
#include <QJsonObject>
#include "config.h"
#include "requestmaneger.h"
class loginBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ username WRITE set_username NOTIFY username_changed)
    Q_PROPERTY(QString password READ password WRITE set_password)
    Q_PROPERTY(QString usertype READ usertype WRITE set_usertype)
    //Q_PROPERTY(bool login_state READ login_state WRITE set_login_state NOTIFY login_state_changed)
    Q_PROPERTY(bool login_state READ login_state WRITE set_login_state)

public:

    explicit loginBackend(QObject *parent = nullptr);
    QString username();
    void set_username(const QString& username);
    QString password();
    void set_password(const QString& password);
    QString usertype();
    void set_usertype(const QString& usertype);
    bool login_state();
    void set_login_state(const bool state);
signals:
    void username_changed(const QString& username);
    void login_state_changed(const bool state);
public slots:
    bool login();
    void handle_login_response(QJsonObject res_json);

private:
    QString _username;
    QString _usertype;
    QString _password;
    bool _login_state;
    RequestManeger requestManeger;
};

#endif // LOGIN_BACKEND_H
