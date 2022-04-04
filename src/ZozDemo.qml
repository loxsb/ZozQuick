import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import ZozQuick 1.0
import "./qml"
import "./qml/qrcode"
import "./qml/animation"

Row {

    Flickable {
        width: 200;
        height: parent.height;
        clip: true;
        contentHeight: titlecolumn.height;

        Column {
            id: titlecolumn;
            width: parent.width;
            spacing: 5;

            // 二维码示例
            Button {
                width: parent.width;
                text: "二维码";
                onClicked: {
                    loader.sourceComponent = qrcodeimage;
                }
                Component {
                    id: qrcodeimage;
                    ZQrcodeImage {
                        width: 400;
                        height: 400;
                        padding: 10;
                        anchors.centerIn: parent;
                        value: "https://www.baidu.com";
                    }
                }
            }

            // 滚动文字示例
            Button {
                width: parent.width;
                text: "滚动文字A";
                onClicked: {
                    loader.sourceComponent = rolltexta;
                }
                Component {
                    id: rolltexta;
                    Column {
                        anchors.centerIn: parent;
                        spacing: 10;
                        ZRollTextA {
                            anchors.horizontalCenter: parent.horizontalCenter;
                            width: 200;
                            font.pixelSize: 30;
                            text: "北国风光，千里冰封，万里雪飘，望长城内外，惟余莽莽；大河上下，顿失滔滔。";
                        }
                        ZRollTextA {
                            anchors.horizontalCenter: parent.horizontalCenter;
                            width: 300;
                            font.pixelSize: 40;
                            text: "北国风光，千里冰封，万里雪飘，望长城内外，惟余莽莽；大河上下，顿失滔滔。";
                        }
                    }
                }
            }

            // Wizard
            Button {
                width: parent.width;
                text: "wizard";
                onClicked: {
                    loader.sourceComponent = wizard;
                }
                Component {
                    id: wizard;
                    Item {
                        width: 800;
                        height: 800;
                        Component.onCompleted: {
                            ZWizard.initParent(parent);
                            ZWizard.launch("A", "qrc:/WizardDemo.qml");
                        }
                    }
                }
            }

            // 加载动画
            Button {
                width: parent.width;
                text: "加载动画";
                onClicked:  {
                    loader.sourceComponent = loadinganimation;
                }
                Component {
                    id: loadinganimation
                    Column {
                        anchors.horizontalCenter: parent.horizontalCenter;
                        LoadingA { }
                    }
                }
            }

            //
        }
    }

    Item {
        width: 800;
        height: 800;
        Loader {
            id: loader;
            anchors.centerIn: parent;
        }
    }
}
