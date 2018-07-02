#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQmlContext>
#include "login_backend.h"
#include "client_backend.h"
void init_context(QQmlContext* context)
{
    loginBackend* _login_backend = new loginBackend();
    ClientBackend* _clientBackend = new ClientBackend();
    context->setContextProperty("login_backend", _login_backend);
    context->setContextProperty("client_backend", _clientBackend);
}
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    init_context(engine.rootContext());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
