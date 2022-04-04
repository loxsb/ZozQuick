#include "ZPageListView.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QtMath>
#include <QDateTime>
#include <QVariant>
#include <QTimer>


#define PAGE_STEAL_MOUSE_THRESHOLD 5 // 页面窃取鼠标事件 必须小于PAGE_TURN_DRAG_THRESHOLD
#define PAGE_TURN_DRAG_THRESHOLD 10 // 翻页拖动阈值 pixel
#define PAGE_TURN_MVVEL_THRESHOLD 0.1 // 翻页拖动速度阈值 pixel/ms


/**********************************************************************************
 * ******
 * ********************************************************************************/
ZPageView::ZPageView(ZPageListViewPrivate* d)
    :QObject(),
      d{d},
      _page{nullptr}
{
    qDebug() << Q_FUNC_INFO;
    resize();
}

ZPageView::~ZPageView()
{
    qDebug() << Q_FUNC_INFO;
    qDeleteAll(_items);
    _items.clear();
    _cacheData.clear();
    _page->deleteLater();
}

void ZPageView::resize()
{
//    qDebug() << Q_FUNC_INFO;
    if(!_page){
        _page = new QQuickItem{d->mContentItem};
    }
    _page->setParentItem(d->mContentItem);
    _page->setPosition(QPointF{0, 0});
    _page->setSize(QSizeF{d->pageWidth(), d->pageHeight()});

    if(d->pageSize()==_items.count()) return;

    const int i = qAbs(d->pageSize()-_items.count());
    if (d->pageSize()>_items.count()){
        for(int j=0; j<i; j++){
            // 创建每页的Item对象
            auto item = d->createDelegateItem(_page);
            if(item){
                item->setProperty("index", j);
                item->setProperty("isLast", false);
                _items.append(item);
            }
        }
    } else {
        for(int j=0; j<i; j++){
            delete _items.takeLast();
        }
    }
    layout();
}

void ZPageView::refill()
{
    if(_cacheData.isNull() || !_cacheData.isValid()) return;
    this->parseAndFill();
}

void ZPageView::reused(bool reused)
{
    for(auto it=_items.begin(); it!=_items.end(); it++){
        if((*it)->isVisible())
            (*it)->setProperty("reused", reused);
    }
}

void ZPageView::layout()
{
    if(_items.isEmpty()) return;

    qreal xDist = _items.first()->width()+d->mColumnSpacing;
    qreal yDist = _items.first()->height()+d->mRowSpacing;
    qreal xOffset = (_page->width()-(xDist*d->mColumns-d->mColumnSpacing))/2.0;
    qreal yOffset = (_page->height()-(yDist*d->mRows-d->mRowSpacing))/2.0;
    for(int i=0; i<_items.count(); i++){
        auto item = _items.at(i);
        item->setX(i%d->mColumns*xDist + xOffset);
        item->setY(qFloor(i/d->mColumns)*yDist + yOffset);
    }
}

void ZPageView::preloadData(int pageIndex)
{
//    qDebug() << Q_FUNC_INFO << pageIndex << _pageIndex << _preloadPageIndex;
    _preloadPageIndex = pageIndex;
    d->mDtLd->search(_preloadPageIndex);
}

void ZPageView::handleDataContent(int pageIndex, const QVariant& datas)
{
    // 返回数据与当前页期待页不符
    if(pageIndex!=_pageIndex && pageIndex!=_preloadPageIndex)
        return;

//    qDebug() << Q_FUNC_INFO << "pageIndex"<< pageIndex << "_pageIndex" << _pageIndex << "_preloadPageIndex"<< _preloadPageIndex;

    _cacheData = datas.toJsonArray();
    // 非动画过程，刷新数据
    if(!d->mInInteractive){
        parseAndFill();
    }
}

