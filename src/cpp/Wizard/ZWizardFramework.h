#pragma once

#include <QtQuick>

class HoleShape : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool propagateEvent READ propagateEvent WRITE setPropagateEvent NOTIFY propagateEventChanged)

public:
    HoleShape(QObject* parent = nullptr);
    virtual QPainterPath shape() = 0;

    qreal x() const;
    void setX(const qreal& x);

    qreal y() const;
    void setY(const qreal& y);

    bool visible() const;
    void setVisible(bool v);

    bool propagateEvent() const;
    void setPropagateEvent(bool v);

    bool contains(const QPointF& point);
    void setPress();

protected:
    void update();

signals:
    void xChanged();
    void yChanged();
    void visibleChanged();
    void propagateEventChanged();
    void pressed();

protected:
    qreal _x;
    qreal _y;
    bool _pressed:1;
    bool _visible:1;
    bool _propagateEvent:1;
};


class RoundRectangle : public HoleShape
{
    Q_OBJECT
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(qreal radius READ radius WRITE setRadius NOTIFY radiusChanged)
public:
    RoundRectangle(QObject* parent = nullptr);
    QPainterPath shape() override;

    qreal width() const;
    void setWidth(qreal v);

    qreal height() const;
    void setHeight(qreal v);

    qreal radius() const;
    void setRadius(qreal v);

signals:
    void widthChanged();
    void heightChanged();
    void radiusChanged();

private:
    qreal _width;
    qreal _height;
    qreal _radius;
};


class RoundPolygon : public HoleShape
{
    Q_OBJECT
    Q_PROPERTY(QVariantList points READ points WRITE setPoints NOTIFY pointsChanged)
    Q_PROPERTY(QList<qreal> radius READ radius WRITE setRadius NOTIFY radiusChanged)

public:
    RoundPolygon(QObject* parent=nullptr);
    QPainterPath shape() override;

    QVariantList points() const;
    void setPoints(const QVariantList& v);

    QList<qreal> radius() const;
    void setRadius(const QList<qreal>& v);

signals:
    void pointsChanged();
    void radiusChanged();

private:
    QList<QPointF> _points;
    QList<qreal> _radius;
};


class HoleArea : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    HoleArea(QQuickItem* parent = nullptr);
    ~HoleArea();
    void paint(QPainter *painter) override;

    QColor color() const;
    void setColor(const QColor& v);

signals:
    void colorChanged();
    void pressed();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseUngrabEvent() override;
    void componentComplete() override;

private:
    void updateShapes();
    HoleShape* findShape(const QPointF& point);

private:
    friend class HoleShape;
    QColor _color;
    QList<HoleShape*> _shapes;
};


class WizardFramework: public QObject
{
    Q_OBJECT
public:
    WizardFramework(QObject* parent=nullptr);
    ~WizardFramework();

    Q_INVOKABLE void initParent(QQuickItem* item);
    Q_INVOKABLE void launch(const QString& flow, const QString& qml);
    Q_INVOKABLE void enterView(const QString& stage);
    Q_INVOKABLE void finish();

signals:
    void currentEnterView(const QString& flow, const QString& stage);

private:
    bool createItem(const QString& qml);

private:
    QQuickItem* _parentItem;
    QQuickItem* _wizardItem;
    QString _flow;
};




