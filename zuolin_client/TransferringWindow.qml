import QtQuick 2.4
import Material 0.2
import Material.ListItems 0.1 as ListItem
import Material.Extras 0.1


Item {
    property var sections: ["Transferring","Finished"]
    property var file_names_transferring:["File one", "File two", "File Three"]
    property var file_names_finished:["File Four"]
    property var file_names: [file_names_transferring, file_names_finished]

    property var file_state_transferring: ["downloading", "uploading", "waiting"]
    property var file_state_finished: ["downloaded in /home/vergil/doc/"]
    property var file_states: [file_state_transferring, file_state_finished]

    property var transferring_processes: [["90%", "80%", "0%"],["80k"]]


    Flickable {
        id:flickable
        anchors {
            fill: parent
            margins: dp(32)
        }
        contentHeight: Math.max(content.implicitHeight, height)

        Column {
            id: content
            anchors.fill: parent
            Repeater {
                id:section_id
                model: sections
                delegate: Column {
                    width: parent.width

                    ListItem.Subheader {
                        text: sections[index]
                    }
                    property var file_name: file_names[index]
                    property var file_state: file_states[index]
                    property var transferring_process: transferring_processes[index]
                    Repeater {
                        model: file_name
                        delegate: ListItem.Subtitled {
                            iconName:"/file/file_doc"
                            text: qsTr(modelData)
                            subText: file_state[index]
                            valueText: transferring_process[index]
                            maximumLineCount: 2
                            backgroundColor: "white"
                            onClicked: {
                                actionSheet.open()
                            }
                        }
                    }
                }
            }
        }
    }
    Scrollbar {
        flickableItem: flickable
    }
    BottomActionSheet {
        id: actionSheet


        actions: [

            Action {
                iconName: "file/file_download"
                name: "Pause"
            },

            Action {
                iconName: "action/settings"
                name: "Cancel"
                hasDividerAfter: true
            }
        ]
    }
}
