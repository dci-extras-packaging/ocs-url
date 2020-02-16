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
#include <QFileInfoList>

namespace Qtil {

class Dir : public QObject
{
    Q_OBJECT

public:
    explicit Dir(const QString &path = QString(), QObject *parent = nullptr);

    Dir(const Dir &other, QObject *parent = nullptr);
    Dir &operator =(const Dir &other);

    QString path() const;
    void setPath(const QString &path);

    bool exists() const;
    QFileInfoList list() const;
    bool make() const;
    bool copy(const QString &newPath) const;
    bool move(const QString &newPath) const;
    bool remove() const;

    static QString rootPath();
    static QString tempPath();
    static QString homePath();
    static QString genericDataPath();
    static QString genericConfigPath();
    static QString genericCachePath();
    static QString kdehomePath();

private:
    bool copyRecursively(const QString &srcPath, const QString &newPath) const;

    QString path_;
};

}
