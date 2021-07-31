#ifndef READONLYCHECKBOX_H
#define READONLYCHECKBOX_H

#include <QCheckBox>

class ReadOnlyCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit ReadOnlyCheckBox(QWidget *parent = nullptr);
    explicit ReadOnlyCheckBox(const QString &text, QWidget *parent = nullptr);
};

#endif // READONLYCHECKBOX_H