void ZPageView::parseAndFill()
{
//    qDebug() << Q_FUNC_INFO << "1->" << _pageIndex << _preloadPageIndex;

    int i=0;
    const auto dy = _cacheData.toJsonArray();
    int c = std::min(dy.count(), _items.count());
    for(; i<c; i++){
        _items.at(i)->setProperty("isLast", false);
        _items.at(i)->setProperty("mdata", dy.at(i));
        _items.at(i)->setEnabled(true);
        _items.at(i)->setVisible(true);
    }
    if(i<_items.count())
        _items.at(i)->setProperty("isLast", true);
    for(; i<_items.count(); i++){
        _items.at(i)->setEnabled(false);
        _items.at(i)->setVisible(false);
    }

    _preloadPageIndex = -1;
    _cacheData.clear();
}
/**********************************************************************************
 * ******
 * ********************************************************************************/



/**********************************************************************************
 * ******
 * ********************************************************************************/
ZDataState::ZDataState(QObject *parent)
    :QObject{parent},
      kPageId{QString::number(QDateTime::currentMSecsSinceEpoch())}
{
    qDebug() << Q_FUNC_INFO;
}

ZDataState::~ZDataState()
{
    qDebug() << Q_FUNC_INFO;
}

bool ZDataState::isEmpty() const
{
    return _isEmpty;
}

int ZDataState::curPage() const
{
    return _curPage;
}

int ZDataState::totalPage() const
{
    return _totalPage;
}

void ZDataState::setCurPage(int v)
{
    if(_curPage==v) return;
    _curPage = v;
    emit curPageChanged();
}

void ZDataState::refresh(int reason)
{
    qDebug() << Q_FUNC_INFO << reason;
    // 在拖动动画过程中，不要去重置
    if(d->mInInteractive || d->mPages.isEmpty()) return;

    if(reason==0) {
        _curPage = 0;
        _totalPage = 0;
        _total = -1;
        ++_statusId;

        int nextPage = 1;
        int prePage = -1;

        d->mPages.at(1)->_pageIndex = _curPage;
        d->mPages.at(1)->_preloadPageIndex = _curPage;

        d->mPages.at(2)->_pageIndex = nextPage;
        d->mPages.at(2)->_preloadPageIndex = nextPage;

        d->mPages.at(0)->_pageIndex = prePage;
        d->mPages.at(0)->_preloadPageIndex = prePage;

        emit fetchDataContent(_curPage, d->pageSize(), _total, _totalPage, true, generatePageName(_curPage));
        emit fetchDataContent(nextPage, d->pageSize(), _total, _totalPage, false, generatePageName(nextPage));
    }
    else if (reason==1){
        int old = _totalPage;
        --_total;
        _totalPage = calculateTotalPage(_total);
        _isEmpty = _total<=0;

        // do nothing
        if(_isEmpty) {

        }
        // 如果页数没减少
        else if(_totalPage == old){
            // 将当前页更新
            d->mPages.at(1)->_pageIndex = _curPage;
            d->mPages.at(1)->preloadData(d->mPages.at(1)->_pageIndex);

            if(_totalPage>1){
                // 后一页
                d->mPages.at(2)->_pageIndex = _curPage==_totalPage-1?0:_curPage+1;
                d->mPages.at(2)->preloadData(d->mPages.at(2)->_pageIndex);

                if(_curPage==0){
                    // 前一页
                    d->mPages.at(0)->_pageIndex = _totalPage-1;
                    d->mPages.at(0)->preloadData(d->mPages.at(0)->_pageIndex);
                }
            }
        }
        // 如果页数减少了
        else {
            _curPage = _curPage==_totalPage?_curPage-1:_curPage;

            // 将当前页更新
            d->mPages.at(1)->_pageIndex = _curPage;
            d->mPages.at(1)->preloadData(d->mPages.at(1)->_pageIndex);

            if(_totalPage>1){
                // 后一页
                d->mPages.at(2)->_pageIndex = _curPage==_totalPage-1?0:_curPage+1;
                d->mPages.at(2)->preloadData(d->mPages.at(2)->_pageIndex);

                if(_curPage==0){
                    // 前一页
                    d->mPages.at(0)->_pageIndex = _totalPage-1;
                    d->mPages.at(0)->preloadData(d->mPages.at(0)->_pageIndex);
                }else if(_curPage==_totalPage-1){
                    // 前一页
                    d->mPages.at(0)->_pageIndex = _curPage-1;
                    d->mPages.at(0)->preloadData(d->mPages.at(0)->_pageIndex);
                }
            }
        }
    }

    emit isEmptyChanged();
    emit curPageChanged();
    emit totalPageChanged();
}

