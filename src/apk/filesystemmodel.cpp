#include "apk/filesystemmodel.h"
#include "apk/resourcemodelindex.h"
#include <QTimer>

#ifdef QT_DEBUG
    #include <QDebug>
#endif

void FileSystemModel::setSourceModel(ResourceItemsModel *model)
{
    if (sourceModel) {
        disconnect(sourceModel, &ResourceItemsModel::dataChanged, this, nullptr);
    }
    sourceModel = model;
    if (model) {
        connect(model, &ResourceItemsModel::dataChanged, this,
                [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
            const auto fromIndex = index(ResourceModelIndex(topLeft).path());
            const auto toIndex = index(ResourceModelIndex(bottomRight).path());
            updated(fromIndex.sibling(fromIndex.row(), 0),
                      toIndex.sibling(toIndex.row(), columnCount() - 1), roles);
        });
    }
}

QModelIndex FileSystemModel::rootIndex() const
{
    return index(rootPath());
}

bool FileSystemModel::replaceResource(const QModelIndex &index, const QString &file, QWidget *parent)
{
    if (!sourceModel) {
        return false;
    }
    const auto path = filePath(index);
    const auto resourceIndex = sourceModel->findIndex(path);
    if (resourceIndex.isValid()) {
        return sourceModel->replaceResource(resourceIndex, file, parent);
    } else {
        if (Utils::replaceFile(path, parent)) {
            updated(index.sibling(index.row(), 0),
                    index.sibling(index.row(), columnCount() - 1));
            return true;
        }
        return false;
    }
}

bool FileSystemModel::removeResource(const QModelIndex &index)
{
    return removeRow(index.row(), index.parent());
}

QString FileSystemModel::getResourcePath(const QModelIndex &index) const
{
    return filePath(index);
}

bool FileSystemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(count)
    if (!sourceModel) {
        return false;
    }
    bool success = true;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
        const auto path = filePath(index(row, 0, parent));
        const auto resourceIndex = sourceModel->findIndex(path);
        if (resourceIndex.isValid()) {
            if (!sourceModel->removeResource(resourceIndex)) {
                success = false;
            }
        } else {
            if (!QFile::remove(path)) {
                success = false;
            }
        }
    }
    endRemoveRows();
    return success;
}

void FileSystemModel::updated(const QModelIndex &from, const QModelIndex &to, const QVector<int> &roles)
{
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        emit dataChanged(from, to, roles);
        timer->deleteLater();
    });
    timer->setSingleShot(true);
    timer->start(10);
}
