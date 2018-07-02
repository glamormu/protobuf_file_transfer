import QtQuick 2.5
import QtQuick.Window 2.2

Window {
    id: rootWindow
    visible: true
    width: 640;
    height: 320;
    minimumWidth: 640;
    minimumHeight: 320;
    color: "lightgray";

    LoginStack {
        id: loginStack
        anchors.rightMargin: -320
        anchors.leftMargin: -320
        anchors.bottomMargin: -160
        anchors.top: parent.verticalCenter
        anchors.right: parent.horizontalCenter
        anchors.bottom: parent.verticalCenter
        anchors.left: parent.horizontalCenter
        anchors.topMargin: -158
        onLoginStateChanged: {
            if(loginStack.loginState == false)
                return
            var comp = Qt.createComponent("qrc:/MainWindow.qml");
            if(comp.status === Component.Ready){
                comp.createObject(rootWindow,{"color" : rootWindow.color});
                rootWindow.visible = false
            }
            else {
                console.info("no log:", comp.status)
                console.info("err:",Component.Error, comp.errorString());
            }
        }
    }
}