void ZDataState::updateTotal(int total, const QString& pageName)
{
    auto page_index = parsePageName(pageName);
    if(page_index<0) return;

    qDebug() << Q_FUNC_INFO << "pageName:" << pageName << "total:" << total;

    if(total<=0){
        _total = 0;
        _curPage = -1;
        _totalPage = 0;
        _isEmpty = true;
    }else{
        _total = total;
        _totalPage = calculateTotalPage(total);
        _isEmpty = _totalPage<=0;

        // 获取到总数后，去获取最后一页
        d->mPages.at(0)->_pageIndex = _totalPage-1;
        d->mPages.at(0)->preloadData(_totalPage-1);
    }

    emit curPageChanged();
    emit totalPageChanged();
    emit isEmptyChanged();
}

void ZDataState::updateDataContent(const QJSValue &datas, const QString &pageName)
{
    auto page_index = parsePageName(pageName);
    if(page_index<0) return;

    qDebug() << Q_FUNC_INFO << "pageName:" << pageName;

    for(auto it=d->mPages.begin(); it!=d->mPages.end(); it++)
        (*it)->handleDataContent(page_index, datas.toVariant());
}

bool ZDataState::checkPageName(const QString &pageName)
{
    auto page_index = parsePageName(pageName);
    return page_index>=0;
}

void ZDataState::search(int pageIndex)
{
    emit fetchDataContent(pageIndex, d->pageSize(), _total, _totalPage, false, generatePageName(pageIndex));
}


// kPageId:_statusId:pageIndex
const QString ZDataState::generatePageName(int pageIndex) const
{
    return QString("%1:%2:%3").arg(kPageId).arg(_statusId).arg(pageIndex);
}

int ZDataState::parsePageName(const QString &pageName)
{
    if(pageName.isEmpty()) return -1;
    auto r = pageName.split(':');
    if(r.count()<3) return -1;
    if(r.at(0)!=kPageId) return -1;
    if(r.at(1).toInt()!=_statusId) return -1;
    return r.at(2).toInt();
}

int ZDataState::calculateTotalPage(int total)
{
    if(total<=0) return 0;
    int tp;
    auto temp = qFloor(total/d->pageSize());
    if(total == 0){
        tp = 0;
    } else if(temp*d->pageSize()==total){
        tp = temp;
    } else {
        tp = temp+1;
    }
//    qDebug() << Q_FUNC_INFO << total << tp;
    return tp;
}
/**********************************************************************************
 * ******
 * ********************************************************************************/


/**********************************************************************************
 * ******
 * ********************************************************************************/
ZPageListViewPrivate::ZPageListViewPrivate(ZPageListView *q)
    :QObject{nullptr},
      q{q},
      mContentItem{new QQuickItem{q}}
{
    qDebug() << Q_FUNC_INFO;

    mContentItem->setParentItem(q);

    mTimeLine.setDuration(200);
    connect(&mTimeLine, &QTimeLine::valueChanged, this, &ZPageListViewPrivate::flickNext);

    // 过渡动画结束稍微延时，等待重布局完成
    mFlickEndTimer.setInterval(10);
    mFlickEndTimer.setSingleShot(true);
    connect(&mTimeLine, &QTimeLine::finished, this, &ZPageListViewPrivate::timeLineEnd);
    connect(&mFlickEndTimer, &QTimer::timeout, this, &ZPageListViewPrivate::flickEnd);
}

ZPageListViewPrivate::~ZPageListViewPrivate()
{
    qDebug() << Q_FUNC_INFO;
    qDeleteAll(mPages);
    mContentItem->deleteLater();
}

