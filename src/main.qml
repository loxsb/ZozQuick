import QtQml 2.12
import QtQuick 2.12
import QtQuick.Window 2.12


Window {
    width: 1000;
    height: 800;
    visible: true;
    title: qsTr("Hello World");


    ZozDemo {
        anchors.fill: parent;
    }
}
