#pragma once

#include <QtQuick/QQuickFramebufferObject>

class FingerCurveFboRenderer;
class FingerCurve : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(float lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(float feather READ feather WRITE setFeather NOTIFY featherChanged)
public:
    FingerCurve(QQuickItem* parent=nullptr);
    ~FingerCurve();

    Renderer *createRenderer() const override;

    Q_INVOKABLE void clear();

    QColor color() const {return _color;}
    void setColor(const QColor& color);

    float lineWidth() const {return _line_width;}
    void setLineWidth(float v);

    float feather() const {return _feather;}
    void setFeather(float v);

signals:
    void colorChanged();
    void lineWidthChanged();
    void featherChanged();
    void finished();

private:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    friend class FingerCurveFboRenderer;
    friend class TouchGraph;

    QColor _color;
    float _line_width;
    float _feather;

    QVector<QVector2D> _points;
    bool _in_drawing;
    bool _is_clear;
};