QQuickItem *ZPageListViewPrivate::createDelegateItem(QQuickItem* parentItem) const
{
    QQuickItem *item = nullptr;
    if(mDelegate){
        QObject *nobj = mDelegate->beginCreate(qmlContext(q));// mDelegate->creationContext()
        item = qobject_cast<QQuickItem*>(nobj);
        if(item){
            if(qFuzzyIsNull(item->z()))
                item->setZ(2);
            item->setVisible(false);
            item->setPosition(QPointF{0, 0});
            item->setParent(parentItem);
            item->setParentItem(parentItem);
            item->setFlags(QQuickItem::ItemHasContents);
        }
        mDelegate->completeCreate();
    }

    return item;
}

void ZPageListViewPrivate::createPageList()
{
//    qDebug() << Q_FUNC_INFO;
    // 创建page
    if(!mPages.isEmpty()){
        qDeleteAll(mPages);
        mPages.clear();
    }
    for(int i=0; i<3; i++){
        auto page = new ZPageView{this};
        mPages.append(page);
    }
}

int ZPageListViewPrivate::pageSize() const
{
    return mRows*mColumns;
}

qreal ZPageListViewPrivate::pageWidth() const
{
    return qFuzzyIsNull(mPageWidth)? q->width():mPageWidth;
}

qreal ZPageListViewPrivate::pageHeight() const
{
    return qFuzzyIsNull(mPageHeight)? q->height():mPageHeight;
}

QPointF ZPageListViewPrivate::contentMovePos(ZPageListViewPrivate::PageTurn pt) const
{
    qreal r = 0.5;
    if(pt==ZPageListViewPrivate::Next) r = 5.0/6.0;
    else if(pt==ZPageListViewPrivate::Prev) r = 1.0/6.0;
    qreal x = isHorizontal()? (r*3.0*pageWidth() - 0.5*q->width()):0.0;
    qreal y = isHorizontal()? 0.0:(r*3.0*pageHeight() - 0.5*q->height());
    return QPointF{-x, -y};
}

bool ZPageListViewPrivate::isVelocityOver(const QPointF &start, const QPointF &end, ulong deltaTs) const
{
    if(deltaTs>0){
        // 计算平均速度 pixel/ms
        QPointF avgVel = (end-start)/deltaTs;
        // 速度大于 判断为可翻页
        return (isHorizontal() && qAbs(avgVel.x())>=(PAGE_TURN_MVVEL_THRESHOLD*mDpW))
                || (!isHorizontal() && qAbs(avgVel.y())>=(PAGE_TURN_MVVEL_THRESHOLD*mDpH));
    }
    return false;
}

bool ZPageListViewPrivate::isMoveOver(const QPointF &start, const QPointF &end) const
{
    auto delta = end - start;
    return (isHorizontal() && qAbs(delta.x())>=(PAGE_TURN_DRAG_THRESHOLD*mDpW))
            || (!isHorizontal() && qAbs(delta.y())>=(PAGE_TURN_DRAG_THRESHOLD*mDpH));
}

void ZPageListViewPrivate::layout()
{
//    qDebug() << Q_FUNC_INFO;
    if(!mDtLd) return;
    if(mPageTurn==PageTurn::Next){
        mPages.move(0,2);
    }else if(mPageTurn==PageTurn::Prev){
        mPages.move(2,0);
    }

    mContentItem->setPosition(contentMovePos(ZPageListViewPrivate::None));
    mContentItem->setSize(QSizeF{isHorizontal()? (3.0*pageWidth()):pageWidth(),
                                  isHorizontal()? pageHeight():(3.0*pageHeight())});

    if(mPages.count()==3){
        mPages.at(0)->_page->setPosition(QPointF{0, 0});
        mPages.at(0)->layout();
        // 将不可见页隐藏
//        mDtLd->mPages.at(0)->mPage->setVisible(false);

        mPages.at(1)->_page->setPosition(QPointF{isHorizontal()?pageWidth():0.0, isHorizontal()?0.0:pageHeight()});
        mPages.at(1)->layout();
        // 将不可见页隐藏
//        mDtLd->mPages.at(1)->mPage->setVisible(true);

        mPages.at(2)->_page->setPosition(QPointF{isHorizontal()?2*pageWidth():0.0, isHorizontal()?0.0:2*pageHeight()});
        mPages.at(2)->layout();
        // 将不可见页隐藏
//        mDtLd->mPages.at(2)->mPage->setVisible(false);

        // 翻页完成，更新为当前页
        mDtLd->setCurPage(mPages.at(1)->_pageIndex);

        // 如果等于2页，使前后两页相同
//        if(mDtLd->totalPage()==2){
//            if(mPageTurn==PageTurn::Next){
//                mPages.at(2)->_pageIndex = mPages.at(2)->_preloadPageIndex = mDtLd->_curPage==0?1:0;
//                mPages.at(2)->handleDataContent(mPages.at(2)->_pageIndex, mPages.at(0)->_originalData);
//            }else if(mPageTurn==PageTurn::Prev){
//                mPages.at(0)->_pageIndex = mPages.at(0)->_preloadPageIndex = mDtLd->_curPage==0?1:0;
//                mPages.at(0)->handleDataContent(mPages.at(0)->_pageIndex, mPages.at(2)->_originalData);
//            }
//        }
    }
}

