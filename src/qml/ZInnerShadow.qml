import QtQml 2.12
import QtQuick 2.12

Item {
    id: item;
    width: 400;
    height: 400;
    anchors.centerIn: parent;

    property int count: 60;
    property real radius: 60;
    property int samples: 1;
    property color color: "#15FFFFFF";
    property bool enableAnimation: false;
    property alias duration: animation.duration;

    NumberAnimation {
        id: animation;
        target: repeater;
        running: enableAnimation;
        loops: 1;
        property: "idx";
        from: -1;
        to: count;
        duration: 250;
    }

    Repeater {
        id: repeater;
        anchors.fill: parent;
        model: item.count;
        property int idx: -1;
        property int lastIdx: 0;
        onIdxChanged: {
            for(let i=repeater.lastIdx; i<repeater.idx; i++)
                repeater.itemAt(i).visible = true;
            repeater.lastIdx = repeater.idx;
        }
        Rectangle {
            anchors.centerIn: parent;
            radius: Math.max(0, item.radius-index*item.samples);
            width: parent.width-index*2*item.samples;
            height: parent.height-index*2*item.samples;
            color: "transparent";
            border.width: item.samples;
            border.color: Qt.rgba(item.color.r, item.color.g, item.color.b, (1.0-index/repeater.count)/(1.0/item.color.a));
            visible: !enableAnimation;
        }
    }
}
