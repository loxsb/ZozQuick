#pragma once


#include <QObject>
#include <QQmlEngine>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>


class ZHttpRequest: public QObject
{
    Q_OBJECT
public:
    inline ZHttpRequest(QObject* parent=nullptr);
    inline ZHttpRequest(QNetworkAccessManager::Operation op, QObject *parent=nullptr);
    inline ~ZHttpRequest();

//    inline Q_INVOKABLE ZHttpRequest* verb(const QString& verb);
    inline Q_INVOKABLE ZHttpRequest* url(const QString& url);

    // get
    inline Q_INVOKABLE ZHttpRequest* queryParam(const QVariantMap& json);

    // post json body
    inline Q_INVOKABLE ZHttpRequest* bodyJson(const QVariantMap& json);

    // on success
    inline Q_INVOKABLE ZHttpRequest* onSuccess(const QJSValue& callback);

    // on failed
    inline Q_INVOKABLE ZHttpRequest* onFailed(const QJSValue& callback);

    // send request
    inline Q_INVOKABLE void send();

    enum BodyType
    {
        None = 0,              // This request does not have a body.
//        Raw,
        Raw_Json,              // application/json
//        X_Www_Form_Urlencoded, // x-www-form-urlencoded
//        FileMap,               // multipart/form-data
//        MultiPart,             // multipart/form-data
//        FormData               // multipart/form-data
    };

private:
    inline static QNetworkAccessManager* manager();
    inline void replyError(QNetworkReply::NetworkError code);
    inline void replyFinish();

private:
    QNetworkAccessManager::Operation _op;
    QNetworkRequest _request;
    QPair<BodyType, QVariant> _body = qMakePair(BodyType::None, QByteArray());
    QNetworkReply* _reply=nullptr;

    QJSValue _hSuccess;
    QJSValue _hFailed;
};


// for qml use
class ZHttp: public QObject
{
    Q_OBJECT
public:
    inline ZHttp(QObject *parent=nullptr);
    inline ~ZHttp();

    inline Q_INVOKABLE ZHttpRequest* get(const QString& url);
    inline Q_INVOKABLE ZHttpRequest* post(const QString& url);
};


static void registerHttpToQml(char* comp) {
    qmlRegisterType<ZHttpRequest>(comp, 1, 0, "ZHttpRequest");
    qmlRegisterSingletonType<ZHttp>(comp, 1, 0, "ZHttp", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        ZHttp *example = new ZHttp(engine);
        return example;
    });
}


inline ZHttpRequest::ZHttpRequest(QObject *parent)
    : QObject{parent}
{
}

inline ZHttpRequest::ZHttpRequest(QNetworkAccessManager::Operation op, QObject *parent)
    : QObject{parent},
      _op{op}
{
    qDebug() << Q_FUNC_INFO;
}

inline ZHttpRequest::~ZHttpRequest()
{
    qDebug() << Q_FUNC_INFO;
    if(_reply){
        if(_reply->isRunning())
            _reply->abort();
    }
}

inline ZHttpRequest *ZHttpRequest::url(const QString &url)
{
    _request.setUrl(url);
    return this;
}

inline ZHttpRequest *ZHttpRequest::queryParam(const QVariantMap &json)
{
    QUrl url(_request.url());
    QUrlQuery urlQuery(url);
    auto iter = json.constBegin();
    while (iter != json.constEnd()) {
        urlQuery.addQueryItem(iter.key(), iter.value().toString());
        ++iter;
    }
    url.setQuery(urlQuery);
    _request.setUrl(url);
    return this;
}

inline ZHttpRequest *ZHttpRequest::bodyJson(const QVariantMap &json)
{
    const QByteArray &value = QJsonDocument(QJsonObject::fromVariantMap(json)).toJson();
    _body = qMakePair(Raw_Json, value);
    _request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return this;
}

inline ZHttpRequest *ZHttpRequest::onSuccess(const QJSValue &callback)
{
    if(callback.isCallable())
        _hSuccess = callback;
    return this;
}

inline ZHttpRequest *ZHttpRequest::onFailed(const QJSValue &callback)
{
    if(callback.isCallable())
        _hFailed = callback;
    return this;
}

inline void ZHttpRequest::send()
{
    switch(_op){
    case QNetworkAccessManager::Operation::GetOperation: {
        _reply = manager()->get(_request);
        break;
    }
    case QNetworkAccessManager::Operation::PostOperation: {
        switch(_body.first){
        case ZHttpRequest::Raw_Json: {
            _reply = manager()->post(_request, _body.second.toByteArray());
            break;
        }
        case ZHttpRequest::None:
        default:
            _reply = nullptr;
        }
        break;
    }
    default:
        _reply = nullptr;
        break;
    }

    if(_reply==nullptr) {
        this->deleteLater();
        return;
    }

    _reply->setParent(this);
    connect(_reply, &QNetworkReply::finished, this, &ZHttpRequest::replyFinish);
    connect(_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &ZHttpRequest::replyError);
}

inline QNetworkAccessManager *ZHttpRequest::manager()
{
    static QNetworkAccessManager manager;
    return &manager;
}

inline void ZHttpRequest::replyError(QNetworkReply::NetworkError code)
{
    if(code!=QNetworkReply::NoError && _hFailed.isCallable()) {
        _hFailed.call({code, _reply->errorString()});
    }
}

inline void ZHttpRequest::replyFinish()
{
    if(_reply->error()==QNetworkReply::NoError && _hSuccess.isCallable()) {
        _hSuccess.call({QString(_reply->readAll())});
    }
    this->deleteLater();
}

inline ZHttp::ZHttp(QObject *parent)
{
    qDebug() << Q_FUNC_INFO;
}

inline ZHttp::~ZHttp()
{
    qDebug() << Q_FUNC_INFO;
}

inline ZHttpRequest* ZHttp::get(const QString &url)
{
    ZHttpRequest* req = new ZHttpRequest{QNetworkAccessManager::GetOperation, nullptr};
    req->url(url);
    return req;
}

inline ZHttpRequest *ZHttp::post(const QString &url)
{
    ZHttpRequest* req = new ZHttpRequest{QNetworkAccessManager::PostOperation, nullptr};
    req->url(url);
    return req;
}