void ZPageListViewPrivate::turnPage(bool isDown)
{
//    qDebug() << Q_FUNC_INFO;
    if(mInflick || mInInteractive || !mDtLd) return;

    // 少于2页，不需要翻页；或是还未获取到总数，不允许翻页
    if(mDtLd->totalPage()<=1) return;

    if(mPages.count()==3){
        mPages.at(0)->_page->setVisible(true);
        mPages.at(2)->_page->setVisible(true);
    }

    auto to = isDown? contentMovePos(ZPageListViewPrivate::Next) : contentMovePos(ZPageListViewPrivate::Prev);
    mPageTurn = isDown? ZPageListViewPrivate::PageTurn::Next: ZPageListViewPrivate::PageTurn::Prev;

    preloadData(isDown? FetchMode::OnlyNext:FetchMode::OnlyPre);

    flickStart(to);
}

void ZPageListViewPrivate::handleMousePressEvent(QMouseEvent *event)
{
    // 正在移动中
    if(mInflick) return;

    // 拖动时显示隐藏的不可见页
//    if(mDtLd->mPages.count()==3){
//        if(mDtLd->totalPage()>1){
//            mDtLd->mPages.at(0)->mPage->setVisible(true);
//            mDtLd->mPages.at(2)->mPage->setVisible(true);
//        }
//    }

    mInInteractive = true;
    mPressPos = event->localPos();
    mPressTs = event->timestamp();
    mLastMovePos = event->localPos();
}


void ZPageListViewPrivate::handleMouseMoveEvent(QMouseEvent *event)
{
    // 正在移动中
    if(mInflick) return;

    // 用来预加载数据
    if(!mIsFirstMove){
        // TODO？？是否允许移动
        mIsFirstMove = true;
    }

    // 移动到一定阈值再steal事件，调整触摸灵敏度
    if(!mStealMouse){
        auto delta = event->localPos()-mPressPos;
        mStealMouse = (isHorizontal() && (qAbs(delta.x())>PAGE_STEAL_MOUSE_THRESHOLD*mDpW))
                || (!isHorizontal() && (qAbs(delta.y())>PAGE_STEAL_MOUSE_THRESHOLD*mDpH));
    }

    QPointF to = contentMovePos(ZPageListViewPrivate::None);
    isHorizontal()? to.setX(to.x()+(event->localPos().x()-mPressPos.x()))
                  : to.setY(to.y()+(event->localPos().y()-mPressPos.y()));

    move(to);
}

