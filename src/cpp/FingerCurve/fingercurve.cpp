#include "fingercurve.h"
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLExtraFunctions>




class FingerCurveFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
    FingerCurveFboRenderer() {
    }
    void render() override {
        _curve.render();
    }
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }
    // 同步数据
    void synchronize(QQuickFramebufferObject *item) override {
        auto old_item = static_cast<FingerCurve*>(item);
        if(old_item){
            _curve.color = old_item->_color;
            _curve.view_size = old_item->size();
            _curve.line_width = old_item->_line_width;
            _curve.feather = old_item->_feather;
            if(old_item->_is_clear){
                old_item->_points.clear();
                _curve.pos.clear();
                old_item->_is_clear = false;
            }
            if(!old_item->_points.isEmpty()){
                if(old_item->_points.size()<3 && _curve.pos.isEmpty())
                    return;
                _curve.addPos(old_item->_points);
                old_item->_points.clear();
            }
        }
    }

private:
    struct FingerCurveRenderer : protected QOpenGLExtraFunctions
    {
        QOpenGLShaderProgram program;
        unsigned int VBO, VAO;
        int pos_a, normal_a, color_u, size_u, line_width_u, feather_u;


        QColor color;
        QSizeF view_size;
        float line_width=20;
        float feather = 0.2;

        QVector<GLfloat> pos;
        int last_length=0;
        QVector2D last_pos;
        QVector2D pre_pos;

        FingerCurveRenderer() {
            init();
        }
        ~FingerCurveRenderer() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        };
        void init() {
            initializeOpenGLFunctions();

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            program.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                       "attribute vec2 a_pos;"
                                                       "attribute vec2 a_normal;"

                                                       "uniform highp float u_linewidth;"
                                                       "uniform highp vec2 u_size;"
                                                       "varying highp vec3 v_pos;"

                                                       "void main() {"
                                                       "    float width = u_linewidth / u_size.x;"
                                                       "    vec4 delta = vec4(a_normal.x, a_normal.y, 0.0, 0.0)*width;"
                                                       "    vec4 pos = vec4(2.0*(a_pos.x/u_size.x-0.5), 2.0*(a_pos.y/u_size.y-0.5), 0.0, 1.0);"
                                                       "    v_pos = vec3(a_normal, width);"
                                                       "    gl_Position = delta + pos;"
                                                       "}");
            program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                       "uniform highp vec4 u_color;"
                                                       "uniform highp float u_feather;"
                                                       "varying highp vec3 v_pos;"

                                                       "void main() {"
