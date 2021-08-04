#ifndef RECENT_H
#define RECENT_H

#include <QPixmap>

class RecentFile
{
public:
    RecentFile(const QString &filename, const QPixmap &thumbnail);
    const QString &filename() const;
    const QPixmap &thumbnail() const;
    bool operator==(const RecentFile &) const;

private:
    class RecentFilePrivate : public QSharedData
    {
    public:
        RecentFilePrivate(const QString &filename, const QPixmap &thumbnail) : filename(filename), thumbnail(thumbnail) {}
        const QString filename;
        const QPixmap thumbnail;
    };

    QSharedDataPointer<RecentFilePrivate> d;
};

class Recent : public QObject
{
public:
    Recent(const QString &identifier, int limit = 10, QObject *parent = nullptr);

    bool add(const QString &filename, const QPixmap &thumbnail);
    bool remove(int index);
    void clear();

    void setLimit(int limit);

    const QList<RecentFile> &all() const;
    QStringList filenames() const;
    QList<QPixmap> thumbnails() const;

private:
    void saveToFile() const;
    QString thumbnailPath(const QString &filename) const;
    QString hash(const QString &string) const;

    QList<RecentFile> recent;
    QString identifier;
    QString thumbsPath;
    int limit;
};

#endif // RECENT_H
