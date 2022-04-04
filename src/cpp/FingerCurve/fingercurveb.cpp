#include "fingercurveb.h"
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickWindow>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLExtraFunctions>




class FingerCurveBFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
    FingerCurveBFboRenderer() {
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
        auto old_item = static_cast<FingerCurveB*>(item);
        if(old_item){
            _curve.view_size = old_item->size();
            if(!old_item->_points.isEmpty()){
                _curve.addPos(old_item->_points);
                old_item->_points.clear();
            }
        }
    }

private:
    struct FingerCurveBRenderer : protected QOpenGLExtraFunctions
    {
        QOpenGLShaderProgram program;
        unsigned int VBO, VAO;
        int pos_a, normal_a, color_u, size_u, line_width_u, feather_u;


        bool is_clear = false;
        QColor color;
        QSizeF view_size;
        float line_width=6;
        float feather = 0.5;

        QVector<GLfloat> pos;
        QPoint last_pos;

        FingerCurveBRenderer() {
            init();
        }
        ~FingerCurveBRenderer() {
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
                                                       "varying highp vec4 v_pos;"
                                                       "varying highp float v_linewidth;"

                                                       "void main() {"
                                                       "    highp vec4 delta = vec4((a_normal * u_linewidth / u_size).xy,  0.0, 0.0);"
                                                       "    highp vec4 pos = vec4(2.0*(a_pos.x/u_size.x-0.5), 2.0*(a_pos.y/u_size.y-0.5), 0.0, 1.0);"
                                                       "    v_pos = vec4(pos.xy, a_normal.xy);"
                                                       "    v_linewidth = u_linewidth / u_size.x;"
                                                       "    gl_Position = delta + pos;"
                                                       "}");
            program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                       "uniform highp vec4 u_color;"
                                                       "uniform highp float u_feather;"
                                                       "varying highp vec4 v_pos;"
                                                       "varying highp float v_linewidth;"

                                                       "void main() {"
                                                       "    highp float alpha = smoothstep(0.0, 1.0, 1.0-length(v_pos.zw));"
//                                                       "    vec4 c_color = vec4(vec3(step(v_linewidth, distance(gl_FragCoord.xy, vec2(0.0)))), 1.0);"
                                                       "    gl_FragColor = u_color*alpha;"//u_color*alpha+
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

        void addPos(const QVector<QPoint>& ps){

            auto f = ps.first();
            if(f.x()==INT_MAX && f.y()==INT_MAX){
                is_clear = true;
                return;
            }

            int i = 0;
            if(f.x()==INT_MAX){
                last_pos = ps.at(1);
                if(ps.size()>2)
                    i=2;
                else
                    return;
            }

            QPoint p1;
            QPoint p2;
            QPoint v;
            QVector2D n;
            for(; i<ps.size(); i++){
                p1 = last_pos;
                p2 = ps.at(i);

                v = p2-p1;
                n = QVector2D(v.y(), -v.x()).normalized();
                pos.append(p1.x()), pos.append(p1.y()), pos.append(-n.x()), pos.append(-n.y());
                pos.append(p1.x()), pos.append(p1.y()), pos.append(n.x()), pos.append(n.y());

                pos.append(p2.x()), pos.append(p2.y()), pos.append(-n.x()), pos.append(-n.y());
                pos.append(p2.x()), pos.append(p2.y()), pos.append(n.x()), pos.append(n.y());

                last_pos = p2;
            }
        }
        void create() {

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, pos.size()*sizeof(GLfloat), pos.data(), GL_STREAM_DRAW);

            glVertexAttribPointer(pos_a, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(pos_a);
            glVertexAttribPointer(normal_a, 2, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
            glEnableVertexAttribArray(normal_a);
        }

        void render() {
            glDepthMask(true);

            if(is_clear){
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }else{
                glClear(GL_DEPTH_BUFFER_BIT); //GL_COLOR_BUFFER_BIT
            }

            program.bind();
            program.setUniformValue(color_u, color);
            program.setUniformValue(size_u, view_size);
            program.setUniformValue(line_width_u, line_width);
            program.setUniformValue(feather_u, feather);
            if(!is_clear){
                create();
            }else{
                is_clear = false;
            }
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, pos.size()/4);//GL_LINE_STRIP GL_TRIANGLE_STRIP

            program.release();

            pos.clear();
        }
    } _curve;
};


FingerCurveB::FingerCurveB(QQuickItem* parent):
    QQuickFramebufferObject(parent)
{
    setFlags(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
}

FingerCurveB::~FingerCurveB()
{

}

QQuickFramebufferObject::Renderer *FingerCurveB::createRenderer() const
{
    return new FingerCurveBFboRenderer();
}

void FingerCurveB::clear()
{
    _points.clear();
    _points.append(QPoint(INT_MAX, INT_MAX));
    update();
}

void FingerCurveB::mousePressEvent(QMouseEvent *event)
{
    _points.clear();
    _points.append(QPoint(INT_MAX, 0));
    _points.append(event->pos());
}

void FingerCurveB::mouseMoveEvent(QMouseEvent *event)
{
    _points.append(event->pos());
    update();
}

void FingerCurveB::mouseReleaseEvent(QMouseEvent *event)
{
    _points.append(event->pos());
    update();
}