//                                                       "    float alpha = smoothstep(v_pos.z*0.5-u_feather, v_pos.z*0.5+u_feather, 1.0-length(v_pos.xy));"
//                                                       "    gl_FragColor = u_color*alpha;"
                                                       "    gl_FragColor = u_color;"
                                                       "}");

            program.link();
            pos_a = program.attributeLocation("a_pos");
            normal_a = program.attributeLocation("a_normal");
            color_u = program.uniformLocation("u_color");
            size_u = program.uniformLocation("u_size");
            line_width_u = program.uniformLocation("u_linewidth");
            feather_u = program.uniformLocation("u_feather");


            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            create();
        }

        void addPos(const QVector<QVector2D>& ps){

            auto cl = [](const QVector2D& p1, const QVector2D& p2, const QVector2D& p3){
                auto ov1 = p2-p1;
                auto ov2 = p3-p2;
                auto on1 = QVector2D(ov1.y(), -ov1.x()).normalized();
                auto on2 = QVector2D(ov2.y(), -ov2.x()).normalized();
                return (on1+on2)/(1+QVector2D::dotProduct(on1, on2));
            };

            int i=-1;
            if(pos.isEmpty()){
                auto p1 = ps.at(0);
                auto p2 = ps.at(1);
                auto p3 = ps.at(2);
                auto v = p2-p1;
                auto n = QVector2D(v.y(), -v.x()).normalized();
                pos.append(p1.x()), pos.append(p1.y()), pos.append(-n.x()), pos.append(-n.y());
                pos.append(p1.x()), pos.append(p1.y()), pos.append(n.x()), pos.append(n.y());

                auto n2 = cl(p1, p2, p3);
                pos.append(p2.x()), pos.append(p2.y()), pos.append(-n2.x()), pos.append(-n2.y());
                pos.append(p2.x()), pos.append(p2.y()), pos.append(n2.x()), pos.append(n2.y());

                pre_pos = p2;
                last_pos = p3;
                i = 2;
            }


            QVector2D np;
            QVector2D n;
            while (i<(ps.size()-1)) {
                np = ps.at(++i);
                n = cl(pre_pos, last_pos, np);
                pos.append(last_pos.x()), pos.append(last_pos.y()), pos.append(-n.x()), pos.append(-n.y());
                pos.append(last_pos.x()), pos.append(last_pos.y()), pos.append(n.x()), pos.append(n.y());

                pre_pos = last_pos;
                last_pos = np;
            }

        }
        void create() {

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            if(last_length!=pos.length()) {
                glBufferData(GL_ARRAY_BUFFER, pos.size()*sizeof(GLfloat), pos.data(), GL_STREAM_DRAW);
                last_length = pos.length();
            }

            glVertexAttribPointer(pos_a, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(pos_a);
            glVertexAttribPointer(normal_a, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
            glEnableVertexAttribArray(normal_a);
        }

        void render() {
            glDepthMask(true);

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//            glViewport(0, 0, 50, 50);
//            glFrontFace(GL_CW);
//            glCullFace(GL_FRONT);
//            glEnable(GL_CULL_FACE);
//            glEnable(GL_DEPTH_TEST);


            program.bind();
            program.setUniformValue(color_u, color);
            program.setUniformValue(size_u, view_size);
            program.setUniformValue(line_width_u, line_width);
            program.setUniformValue(feather_u, feather);
            create();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, pos.size()/4);//GL_LINE_STRIP GL_TRIANGLE_STRIP

            program.release();


//            glDisable(GL_DEPTH_TEST);
//            glDisable(GL_CULL_FACE);
        }
    } _curve;
};


FingerCurve::FingerCurve(QQuickItem* parent):
    QQuickFramebufferObject(parent),
    _color("#000000"),
    _line_width(6),
    _feather(0.1),
    _in_drawing{true},
    _is_clear{false}
{
    setFlags(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
}

FingerCurve::~FingerCurve()
{

}

QQuickFramebufferObject::Renderer *FingerCurve::createRenderer() const
{
    return new FingerCurveFboRenderer();
}

void FingerCurve::clear()
{
    _points.clear();
    _in_drawing = true;
    _is_clear = true;
    update();
}

void FingerCurve::setColor(const QColor &color)
{
    if(color==_color) return;
    _color = color;
    emit colorChanged();
    update();
}

void FingerCurve::setLineWidth(float v)
{
    if(qFuzzyCompare(v, _line_width) || v<0)
        return;
    _line_width = v;
    emit lineWidthChanged();
    update();
}

void FingerCurve::setFeather(float v)
{
    if(qFuzzyCompare(v, _feather) || v<0)
        return;
    _feather = v;
    emit featherChanged();
    update();
}

void FingerCurve::mouseMoveEvent(QMouseEvent *event)
{

    if(_in_drawing){
        _points.append(QVector2D(event->pos()));
        update();
    }
}

void FingerCurve::mousePressEvent(QMouseEvent *event)
{
    if(_in_drawing){
        _points.clear();
        _points.append(QVector2D(event->pos()));
    }
}

void FingerCurve::mouseReleaseEvent(QMouseEvent *event)
{
    if(_in_drawing)
        emit finished();
    _in_drawing = false;
}