void ZPageListViewPrivate::handleMouseReleaseEvent(QMouseEvent *event)
{
    // 正在移动中
    if(mInflick) return;

    // 是否速度超过阈值
    bool hasVel = isVelocityOver(mPressPos, event->localPos(), event->timestamp()-mPressTs);

    // 并没有被移动过，以及按下与释放之间速度未超过阈值
    if(!mIsFirstMove && !hasVel) {
        resetState();
        return;
    }    

    // 是否移动距离超过阈值
    bool hasMvOver = isMoveOver(mPressPos, event->localPos());

    // 是否翻页，交互阈值超限，以及页数大于1
    bool isTurn = mDtLd->totalPage()>1 && (hasVel || hasMvOver);

    // 是否翻下页
    bool isNext = (isHorizontal() && (event->localPos().x()-mPressPos.x())<0)
            || (!isHorizontal() && (event->localPos().y()-mPressPos.y())<0);

    // 翻页
    QPointF to;
    if(isTurn){
        to = isNext? contentMovePos(ZPageListViewPrivate::Next) : contentMovePos(ZPageListViewPrivate::Prev);
    }
    // 不翻页，回到原位置
    else{
        to = contentMovePos(ZPageListViewPrivate::None);
    }

    mPageTurn = (!isTurn)? ZPageListViewPrivate::PageTurn::None:
                            isNext?ZPageListViewPrivate::PageTurn::Next:
                                   ZPageListViewPrivate::PageTurn::Prev;

    // 如果翻页则预加载
    if(isTurn)
        preloadData(isNext?FetchMode::OnlyNext:FetchMode::OnlyPre);

    flickStart(to);
}

void ZPageListViewPrivate::preloadData(FetchMode fm)
{
//    qDebug() << Q_FUNC_INFO << fm;
    if(mPages.count()<3 || !mDtLd || mDtLd->totalPage()<=1) return;

    // 如果少于3页，不加载数据，由layout去处理
//    if(mDtLd->totalPage()<=3) return;

    switch (fm) {
    case FetchMode::OnlyNext:{
        int p = mPages.at(2)->_pageIndex+1;
        p = (p>=mDtLd->totalPage())?0:p;
        mPages.at(0)->preloadData(p);
        return;
    }
    case FetchMode::OnlyPre:{
        int p = mPages.at(0)->_pageIndex-1;
        p = p<0?(mDtLd->totalPage()-1):p;
        mPages.at(2)->preloadData(p);
        return;
    }
    default:
        return;
    }
}

void ZPageListViewPrivate::move(const QPointF &toPos)
{
//    qDebug() << Q_FUNC_INFO << toPos;
    if(toPos.isNull()) return;
    if(isHorizontal()){
        mContentItem->setX(toPos.x());
    }else{
        mContentItem->setY(toPos.y());
    }
    q->polish();
}

void ZPageListViewPrivate::flickStart(const QPointF& to)
{
//    qDebug() << Q_FUNC_INFO << to;

    mFlickFromPos = mContentItem->position();
    mFlickToPos = to;

    mInflick = true;
    mInInteractive = true;

    if(mPageTurn && mDtLd){
        if(mPageTurn==ZPageListViewPrivate::Next){
            mPages.at(0)->_pageIndex = (mPages.at(2)->_pageIndex+1)>=mDtLd->totalPage()?0:mPages.at(2)->_pageIndex+1;
        }else if(mPageTurn==ZPageListViewPrivate::Prev){
            mPages.at(2)->_pageIndex = (mPages.at(0)->_pageIndex-1)<0?mDtLd->totalPage()-1:mPages.at(0)->_pageIndex-1;
        }
        mTimeLine.start();
    }else{
        move(to);
        flickEnd();
    }
}

void ZPageListViewPrivate::flickNext(qreal timeLineValue)
{
    QPointF to = mFlickFromPos+(mFlickToPos-mFlickFromPos)*timeLineValue;
    move(to);
}

void ZPageListViewPrivate::timeLineEnd()
{
    layout();
    q->polish();
    mFlickEndTimer.start();
}

void ZPageListViewPrivate::flickEnd()
{
    if(mPages.count()>=3){
        mPages.at(0)->refill();
        mPages.at(1)->refill();
        mPages.at(2)->refill();
    }
    resetState();
}

void ZPageListViewPrivate::resetState()
{
    q->ungrabMouse();
    q->setKeepMouseGrab(false);
    mPageTurn = PageTurn::None;
    mIsFirstMove = false;
    mInInteractive = false;
    mStealMouse = false;
    mInflick = false;
}
/**********************************************************************************
 * ******
 * ********************************************************************************/




