import QtQuick 2.5
import QtQuick.Layouts 1.2
import QtQuick.Window 2.1
import QtQuick.Controls 1.3 as QuickControls
import Material 0.2
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1


Item {
    id: loginWindow

    visible: true;  
    property var currentExample: null;
    property bool usertype: true
    property bool loginState: false
    property bool loading: false
    property int  timeout: 5

    //登录超时计时器
    Timer {
        id:countDowm;
        repeat: true;
        interval: 1000;
        triggeredOnStart: true;
        onTriggered: {
            loginWindow.timeout --;
            if (loginWindow.timeout < 0) {
                loading = false
                loginStateLabel.visible = true
                loginWindow.timeout = 5
            }
        }
    }

    ListItem.Subheader {
        text: usertype ? "Administor" : "Nomal User"
    }

    Rectangle {        
        id: rootItem
        width: parent.width * 0.7
        height: parent.height * 0.7
        anchors.centerIn: parent

        LoginTextBox {
            id: textBoxID
            width: parent.width / 3
            anchors {
                top: parent.top
                topMargin: parent.height / 3
                horizontalCenter: parent.horizontalCenter
            }
            placeholder: qsTr("ID")
            Keys.onReturnPressed: {
                textBoxPass.forceActiveFocus(Qt.MouseFocusReason);
            }
            onTextChanged: {
                loginBtn.enabled = verify_input(text, textBoxPass.text)
            }
        }

        LoginTextBox {
            id: textBoxPass
            width: textBoxID.width
            anchors {
                top: textBoxID.bottom
                topMargin: 50
                horizontalCenter: parent.horizontalCenter
            }
            placeholder: qsTr("PASS")
            passwordMaskEnabled: true

            onTextChanged: {
                loginBtn.enabled = verify_input(textBoxID.text, text)
            }
            Behavior on opacity { NumberAnimation { duration: 500; easing.type: Easing.OutQuart } }
            Behavior on anchors.leftMargin { NumberAnimation { duration: 500; easing.type: Easing.OutQuart } }
        }

        Button {
            id: loginBtn
            text: "登录"
            elevation: 1
            enabled: false
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: textBoxPass.bottom
                topMargin: 50
            }
            onClicked: {
                login_backend.username = textBoxID.text;
                login_backend.password = textBoxPass.text;
                login_backend.login_state = false
                login_backend.login();
                countDowm.start();
                loading = true
                loginStateLabel.visible = false
            }
            visible: !loading
        }
        Label {
            id: loginStateLabel
            text : "登录超时请重试"
            visible: false
            color: "red"
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: loginBtn.bottom
                topMargin: 20
            }
        }

        ProgressCircle {
            anchors.centerIn: parent
            visible: loading
            color: "blue"
            anchors {
                top: textBoxID.bottom
                topMargin: 50
                horizontalCenter: parent.horizontalCenter
            }
        }
    }
    Connections
    {
        target: login_backend
        onLogin_state_changed:
        {
            console.log("Got signal from SerialCom in QML. passed bool value is: " + state);
            loginState = state
        }
    }

    ActionButton {
        anchors {
            right: parent.right
            top: parent.top
            margins: 64 //dp(32)
        }

        action: Action {
            id: addContent
            text: "&Switch"
            shortcut: "Ctrl+C"
            onTriggered: { 
                usertype ? usertype = false : usertype = true;
                console.info("triggle", usertype);
            }
        }
        iconName: "content/add"
    }
    function closeCurrentExample() {  
        currentExample.destroy();  
    }

    function verify_input(username, password) {
        if(username.length > 0 && password.length >= 6) {
            return true;
        }
        else {
            return false;
        }
    }
}
