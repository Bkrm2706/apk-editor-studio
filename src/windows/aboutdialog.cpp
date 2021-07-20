#include "windows/aboutdialog.h"
#include "tools/adb.h"
#include "tools/apktool.h"
#include "tools/apksigner.h"
#include "tools/java.h"
#include "tools/javac.h"
#include "base/utils.h"
#include <QFormLayout>
#include <QTabWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QTextStream>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(Utils::scale(700, 400));

    QTabWidget *tabs = new QTabWidget(this);
    tabs->addTab(createAboutTab(), tr("About"));
    tabs->addTab(createAuthorsTab(), tr("Authors"));
    tabs->addTab(createVersionsTab(), tr("Version History"));
    tabs->addTab(createLibrariesTab(), tr("Technologies"));
    tabs->addTab(createLicenseTab(), tr("License"));

    QLabel *icon = new QLabel(this);
    icon->setContentsMargins(0, 0, 10, 4);
    icon->setPixmap(QIcon::fromTheme("apk-editor-studio").pixmap(Utils::scale(48, 48)));
    icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QLabel *title = new QLabel(Utils::getAppTitleAndVersion(), this);
    QFont titleFont = title->font();
#ifndef Q_OS_OSX
    titleFont.setPointSize(11);
#else
    titleFont.setPointSize(16);
#endif
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);

    QHBoxLayout *layoutTitle = new QHBoxLayout;
    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutTitle->addWidget(icon);
    layoutTitle->addWidget(title);
    layoutMain->addLayout(layoutTitle);
    layoutMain->addWidget(tabs);
    layoutMain->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &AboutDialog::accept);
}

GradientWidget *AboutDialog::createAboutTab()
{
    auto tab = new GradientWidget(this);

    auto icon = new QLabel(this);
    icon->setMargin(16);
    icon->setPixmap(QPixmap(":/icons/other/about.png").scaled(Utils::scale(128, 128)));
    icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto labelApplicationTitle = new QLabel(QString("<b>%1</b>").arg(Utils::getAppTitleAndVersion()), this);
    auto labelAuthor = new QLabel("Alexander Gorishnyak", this);
    auto labelWebsiteLink = new QLabel(createLink(Utils::getWebsiteUtmUrl(), Utils::getWebsiteUrl()), this);
    auto labelIssuesLink = new QLabel(createLink(Utils::getIssuesUrl()), this);
    auto labelTranslateLink = new QLabel(createLink(Utils::getTranslationsUrl()), this);
    auto labelBuildTime = new QLabel(QString("<p>%1 - %2</p>").arg(QString(__DATE__).toUpper(), __TIME__), this);
    labelWebsiteLink->setOpenExternalLinks(true);
    labelIssuesLink->setOpenExternalLinks(true);
    labelTranslateLink->setOpenExternalLinks(true);
    labelWebsiteLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    labelIssuesLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    labelTranslateLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    if (layoutDirection() == Qt::RightToLeft) {
        labelApplicationTitle->setAlignment(Qt::AlignRight);
        labelAuthor->setAlignment(Qt::AlignRight);
        labelWebsiteLink->setAlignment(Qt::AlignRight);
        labelIssuesLink->setAlignment(Qt::AlignRight);
        labelTranslateLink->setAlignment(Qt::AlignRight);
        labelBuildTime->setAlignment(Qt::AlignRight);
    }

    auto formLayout = new QFormLayout;
    formLayout->setSpacing(2);
    formLayout->addRow(tr("Author:"), labelAuthor);
    formLayout->addItem(new QSpacerItem(0, 10));
    formLayout->addRow(tr("Website:"), labelWebsiteLink);
    formLayout->addRow(tr("Bug Tracker:"), labelIssuesLink);
    formLayout->addRow(tr("Translation:"), labelTranslateLink);

    auto contentLayout = new QVBoxLayout;
    contentLayout->setSpacing(16);
    contentLayout->addStretch(1);
    contentLayout->addWidget(labelApplicationTitle);
    contentLayout->addLayout(formLayout);
    contentLayout->addWidget(labelBuildTime);
    contentLayout->addStretch(1);

    auto layout = new QHBoxLayout(tab);
    layout->addWidget(icon);
    layout->addLayout(contentLayout);

    return tab;
}

QWidget *AboutDialog::createAuthorsTab()
{
    QString content = "";
    const QString br("<br />");
    QFile file(Utils::getSharedPath("docs/authors.txt"));
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        while (!stream.atEnd()) {
            QString line = stream.readLine();

            // Parse Markdown headers and lists:
            if (line.startsWith("# ")) {
                line = QString("<h3>%1</h3>").arg(line.mid(2));
                if (content.endsWith(br)) {
                    content.chop(br.length());
                }
            } else if (line.startsWith("## ")) {
                line = QString("<h4>%1</h4>").arg(line.mid(3));
                if (content.endsWith(br)) {
                    content.chop(br.length());
                }
            } else if (line.startsWith("- ")) {
                line = line.mid(2) + br;
            }

            // Parse Markdown links:
            QRegularExpression regex("\\[(.+?)\\]\\((.+?)\\)");
            forever {
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    const QString title = match.captured(1);
                    const QString href = match.captured(2);
                    const QString link = QString("<a href=\"%1\">%2</a>").arg(href, title);
                    line.replace(match.capturedStart(0), match.capturedLength(0), link);
                } else {
                    break;
                }
            }
            content.append(line);
        }
    }

    QTextBrowser *tab = new QTextBrowser(this);
    tab->setReadOnly(true);
    tab->setOpenExternalLinks(true);
    tab->setText(content);
    return tab;
}

