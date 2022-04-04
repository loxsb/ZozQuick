#pragma once

#include <QtQuick/QQuickFramebufferObject>

class FingerCurveBFboRenderer;
class FingerCurveB : public QQuickFramebufferObject
{
    Q_OBJECT
public:
    FingerCurveB(QQuickItem* parent=nullptr);
    ~FingerCurveB();

    Renderer *createRenderer() const override;

    Q_INVOKABLE void clear();

private:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    friend class FingerCurveBFboRenderer;

    QVector<QPoint> _points;
};
