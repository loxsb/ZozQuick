#include "ZWizardFramework.h"


HoleShape::HoleShape(QObject *parent)
    : QObject(parent),
      _x(8),
      _y(0),
      _pressed(false),
      _visible(true),
      _propagateEvent(false)
{
}

QPainterPath HoleShape::shape()
{}

void HoleShape::update()
{
    auto item = qobject_cast<HoleArea*>(parent());
    if(item){
        item->updateShapes();
        item->update();
    }
}

qreal HoleShape::x() const
{
    return _x;
}

void HoleShape::setX(const qreal &x)
{
    if(qFuzzyCompare(x, _x)) return;
    _x = x;
    update();
    emit xChanged();
}

qreal HoleShape::y() const
{
    return _y;
}

void HoleShape::setY(const qreal &y)
{
    if(qFuzzyCompare(y, _y)) return;
    _y = y;
    update();
    emit yChanged();
}

bool HoleShape::visible() const
{
    return _visible;
}

void HoleShape::setVisible(bool v)
{
    if(v==_visible) return;
    _visible = v;
    update();
    emit visibleChanged();
}

bool HoleShape::propagateEvent() const
{
    return _propagateEvent;
}

void HoleShape::setPropagateEvent(bool v)
{
    if(v==_propagateEvent) return;
    _propagateEvent = v;
    emit propagateEventChanged();
}

bool HoleShape::contains(const QPointF &point)
{
    if(!_visible) return false;
    return shape().contains(point);
}

void HoleShape::setPress()
{
    _pressed = true;
    emit pressed();
}

HoleArea::HoleArea(QQuickItem* parent)
    : QQuickPaintedItem(parent),
      _color(1, 1, 1, 100)
{
//    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(false); // rely on mouse events synthesized from touch
//    setFiltersChildMouseEvents(true);
    setFlag(QQuickItem::ItemHasContents);
}

HoleArea::~HoleArea()
{
}

void HoleArea::paint(QPainter *painter)
{
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setBrush(_color);

    QPainterPath clipPath;
    for(auto shape: _shapes){
        clipPath = clipPath.united(shape->shape());
    }

    QPainterPath path;
    QSizeF itemSize = size();
    path.addRect(QRectF(0, 0, itemSize.width(), itemSize.height()));
    path = path.subtracted(clipPath);

    painter->drawPath(path);
}

QColor HoleArea::color() const
{
    return _color;
}

void HoleArea::setColor(const QColor &v)
{
    if(_color == v) return;
    _color = v;
    update();
    emit colorChanged();
}

void HoleArea::mousePressEvent(QMouseEvent *event)
{
    auto shape = findShape(event->localPos());
    grabMouse();
    setKeepMouseGrab(true);
    if(shape){
        shape->setPress();
        if(shape->propagateEvent())
            event->ignore();
        else
            event->accept();
    }else{
        event->accept();
    }
    emit pressed();
}

void HoleArea::mouseReleaseEvent(QMouseEvent *event)
{
    auto shape = findShape(event->localPos());
    if(shape){
//        shape->setRelease();
    }
    QQuickWindow *w = window();
    if (w && w->mouseGrabberItem() == this)
        ungrabMouse();
    setKeepMouseGrab(false);
}

void HoleArea::mouseMoveEvent(QMouseEvent *event)
{
}

void HoleArea::mouseUngrabEvent()
{
    setKeepMouseGrab(false);
}

void HoleArea::componentComplete()
{
    updateShapes();
    QQuickPaintedItem::componentComplete();
}

void HoleArea::updateShapes()
{
    _shapes.clear();
    auto cds = children();
    for(auto cd: cds){
        auto shape = qobject_cast<HoleShape*>(cd);
        if(shape && shape->visible()){
            _shapes.append(shape);
        }
    }
}

HoleShape *HoleArea::findShape(const QPointF &point)
{
    for(int i=0; i<_shapes.size(); ++i){
        if(_shapes.at(i)->contains(point))
            return _shapes.at(i);
    }
    return nullptr;
}


RoundRectangle::RoundRectangle(QObject *parent)
    : HoleShape(parent),
      _width(0),
      _height(0),
      _radius(0)
{}

QPainterPath RoundRectangle::shape()
{
    QPainterPath path;
    path.addRoundedRect(_x, _y, _width, _height, _radius, _radius);
    return path;
}

qreal RoundRectangle::width() const
{
    return _width;
}

void RoundRectangle::setWidth(qreal v)
{
    if(qFuzzyCompare(v, _width)) return;
    _width = v;
    update();
    emit widthChanged();
}

qreal RoundRectangle::height() const
{
    return _height;
}

void RoundRectangle::setHeight(qreal v)
{
    if(qFuzzyCompare(v, _height)) return;
    _height = v;
    update();
    emit heightChanged();
}

qreal RoundRectangle::radius() const
{
    return _radius;
}

void RoundRectangle::setRadius(qreal v)
{
    if(qFuzzyCompare(v, _radius)) return;
    _radius = v;
    update();
    emit radiusChanged();
}



RoundPolygon::RoundPolygon(QObject *parent)
    :HoleShape(parent)
{

}