QWidget *AboutDialog::createVersionsTab()
{
    QPlainTextEdit *tab = new QPlainTextEdit(this);
    tab->setReadOnly(true);

    QFile file(Utils::getSharedPath("docs/versions.txt"));
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        while (!stream.atEnd()) {
            QString line = stream.readLine();

            // Parse Markdown headers and lists:
            if (line.startsWith("# ")) {
                line = QString("<h3>%1</h3>").arg(line.mid(2));
            } else if (line.startsWith("## ")) {
                line = QString("<h3>%1<br></h3>").arg(line.mid(3));
            } else if (line.startsWith("- ")) {
                line = line.mid(2);
            }

            // Parse Markdown links:
            QRegularExpression regex("\\[(.+?)\\]\\((.+?)\\)");
            forever {
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    const QString title = match.captured(1);
                    const QString href = match.captured(2);
                    const QString link = QString("<a href=\"%1\">%2</a>").arg(href, title);
                    line.replace(match.capturedStart(0), match.capturedLength(0), link);
                } else {
                    break;
                }
            }
            tab->appendHtml(line);
        }
        tab->moveCursor(QTextCursor::Start);
        tab->ensureCursorVisible();
    }

    return tab;
}

QWidget *AboutDialog::createLibrariesTab()
{
    GradientWidget *tab = new GradientWidget(this);

    const QChar mdash(0x2014);
    const QChar ellipsis(0x2026);

    QLabel *labelQt = new QLabel(ellipsis, this);
    QLabel *labelJre = new QLabel(ellipsis, this);
    QLabel *labelJdk = new QLabel(ellipsis, this);
    QLabel *labelApktool = new QLabel(ellipsis, this);
    QLabel *labelApksigner = new QLabel(ellipsis, this);
    QLabel *labelAdb = new QLabel(ellipsis, this);

    QFormLayout *layout = new QFormLayout(tab);
    layout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    layout->setFormAlignment(Qt::AlignCenter);
    layout->setLabelAlignment(Qt::AlignRight);
    layout->addRow(new QLabel("Qt", this), labelQt);
    layout->addRow(new QLabel("JRE", this), labelJre);
    layout->addRow(new QLabel("JDK", this), labelJdk);
    layout->addRow(new QLabel("Apktool", this), labelApktool);
    layout->addRow(new QLabel("Apksigner", this), labelApksigner);
    layout->addRow(new QLabel("ADB", this), labelAdb);

    // Set Qt version:

    labelQt->setText(QT_VERSION_STR);

    // Set JRE version:

    auto jre = new Java::Version(this);
    connect(jre, &Java::Version::finished, this, [=](bool success) {
        labelJre->setText(success ? jre->version() : mdash);
        jre->deleteLater();
    });
    jre->run();

    // Set JDK version:

    auto jdk = new Javac::Version(this);
    connect(jdk, &Javac::Version::finished, this, [=](bool success) {
        labelJdk->setText(success ? jdk->version() : mdash);
        jdk->deleteLater();
    });
    jdk->run();

    // Set Apktool version:

    auto apktool = new Apktool::Version(this);
    connect(apktool, &Apktool::Version::finished, this, [=](bool success) {
        labelApktool->setText(success ? apktool->version() : mdash);
        apktool->deleteLater();
    });
    apktool->run();

    // Set Apksigner version:

    auto apksigner = new Apksigner::Version(this);
    connect(apksigner, &Apksigner::Version::finished, this, [=](bool success) {
        labelApksigner->setText(success ? apksigner->version() : mdash);
        apksigner->deleteLater();
    });
    apksigner->run();

    // Set ADB version:

    auto adb = new Adb::Version(this);
    connect(adb, &Adb::Version::finished, this, [=](bool success) {
        labelAdb->setText(success ? adb->version() : mdash);
        adb->deleteLater();
    });
    adb->run();

    return tab;
}

QWidget *AboutDialog::createLicenseTab()
{
    QTextBrowser *tab = new QTextBrowser(this);
    tab->setReadOnly(true);
    tab->setOpenExternalLinks(true);

    QFile file(Utils::getSharedPath("docs/licenses/apk-editor-studio.html"));
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        tab->setText(stream.readAll());
    }

    return tab;
}

QString AboutDialog::createLink(const QString &url, const QString &title) const
{
    const QString link("<a href=\"%1\">%2</a>");
    return link.arg(url, title.isEmpty() ? url : title);
}
