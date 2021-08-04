#include "widgets/logdelegate.h"
#include "apk/logmodel.h"
#include "base/utils.h"
#include <QApplication>
#include <QPainter>

LogDelegate::LogDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    spinnerAngle = 0;
    iconExpand = QIcon::fromTheme("emblem-information");
    spinnerTimer.setInterval(25);
    connect(&spinnerTimer, &QTimer::timeout, this, [this]() {
        spinnerAngle = (spinnerAngle > 0) ? spinnerAngle - 160 : 5760;
        emit updated();
    });
}

void LogDelegate::setLoading(bool loading)
{
    loading
        ? spinnerTimer.start()
        : spinnerTimer.stop();
}

void LogDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    painter->save();
    painter->setPen(QPen(QPalette().windowText(), Utils::scale(1.5)));
    painter->setRenderHint(QPainter::Antialiasing);

    const bool ltr = QApplication::layoutDirection() == Qt::LeftToRight;
    const int margin = 2;
    const int h = option.rect.height() - Utils::scale(8);
    const int w = h;
    const int x = option.rect.right() - w - margin;
    const int y = option.rect.center().y() - h / 2;

    const auto model = static_cast<const LogModel *>(index.model());
    const bool loading = (index.row() == model->rowCount() - 1) ? model->getLoadingState() : false;
    const QString message = index.sibling(index.row(), LogModel::DescriptiveColumn).data().toString();

    if (loading) {
        painter->drawArc(ltr ? x : margin, y, w, h, spinnerAngle, 12 * 360);
    } else if (!message.isEmpty()) {
        if (!ltr) {
            QTransform mirror;
            mirror.scale(-1, 1);
            mirror.translate(-option.rect.width(), 0);
            painter->setTransform(mirror);
        }
        iconExpand.paint(painter, x, y, w, h);
    }

    painter->restore();
}
