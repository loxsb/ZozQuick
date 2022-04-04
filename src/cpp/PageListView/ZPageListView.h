#pragma once

#include <QJsonArray>
#include <QQuickItem>
#include <QTimeLine>
#include <QTimer>
#include <memory>


class ZDataState;
class ZPageView;
class ZPageListView;
class ZPageListViewPrivate;



/***********************************************************************
 * ***** Begin of ZPageView
 **********************************************************************/
class ZPageView:public QObject
{
    Q_OBJECT
public:
    ZPageView(ZPageListViewPrivate*);
    ~ZPageView();

    // 页面item数量发生变化
    void resize();
    // 填充数据
    void refill();
    // 页面被复用
    void reused(bool reused);
    // 间距发生变化，重新布局
    void layout();
    // 预获取该页的趋向页数据
    void preloadData(int pageIndex);
    // 处理获取到的数据
    void handleDataContent(int pageIndex, const QVariant& datas);

private:
    void parseAndFill();

private:
    friend class ZPageListViewPrivate;
    friend class ZDataState;

    ZPageListViewPrivate* d=nullptr;
    QQuickItem* _page=nullptr;

    QVector<QQuickItem*> _items;
    QVariant _cacheData; // toJsonArray

    int _pageIndex=-1; // page当前页
    int _preloadPageIndex=-1; // 趋向页
};
/***********************************************************************
 * ***** End of ZPageView
 **********************************************************************/



/***********************************************************************
 * ***** Begin of DataLoader
 **********************************************************************/
class ZDataState: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(int curPage READ curPage NOTIFY curPageChanged)
    Q_PROPERTY(int totalPage READ totalPage NOTIFY totalPageChanged)

public:
    ZDataState(QObject* parent=nullptr);
    ~ZDataState();

    bool isEmpty() const;
    int curPage() const;
    int totalPage() const;

    void setCurPage(int v);

    // 是否就地更新，then 重新搜索
    // reason==0  过滤条件变化需要刷新
    // reason==1  删除一项需要就地刷新
    Q_INVOKABLE void refresh(int reason);
    // 更新总数
    Q_INVOKABLE void updateTotal(int total, const QString& pageName);
    // 更新一页数据内容
    Q_INVOKABLE void updateDataContent(const QJSValue& data, const QString& pageName);
    // 检查pagename
    Q_INVOKABLE bool checkPageName(const QString& pageName);

signals:
    void isEmptyChanged();
    void curPageChanged();
    void totalPageChanged();

    // 通知获取页面数据内容
    void fetchDataContent(int pageIndex, int countPerPage, int total, int totalPage, bool isGetTotal, const QString& pageName);

private:
    void search(int pageIndex);

    const QString generatePageName(int pageIndex) const; // kPageId:_statusId:pageIndex
    int parsePageName(const QString& pageName); // return pageIndex <0=>error
    int calculateTotalPage(int total); // 计算总页数

private:
    friend class ZPageListViewPrivate;
    friend class ZPageView;
    friend class ZPageListView;

    ZPageListViewPrivate* d=nullptr;

    bool _isEmpty=true;
    int _curPage=-1; // 当前页码
    int _totalPage=0; // 总页数
    int _total=-1; // 搜索条件下总数

    int _statusId=0; // 搜索条件变化 +1 (each reset +1)
    const QString kPageId; // 类实例唯一id
};
/***********************************************************************
 * ***** End of DataLoader
 **********************************************************************/




/***********************************************************************
 * ***** Begin of ZPageListViewPrivate
 **********************************************************************/
class ZPageListViewPrivate: public QObject {
    Q_OBJECT
public:
    ZPageListViewPrivate(ZPageListView* q);
    ~ZPageListViewPrivate();

    enum PageTurn { None=0, Prev, Next };
    enum FetchMode { OnlyReset=0, OnlyNext, OnlyPre};

    QQuickItem *createDelegateItem(QQuickItem* parentItem) const;
    void createPageList();

    int pageSize() const;
    qreal pageWidth() const;
    qreal pageHeight() const;
    QPointF contentMovePos(PageTurn) const;
    // 水平
    bool isHorizontal() const { return mOrientation==Qt::Horizontal; }
    // 速度
    bool isVelocityOver(const QPointF& start, const QPointF& end, ulong deltaTs) const;
    // 移动
    bool isMoveOver(const QPointF& start, const QPointF& end) const;

    void layout();
    void turnPage(bool isDown);

