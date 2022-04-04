import QtQml 2.12
import QtQuick 2.12
import QtQuick.Controls 2.12
import ZozQuick 1.0
import QtQml.StateMachine 1.0 as DSM

Item {
    anchors.fill: parent

//    DSM.StateMachine {
//        id: stateMachine
//        initialState: s1
//        running: true
//        DSM.State {
//            id: s1
//            DSM.SignalTransition {
//                targetState: s2
////                signal: ha1.pressed
//                signal: area.pressed
//            }
//            onExited: {
//                console.log("s1 exited")
//                Wizard.enterView("s1")
//            }
//        }
//        DSM.State {
//            id: s2
//            DSM.SignalTransition {
//                targetState: s3
////                signal: ha2.pressed
//                signal: area.pressed
//            }
//            onExited: {
//                console.log("s2 exited")
//                Wizard.enterView("s2")
//            }
//        }
//        DSM.State {
//            id: s3
//            DSM.SignalTransition {
//                targetState: finalState
////                signal: ha3.pressed
//                signal: area.pressed
//            }
//            onExited: {
//                console.log("s3 exited")
//                Wizard.enterView("s3")
//            }
//        }
//        DSM.FinalState {
//            id: finalState
//        }

//        onFinished: {
//            console.log("state machine finished")
//            stop();
//            Wizard.finish();
//        }
//    }

    ZHoleArea {
        id: area;
        anchors.fill: parent;
        color: "#90000000"

        onPressed: {

        }

        ZRoundPolygon {
            x: 10
            y: 100
            propagateEvent: true
            points: [
                Qt.point(0, 0),
                Qt.point(50, 50),
                Qt.point(0, 100),
                Qt.point(100, 50)
            ];
            radius: [
                0.0,
                20.0,
                0.0,
                5.0
            ]
        }

        ZRoundPolygon {
            x: 120
            y: 100
            propagateEvent: true
            points: [
                Qt.point(0, 0),
                Qt.point(0, 100),
                Qt.point(100, 100),
                Qt.point(100, 0)
            ];
            radius: [
                50.0,
                20.0,
                10.0,
                0.0
            ]
        }

        ZRoundPolygon {
            x: 230
            y: 100
            propagateEvent: true
            points: [
                Qt.point(0, 40),
                Qt.point(80, 100),
                Qt.point(100, 0)
            ];
            radius: [
                10.0,
                20.0,
                5.0
            ]
        }

        ZRoundPolygon {
            x: 340
            y: 100
            propagateEvent: true
            points: [
                Qt.point(20, 0),
                Qt.point(40, 50),
                Qt.point(0, 100),
                Qt.point(50, 100),
                Qt.point(90, 50),
                Qt.point(70,0)
            ];
            radius: [
                1.0,
                5.0,
                9.0,
                13.0,
                17.0,
                21.0
            ]
        }

        ZRoundPolygon {
            x: 450
            y: 100
            propagateEvent: true
            points: [
                Qt.point(0, 0),
                Qt.point(10, 0),
                Qt.point(100, 0),
                Qt.point(50, 100)
            ]
            radius: [
                10,
                10,
                10
            ]
        }

        ZRoundPolygon {
            x: 560
            y: 100
            propagateEvent: true
            points: [
                Qt.point(50, 50),
                Qt.point(20, 70),
                Qt.point(70, 70),
                Qt.point(60, 10),
                Qt.point(0, 0),
                Qt.point(0, 70)
            ]
            radius: [
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
            ]
        }

        ZRoundPolygon {
            x: 670
            y: 100
            propagateEvent: true
            points: [
                Qt.point(20, 0),
                Qt.point(0, 152),
                Qt.point(234, 152),
                Qt.point(234, 0)
            ]
            radius: [
                20.0,
                10.0,
                0.0,
                0.0
            ]
        }

//        RoundRectangle {
//            id: ha2
//            x: 200;
//            y: 70;
//            width: 150
//            height: 150
//            radius: 20
//            visible: s2.active
//            propagateEvent: false
//        }

//        RoundRectangle {
//            id: ha3
//            x: 200;
//            y: 130;
//            width: 150
//            height: 150
//            radius: 20
//            visible: s3.active
//            propagateEvent: false
//        }
    }
}