/**********************************************************************************
 * ******
 * ********************************************************************************/
ZPageListView::ZPageListView(QQuickItem *parent)
    :QQuickItem{parent},
      d{new ZPageListViewPrivate{this}}
{
    qDebug() << Q_FUNC_INFO;

    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(false); // rely on mouse events synthesized from touch
    setFiltersChildMouseEvents(true);
}

ZPageListView::~ZPageListView()
{
    qDebug() << Q_FUNC_INFO;
    delete d;
}

QQmlComponent *ZPageListView::delegate() const
{
    return d->mDelegate;
}

void ZPageListView::setDelegate(QQmlComponent *comp)
{
    if(comp == d->mDelegate)
        return;
    d->mDelegate = comp;

    emit delegateChanged();
}

ZDataState* ZPageListView::dataState() const
{
    return d->mDtLd;
}

void ZPageListView::setDataState(ZDataState* v)
{
//    qDebug() << Q_FUNC_INFO;
    if(v==d->mDtLd) return;
    v->d = d;
    d->mDtLd = v;
    if(this->isComponentComplete()){
        d->layout();
//        v->refresh(0);
    }
    emit dataStateChanged();
}

QQmlComponent *ZPageListView::highlight() const
{
    return d->mHighlight;
}

void ZPageListView::setHighlight(QQmlComponent *v)
{
    if(!v || v==d->mHighlight)
        return;
    d->mHighlight = v;
    emit highlightChanged();
}

Qt::Orientation ZPageListView::orientation() const
{
    return d->mOrientation;
}

void ZPageListView::setOrientation(Qt::Orientation v)
{
    if(v == d->mOrientation)
        return;
    d->mOrientation = v;
    emit orientationChanged();
}

qreal ZPageListView::pageWidth() const
{
    return d->mPageWidth;
}

void ZPageListView::setPageWidth(qreal v)
{

    if(qFuzzyCompare(v,d->mPageWidth) || v<0)
        return;
    d->mPageWidth = v;
    d->layout();
    emit pageWidthChanged();
}

qreal ZPageListView::pageHeight() const
{
    return d->mPageHeight;
}

void ZPageListView::setPageHeight(qreal v)
{
    if(qFuzzyCompare(v,d->mPageHeight) || v<0)
        return;
    d->mPageHeight = v;
    d->layout();
    emit pageHeightChanged();
}

int ZPageListView::rows() const
{
    return d->mRows;
}

void ZPageListView::setRows(int v)
{
    if(v==d->mRows || v<0)
        return;
    d->mRows = v;
    for(auto it=d->mPages.begin(); it!=d->mPages.end(); it++)
        (*it)->resize();
    d->layout();
    polish();

    emit rowsChanged();
}

int ZPageListView::columns() const
{
    return d->mColumns;
}

void ZPageListView::setColumns(int v)
{
    if(v==d->mColumns || v<0)
        return;
    d->mColumns = v;
    for(auto it=d->mPages.begin(); it!=d->mPages.end(); it++)
        (*it)->resize();
    d->layout();
    polish();
    emit columnsChanged();
}

qreal ZPageListView::rowSpacing() const
{
    return d->mRowSpacing;
}

void ZPageListView::setRowSpacing(qreal v)
{
    if(qFuzzyCompare(v, d->mRowSpacing) || v<0)
        return;
    d->mRowSpacing = v;
    emit rowSpacingChanged();
}

qreal ZPageListView::columnSpacing() const
{
    return d->mColumnSpacing;
}

void ZPageListView::setColumnSpacing(qreal v)
{
    if(qFuzzyCompare(v, d->mColumnSpacing) || v<0)
        return;
    d->mColumnSpacing = v;
    emit columnSpacingChanged();
}

int ZPageListView::flickDuration() const
{
    return d->mTimeLine.duration();
}

void ZPageListView::setFlickDuration(int v)
{
    if(d->mTimeLine.state() != QTimeLine::NotRunning || v==d->mTimeLine.duration() || v<0)
        return;

    d->mTimeLine.setDuration(v);
    emit flickDurationChanged();
}