QPainterPath RoundPolygon::shape()
{
    if(_points.size()<3) return QPainterPath();

    while(_radius.size()<_points.size())
        _radius.append(0);

    // ?????????
    auto included_angle = [](const QVector2D& v1, const QVector2D& v2){
        auto cos_ag = QVector2D::dotProduct(v1, v2)/(v1.length()*v2.length());
        auto angle = qAcos(cos_ag);
        return angle;
    };

    // ??????????????????
    auto intersection_point = [](const QPointF& p1, const QPointF& n1, const QPointF& p2, const QPointF& n2){
        float a = n1.x();
        float b = p1.x();
        float c = n1.y();
        float d = p1.y();

        float e = n2.x();
        float f = p2.x();
        float g = n2.y();
        float h = p2.y();

        auto dd = c * e - a * g;
        if(qFuzzyIsNull(dd)){
            return QPointF();
        }

        auto x = (-a*b*g-c*d*g+c*e*f+c*g*h)/dd;
        auto y = (a*b*e-a*e*f-a*g*h+c*d*e)/dd;
        return QPointF(x, y);
    };


    QPointF p1, p2, p3; // ???????????? ????????? ????????????
    QVector2D p1_v, p3_v; // ?????????????????? ??????????????????
    float angle, r; // ?????? ??????
    float start_angle, sweep_length; // ???????????????????????? ??????????????????
    QPointF arc_centre_p, arc_start_p, arc_end_p; // ?????? ?????? ??????
    QVector2D arc_start_v, arc_end_v; // ????????????????????? ???????????????

    QPainterPath path;

    int len = _points.size();
    p1 = _points.last();
    p2 = _points.last();
    for(int i=0; i<len; i++){
        // ??????????????????????????????
        p1 = p2;
        p2 = _points.at((i) % len);
        p3 = _points.at((i+1) % len);

        // ??????????????????????????????
        r = _radius.at(i);
        if(qFuzzyIsNull(r)){
            if(i==0){
                path = QPainterPath(p2);
            }else{
                path.lineTo(p2);
            }
            continue;
        }

        // ???????????????????????? ??????????????????
        p1_v = QVector2D(p1-p2);
        p3_v = QVector2D(p3-p2);

        // ????????????
        angle = included_angle(p1_v, p3_v);

        // ???????????????????????????
        auto len_contact = r / qTan(angle * 0.5);

        // ???????????? ?????? ?????? ??????
        arc_start_p  = p1_v.toPointF() * (len_contact/p1_v.length()) + p2;
        arc_end_p    = p3_v.toPointF() * (len_contact/p3_v.length()) + p2;
        arc_centre_p = intersection_point(arc_start_p, p1_v.toPointF(), arc_end_p, p3_v.toPointF());

        // ?????? ???????????? ????????????
        arc_start_v = QVector2D(arc_start_p-arc_centre_p);
        arc_end_v = QVector2D(arc_end_p-arc_centre_p);

        // ???????????????
        start_angle = included_angle(QVector2D(1.0, 0.0), arc_start_v);
        // ???????????????????????????
        start_angle = arc_start_p.y() > arc_centre_p.y()? 2 * M_PI - start_angle : start_angle;

        // ?????????????????????????????????????????????????????????
        sweep_length = QVector3D::crossProduct(QVector3D(p1_v), QVector3D(p3_v)).z()>0? M_PI - angle : angle - M_PI;

        // ???path???????????????
        if(i==0) path = QPainterPath(arc_start_p);

        // ????????????
        path.arcTo(arc_centre_p.x()-r, arc_centre_p.y()-r, r*2, r*2, qRadiansToDegrees(start_angle), qRadiansToDegrees(sweep_length));
    }

    // ????????????
    QPainterPath path_tf;
    path_tf.addPolygon(path.toFillPolygon(QTransform().translate(_x, _y)));
    return path_tf;
}

QVariantList RoundPolygon::points() const
{
    QVariantList list;
    for(int i=0; i<_points.size(); i++)
        list.append(_points.at(i));
    return list;
}

void RoundPolygon::setPoints(const QVariantList &v)
{
    _points.clear();
    for(int i=0; i<v.size(); i++)
        _points.append(v.at(i).toPointF());
    update();
    emit pointsChanged();
}

QList<qreal> RoundPolygon::radius() const
{
    return _radius;
}

void RoundPolygon::setRadius(const QList<qreal> &v)
{
    if(v==_radius) return;
    _radius = v;
    update();
    emit radiusChanged();
}



WizardFramework::WizardFramework(QObject *parent)
    :QObject(parent),
      _parentItem(nullptr),
      _wizardItem(nullptr),
      _flow("")
{
    qDebug() << Q_FUNC_INFO;
}

WizardFramework::~WizardFramework()
{
    qDebug() << Q_FUNC_INFO;
    finish();
}

void WizardFramework::initParent(QQuickItem *item)
{
    _parentItem = item;
    if(_wizardItem) _wizardItem->setParentItem(_parentItem);
}

void WizardFramework::launch(const QString &flow, const QString &qml)
{
    if(flow.isEmpty() || qml.isEmpty())
        return;
    finish();
    _flow = flow;
    if(!createItem(qml)){
        _flow.clear();
    }
}

void WizardFramework::enterView(const QString &stage)
{
    if(_flow.isEmpty()) return;
    emit currentEnterView(_flow, stage);
}

bool WizardFramework::createItem(const QString &qml)
{
    auto engine = qmlEngine(_parentItem);
    if(qml.isEmpty() || !engine) return false;
    QQmlComponent comp(engine, qml);
    auto obj = comp.beginCreate(engine->rootContext());
    _wizardItem = qobject_cast<QQuickItem*>(obj);
    if(!_wizardItem) {
        qWarning() << Q_FUNC_INFO << comp.errorString();
        return false;
    }
    _wizardItem->setParentItem(_parentItem);
    _wizardItem->setVisible(true);
    comp.completeCreate();
    return true;
}

void WizardFramework::finish()
{
    if(_wizardItem){
        _wizardItem->deleteLater();
        _wizardItem = nullptr;
    }
    if(!_flow.isEmpty()){
        _flow.clear();
    }
}







