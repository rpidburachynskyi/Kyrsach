import QtQuick 2.0
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.13

Rectangle {
    id: connectPanel;
    color: "black";
    opacity: 0.7;

    visible: !client.clientSocket.connectedToControler;

    MouseArea {
        anchors.fill: parent;
    }

    ColumnLayout {
        anchors.centerIn: parent;
        width: parent.width;
        height: 180;
        spacing: 5;
        TextField {
            id: keyField;
            Layout.alignment: Qt.AlignCenter;
            Layout.preferredWidth: parent.width / 3;
            Layout.preferredHeight: 60;
            text: "KEYROSTIK"; // FIXIT
        }
        TextField {
            id: passField;
            Layout.alignment: Qt.AlignCenter;
            Layout.preferredWidth: parent.width / 3;
            Layout.preferredHeight: 60;
            text: "PASS"; // FIXIT
        }
        Button {
            Layout.alignment: Qt.AlignCenter;
            Layout.preferredWidth: parent.width / 4;
            Layout.preferredHeight: 60;

            text: "Connect";

            onClicked: {
                client.clientSocket.connectToControler(keyField.text, passField.text);
            }
        }
    }
}