qreal ZPageListView::dpw()
{
    return d->mDpW;
}

void ZPageListView::setDpw(qreal v)
{
    if(qFuzzyCompare(v, d->mDpW) || v<0) return;
    d->mDpW = v;
    emit dpwChanged();
}

qreal ZPageListView::dph()
{
    return d->mDpH;
}

void ZPageListView::setDph(qreal v)
{
    if(qFuzzyCompare(v, d->mDpH) || v<0) return;
    d->mDpH = v;
    emit dphChanged();
}

void ZPageListView::pageUp()
{
    d->turnPage(false);
}

void ZPageListView::pageDown()
{
    d->turnPage(true);
}

bool ZPageListView::childMouseEventFilter(QQuickItem *i, QEvent *e)
{
    if(!isVisible()){
        return QQuickItem::childMouseEventFilter(i, e);
    }

    switch (e->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:{
        return filterMouseEvent(i, static_cast<QMouseEvent *>(e));
    }
    case QEvent::UngrabMouse:{
//        qDebug() << Q_FUNC_INFO << "UngrabMouse";
//        if (d->window && d->window->mouseGrabberItem() && d->window->mouseGrabberItem() != this) {
//            // The grab has been taken away from a child and given to some other item.
//            mouseUngrabEvent();
//        }
        break;
    }
    default:
        break;
    }
    return QQuickItem::childMouseEventFilter(i, e);
}

void ZPageListView::mousePressEvent(QMouseEvent *event)
{
    d->handleMousePressEvent(event);
    event->accept();
}

void ZPageListView::mouseMoveEvent(QMouseEvent *event)
{
    d->handleMouseMoveEvent(event);
    event->accept();
}

void ZPageListView::mouseReleaseEvent(QMouseEvent *event)
{
    d->handleMouseReleaseEvent(event);
    event->accept();
}

void ZPageListView::wheelEvent(QWheelEvent *event)
{
    QQuickItem::wheelEvent(event);
    event->accept();
}

void ZPageListView::timerEvent(QTimerEvent *event)
{
}

bool ZPageListView::filterMouseEvent(QQuickItem *receiver, QMouseEvent *event)
{
    QPointF localPos = mapFromScene(event->windowPos());
    Q_ASSERT_X(receiver != this, "", "ZPageListView received a filter event for itself");
    if (receiver == this && d->mStealMouse) {
        // we are already the grabber and we do want the mouse event to ourselves.
        return true;
    }

    bool stealThisEvent = d->mStealMouse;

    QScopedPointer<QMouseEvent> mouseEvent(
                new QMouseEvent(
                    event->type(), localPos, event->windowPos(), event->screenPos(),
                    event->button(), event->buttons(), event->modifiers()
                    ));
    mouseEvent->setAccepted(false);

    switch (event->type()) {
    case QEvent::MouseButtonPress:{
        d->handleMousePressEvent(mouseEvent.data());
        stealThisEvent = false;
        break;
    }
    case QEvent::MouseMove:
    {
        d->handleMouseMoveEvent(mouseEvent.data());
        stealThisEvent = d->mStealMouse;
        break;
    }
    case QEvent::MouseButtonRelease:{
        d->handleMouseReleaseEvent(mouseEvent.data());
        stealThisEvent = d->mStealMouse;
        break;
    }
    default: break;
    }

    if(stealThisEvent){
        grabMouse();
        setKeepMouseGrab(true);
    }

    return stealThisEvent;
}

void ZPageListView::mouseUngrabEvent()
{
    QQuickItem::mouseUngrabEvent();
}

void ZPageListView::updatePolish()
{
    QQuickItem::updatePolish();
}

void ZPageListView::componentComplete()
{
    qDebug() << Q_FUNC_INFO;
    QQuickItem::componentComplete();
    d->createPageList();
    d->layout();
//    if(d->mDtLd)
//        d->mDtLd->refresh(0);
}

void ZPageListView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}
/**********************************************************************************
 * ******
 * ********************************************************************************/








