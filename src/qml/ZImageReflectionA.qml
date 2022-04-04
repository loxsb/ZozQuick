import QtQml 2.12
import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    property alias source: img.source;
    property real spacing: 5;
    property alias depth: reflecte.depth; // 0.0-1.0
    property alias reflectionOpacity: reflecte.opacity;
    property alias hideReflection: reflecte.visible;

    Image {
        id: img;
        sourceSize: Qt.size(width, height);
        anchors.fill: parent;
    }


    ShaderEffect {
        id: reflecte;
        anchors.top: parent.bottom;
        anchors.topMargin: spacing;
        width: parent.width;
        height: parent.height;

        property real depth: 1.0;
        property variant source: img;

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform lowp float qt_Opacity;
            uniform lowp sampler2D source;
            uniform highp float height;
            uniform highp float width;
            uniform highp float depth;
            void main() {
                lowp vec2 coord = vec2(qt_TexCoord0.x, 1.0-qt_TexCoord0.y);
                lowp vec4 pt = texture2D(source, coord);
                gl_FragColor = pt * qt_Opacity * (1.-smoothstep(0., depth, qt_TexCoord0.y));
            }"
    }
}
