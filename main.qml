import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

import UIControl 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    UIControl {
        id: control
    }

    Button {
        id: btn_sample1
        x: 25
        y: 41
        text: qsTr("asio ftp connect")
        onClicked: control.ftp_connect()

    }

    Button {
        id: sample2
        x: 139
        y: 41
        text: qsTr("Sample2")
        onClicked: control.OnClick2()
    }

}
