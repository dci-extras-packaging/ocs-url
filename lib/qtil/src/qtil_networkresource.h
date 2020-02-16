/**
 * Qtil
 *
 * @author      Akira Ohgaki <akiraohgaki@gmail.com>
 * @copyright   2016, Akira Ohgaki
 * @license     https://opensource.org/licenses/LGPL-3.0
 * @link        https://github.com/akiraohgaki/qtil
 */

#pragma once

#include <QObject>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Qtil {

class NetworkResource : public QObject
{
    Q_OBJECT

public:
    explicit NetworkResource(const QString &id = QString(), const QUrl &url = QUrl(), bool async = true, QObject *parent = nullptr);
    ~NetworkResource();

    NetworkResource(const NetworkResource &other, QObject *parent = nullptr);
    NetworkResource &operator =(const NetworkResource &other);

    QString id() const;
    void setId(const QString &id);
    QUrl url() const;
    void setUrl(const QUrl &url);
    bool async() const;
    void setAsync(bool async);
    QNetworkRequest request() const;
    void setRequest(const QNetworkRequest &request);
    QNetworkAccessManager *manager() const;
    QNetworkReply *reply() const;
    QString method() const;
    QString contentType() const;
    QByteArray contentData() const;

    NetworkResource *head();
    NetworkResource *get();
    NetworkResource *post(const QByteArray &contentData, const QString &contentType);
    NetworkResource *post(const QUrlQuery &contentData);
    NetworkResource *put(const QByteArray &contentData, const QString &contentType);
    NetworkResource *put(const QUrlQuery &contentData);
    NetworkResource *deleteResource();
    bool isFinishedWithNoError() const;
    QByteArray readData() const;
    bool saveData(const QString &path) const;

signals:
    void finished(NetworkResource *resource);
    void downloadProgress(QString id, qint64 bytesReceived, qint64 bytesTotal);
    void uploadProgress(QString id, qint64 bytesSent, qint64 bytesTotal);

public slots:
    void abort();

private slots:
    void replyFinished();
    void replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void replyUploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    void setManager(QNetworkAccessManager *manager);
    void setReply(QNetworkReply *reply);
    void setMethod(const QString &method);
    void setContentType(const QString &contentType);
    void setContentData(const QByteArray &contentData);

    NetworkResource *send(const QUrl &url, bool async);

    QString id_;
    QUrl url_;
    bool async_;
    QNetworkRequest request_;
    QNetworkAccessManager *manager_;
    QNetworkReply *reply_;
    QString method_;
    QString contentType_;
    QByteArray contentData_;
};

}
