import QtQuick 2.12
import QtGraphicalEffects 1.12


//不规则图片
Item {
    id: main;


    //图片资源
    property alias source: image.source;
    //异步加载
    property alias asynchronous: image.asynchronous;
    //遮罩mask图片
    property alias maskSource: r.maskSource;
    //圆角大小
    property alias radius: mask.radius;
    // 如果图片加载错误，则使用该设置的图片
    property url defaultImage: "";
    //
    property alias cache: image.cache;


    Image {
        id: image;
        anchors.fill: parent;
        visible: false;
        sourceSize: Qt.size(width, height);
        onStatusChanged: {
            if(status===Image.Error || status===Image.Null){
                source = defaultImage;
            }
        }
    }

    Rectangle {
        id: mask;
        anchors.fill: parent;
        radius: height*0.5;
        visible: false;
    }

    OpacityMask {
        id: r;
        anchors.fill: image;
        source: image;
        maskSource: mask;
    }
}