    void handleMousePressEvent(QMouseEvent *);
    void handleMouseMoveEvent(QMouseEvent *);
    void handleMouseReleaseEvent(QMouseEvent *);

private:
    void preloadData(FetchMode fm);

    void move(const QPointF& toPos);
    void flickStart(const QPointF& to);
    void flickNext(qreal timeLineValue);
    void timeLineEnd();
    void flickEnd();
    void resetState();

public:
    ZPageListView* q=nullptr;
    QVector<ZPageView*> mPages;

    QTimeLine mTimeLine;
    QTimer mFlickEndTimer;
    QQuickItem* mContentItem=nullptr;

    /*****/
    PageTurn mPageTurn=PageTurn::None;
    bool mIsFirstMove = false;
    bool mStealMouse = false;
    bool mInInteractive = false;
    bool mInflick = false;
    ulong mPressTs=0;
    QPointF mPressPos;
    QPointF mLastMovePos;
    QPointF mFlickFromPos;
    QPointF mFlickToPos;
    /*****/

    /*****/
    QQmlComponent* mDelegate=nullptr;
    ZDataState* mDtLd=nullptr;
    QQmlComponent *mHighlight=nullptr;
    /**/
    Qt::Orientation mOrientation=Qt::Horizontal;
    qreal mPageWidth=0;
    qreal mPageHeight=0;
    int mRows=0;
    int mColumns=0;
    qreal mRowSpacing=0;
    qreal mColumnSpacing=0;
    qreal mDpW=1.0;
    qreal mDpH=1.0;
    /*****/
};
/***********************************************************************
 * ***** End of ZPageListViewPrivate
 **********************************************************************/



/***********************************************************************
 * ***** Begin of ZPageListView
 **********************************************************************/
class ZPageListView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(ZDataState* dataState READ dataState WRITE setDataState NOTIFY dataStateChanged)
    Q_PROPERTY(QQmlComponent *highlight READ highlight WRITE setHighlight NOTIFY highlightChanged)

    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(qreal pageWidth READ pageWidth WRITE setPageWidth NOTIFY pageWidthChanged)
    Q_PROPERTY(qreal pageHeight READ pageHeight WRITE setPageHeight NOTIFY pageHeightChanged)
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(qreal rowSpacing READ rowSpacing WRITE setRowSpacing NOTIFY rowSpacingChanged)
    Q_PROPERTY(qreal columnSpacing READ columnSpacing WRITE setColumnSpacing NOTIFY columnSpacingChanged)
    Q_PROPERTY(int flickDuration READ flickDuration WRITE setFlickDuration NOTIFY flickDurationChanged)
    Q_PROPERTY(qreal dpw READ dpw WRITE setDpw NOTIFY dpwChanged)
    Q_PROPERTY(qreal dph READ dph WRITE setDph NOTIFY dphChanged)

public:
    ZPageListView(QQuickItem *parent = nullptr);
    ~ZPageListView();

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *);

    ZDataState* dataState() const;
    void setDataState(ZDataState* v);

    QQmlComponent *highlight() const;
    void setHighlight(QQmlComponent *v);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation v);

    qreal pageWidth() const;
    void setPageWidth(qreal v);

    qreal pageHeight() const;
    void setPageHeight(qreal v);

    int rows() const;
    void setRows(int v);

    int columns() const;
    void setColumns(int v);

    qreal rowSpacing() const;
    void setRowSpacing(qreal v);

    qreal columnSpacing() const;
    void setColumnSpacing(qreal v);

    int flickDuration() const;
    void setFlickDuration(int v);

    qreal dpw();
    void setDpw(qreal v);

    qreal dph();
    void setDph(qreal v);

    Q_INVOKABLE void pageUp();
    Q_INVOKABLE void pageDown();

signals:
    void delegateChanged();
    void dataStateChanged();
    void highlightChanged();
    void orientationChanged();
    void pageWidthChanged();
    void pageHeightChanged();
    void rowsChanged();
    void columnsChanged();
    void rowSpacingChanged();
    void columnSpacingChanged();
    void flickDurationChanged();
    void pageItemTypeChanged();
    void dpwChanged();
    void dphChanged();

protected:
    bool childMouseEventFilter(QQuickItem *, QEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    bool filterMouseEvent(QQuickItem *receiver, QMouseEvent *event);
    void mouseUngrabEvent() override;

protected:
    void updatePolish() override;
    void componentComplete() override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    ZPageListViewPrivate *d=nullptr;
};
/***********************************************************************
 * ***** End of ZPageListView
 **********************************************************************/





