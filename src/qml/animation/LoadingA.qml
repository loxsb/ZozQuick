import QtQml 2.12
import QtQuick 2.12

Item {
    Row {
        id: row
        anchors.centerIn: parent;
        height: 100
        spacing: 6;
        property real curTime: 0
        SequentialAnimation {
            running: true;
            loops: Animation.Infinite;
            NumberAnimation {
                target: row;
                properties: "curTime";
                from: 0; to: 1;
                duration: 2000;
            }
        }
        Repeater {
            id: repeater;
            model: 8;
            Rectangle {
                radius: 2;
                anchors.verticalCenter: parent.verticalCenter;
                height: Math.abs(Math.sin(2*Math.PI*1*row.curTime-index*(2*Math.PI/(2*repeater.model))))*40+10;
                width: 5;
                color: "#00A45E";
            }
        }
    }
}
