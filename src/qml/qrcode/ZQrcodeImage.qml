import QtQuick 2.12
import "qrious.js" as QRious

Item {
    width: 300;
    height: 300;



    property url logo: "";
    property real padding: 0;
    property alias radius: bg.radius;
    property alias background: bg.color;
    property color foreground: "#000000";
    property string value: "";
    //容错率 "L"==7% "M"==15% "Q"==25% "H"==30%
    property string level: "L";


//    property bool useCanves: false

    onValueChanged: {
//        useCanves? canvas.createQrcode() : grid.createQrcode();
        canvas.createQrcode();
//        grid.createQrcode()
    }
//    onLevelChanged: {
//        useCanves? canvas.createQrcode() : grid.createQrcode();
//        canvas.createQrcode();
//        grid.createQrcode()
//    }
//    onPaddingChanged: {
//        useCanves? canvas.createQrcode() : grid.createQrcode();
//        canvas.createQrcode();
//        grid.createQrcode()
//    }



    Rectangle {
        id: bg;
        anchors.fill: parent;
        color: background;
    }


    Canvas {
        id: canvas;
        width: Math.min(parent.width, parent.height)-padding*2;
        height: width;
        anchors.centerIn: parent;
        function createQrcode(){
            if(!available) return;
            let ctx = getContext("2d");
            ctx.reset();
            requestPaint();
        }
        onPaint: {
            if(value==="") return;
            let ctx = getContext("2d");
            ctx.fillStyle = foreground;
            let frame = new QRious.Frame({ level: level, value: value });
            let moduleSize = Math.floor(width/frame.width);
            let offset = Math.floor((width-moduleSize*frame.width)*0.5);
            for (let i = 0; i < frame.width; i++) {
                for (let j = 0; j < frame.width; j++) {
                    if (frame.buffer[j * frame.width + i]) {
                        ctx.fillRect(moduleSize * i + offset, moduleSize * j + offset, moduleSize, moduleSize);
                    }
                }
            }
        }
    }


    // 绘制效率很低，每次都需要绘制无效矩形
//    Grid {
//        id: grid;
//        width: Math.min(parent.width, parent.height)-parent.padding*2;
//        height: width;
//        rows: columns;
//        anchors.centerIn: parent;
//        function createQrcode(){
//            if(value===""){
//                repeater.model = 0;
//                return;
//            }
//            let frame = new QRious.Frame({ level: level, value: value });
//            grid.columns = frame.width;
//            repeater.moduleSize = Math.floor(grid.width/frame.width);
//            repeater.model = frame.buffer;
//        }
//        Repeater {
//            id: repeater;
//            property real moduleSize: 0;
//            model: 0;
//            Rectangle {
//                width: repeater.moduleSize;
//                height: width;
//                color: modelData? foreground:"#00FFFFFF";
//            }
//        }
//    }
}
