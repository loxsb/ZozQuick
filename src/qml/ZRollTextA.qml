import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12

Text {
    id: text;
    font.pixelSize: 20;
    onTextChanged: animation.restart();
    leftPadding: 0;
    clip: (width<contentWidth);
    SequentialAnimation {
        id: animation; loops: Animation.Infinite;
        running: (text.width<text.contentWidth);
        NumberAnimation {
            target: text; property: "leftPadding";
            from: 0; to: (text.width-text.contentWidth);
            duration: Math.max(1000, 500*(text.contentWidth-text.width)/text.font.pixelSize);
        }
        PauseAnimation { duration: 800; }
        NumberAnimation {
            target: text; property: "leftPadding";
            to: 0; duration: 1;
        }
        PauseAnimation { duration: 1000; }
    }
}
