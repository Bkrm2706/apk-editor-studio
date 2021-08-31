#include "base/actionprovider.h"
#include "base/application.h"
#include "base/updater.h"
#include "windows/androidexplorer.h"
#include "windows/devicemanager.h"
#include "windows/dialogs.h"
#include "windows/keymanager.h"
#include "windows/optionsdialog.h"
#include "windows/rememberdialog.h"
#include "tools/adb.h"
#include "tools/keystore.h"
#include <QMenu>
#include <QDesktopServices>
#include <QDateTime>

void ActionProvider::openApk(MainWindow *window) const
{
    const QStringList paths = Dialogs::getOpenApkFilenames(window);
    openApk(paths, window);
}

void ActionProvider::openApk(const QString &path, MainWindow *window) const
{
    openApk(QStringList(path), window);
}

void ActionProvider::openApk(const QStringList &paths, MainWindow *window) const
{
    for (const QString &path : paths) {
        auto project = projects.add(path, window);
        if (project) {
            auto command = new Project::ProjectCommand(project);
            command->add(project->createUnpackCommand(), true);
            command->run();
        }
    }
}

void ActionProvider::optimizeApk(MainWindow *window) const
{
    const QStringList paths = Dialogs::getOpenApkFilenames(window);
    optimizeApk(paths, window);
}

void ActionProvider::optimizeApk(const QStringList &paths, MainWindow *window) const
{
    for (const QString &path : paths) {
        auto project = projects.add(path, window);
        if (project) {
            auto command = new Project::ProjectCommand(project);
            command->add(project->createZipalignCommand(), true);
            command->run();
        }
    }
}

void ActionProvider::signApk(MainWindow *window) const
{
    const QStringList paths = Dialogs::getOpenApkFilenames(window);
    signApk(paths, window);
}

void ActionProvider::signApk(const QStringList &paths, MainWindow *window) const
{
    const auto keystore = Keystore::get(window);
    if (keystore) {
        signApk(paths, keystore.get(), window);
    }
}

void ActionProvider::signApk(const QStringList &paths, const Keystore *keystore, MainWindow *window) const
{
    Q_ASSERT(keystore);
    for (const QString &path : paths) {
        auto project = projects.add(path, window);
        if (project) {
            auto command = new Project::ProjectCommand(project);
            command->add(project->createSignCommand(keystore), true);
            command->run();
        }
    }
}

void ActionProvider::installApk(MainWindow *window) const
{
    const auto device = Dialogs::getInstallDevice(window);
    if (!device.isNull()) {
        installApk(device.getSerial(), window);
    }
}

void ActionProvider::installApk(const QString &serial, MainWindow *window) const
{
    const QStringList paths = Dialogs::getOpenApkFilenames(window);
    installApk(paths, serial, window);
}

void ActionProvider::installApk(const QStringList &paths, const QString &serial, MainWindow *window) const
{
    Q_ASSERT(!serial.isEmpty());
    for (const QString &path : paths) {
        auto project = projects.add(path, window);
        if (project) {
            auto command = new Project::ProjectCommand(project);
            command->add(project->createInstallCommand(serial), true);
            command->run();
        }
    }
}

bool ActionProvider::closeApk(Project *project) const
{
    return projects.close(project);
}

void ActionProvider::visitWebPage() const
{
    QDesktopServices::openUrl(Utils::getWebsiteUtmUrl());
}

void ActionProvider::visitSourcePage() const
{
    QDesktopServices::openUrl(Utils::getRepositoryUrl());
}

void ActionProvider::visitDonatePage() const
{
    QDesktopServices::openUrl(Utils::getDonationsUrl());
}

void ActionProvider::visitUpdatePage() const
{
    QDesktopServices::openUrl(Utils::getUpdateUrl());
}

void ActionProvider::visitBlogPage(const QString &post) const
{
    QDesktopServices::openUrl(Utils::getBlogPostUrl(post));
}

void ActionProvider::exit(QWidget *widget) const
{
    widget->close();
}

void ActionProvider::checkUpdates(QWidget *parent) const
{
    Updater::check(true, parent);
}

bool ActionProvider::resetSettings(QWidget *parent) const
{
    const QString question(tr("Are you sure you want to reset settings?"));
    const int answer = QMessageBox::question(parent, {}, question);
    if (answer != QMessageBox::Yes) {
        return false;
    }
    app->settings->reset();
    return true;
}

void ActionProvider::openOptions(QWidget *parent) const
{
    OptionsDialog settings(parent);
    settings.exec();
}

void ActionProvider::openDeviceManager(QWidget *parent) const
{
    DeviceManager deviceManager(parent);
    deviceManager.exec();
}

void ActionProvider::openKeyManager(QWidget *parent) const
{
    KeyManager keyManager(parent);
    keyManager.exec();
}

void ActionProvider::openAndroidExplorer(MainWindow *window) const
{
    const auto device = Dialogs::getExplorerDevice(window);
    if (!device.isNull()) {
        openAndroidExplorer(device.getSerial(), window);
    }
}

void ActionProvider::openAndroidExplorer(const QString &serial, MainWindow *window) const
{
    auto explorer = new AndroidExplorer(serial, window);
    explorer->setAttribute(Qt::WA_DeleteOnClose);
    explorer->show();
}

void ActionProvider::takeScreenshot(QWidget *parent) const
{
    const auto device = Dialogs::getScreenshotDevice(parent);
    if (!device.isNull()) {
        takeScreenshot(device.getSerial(), parent);
    }
}

void ActionProvider::takeScreenshot(const QString &serial, QWidget *parent) const
{
    const QString datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    const QString filename = QString("screenshot_%1.png").arg(datetime);
    const QString dst = Dialogs::getSaveImageFilename(filename, parent);
    if (!dst.isEmpty()) {
        auto screenshot = new Adb::Screenshot(dst, serial, parent);
        app->connect(screenshot, &Adb::Screenshot::finished, parent, [=](bool success) {
            if (success) {
                RememberDialog::say("ScreenshotSuccess", tr("Screenshot has been successfully created!"), parent);
            } else {
                QMessageBox::warning(parent, {}, tr("Could not take a screenshot."));
            }
            screenshot->deleteLater();
        });
        screenshot->run();
    }
}

QAction *ActionProvider::getOpenApk(MainWindow *window) const
{
    auto action = new QAction(QIcon::fromTheme("document-open"), {}, window);
    action->setShortcut(QKeySequence::Open);

    auto translate = [=]() { action->setText(tr("&Open APK...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, window, [=]() {
        openApk(window);
    });
    return action;
}

QAction *ActionProvider::getOptimizeApk(MainWindow *window) const
{
    auto action = new QAction(QIcon::fromTheme("apk-optimize"), {}, window);

    auto translate = [=]() { action->setText(tr("&Optimize External APK...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, window, [=]() {
        optimizeApk(window);
    });

    return action;
}

QAction *ActionProvider::getSignApk(MainWindow *window) const
{
    auto action = new QAction(QIcon::fromTheme("apk-sign"), {}, window);

    auto translate = [=]() { action->setText(tr("&Sign External APK...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, window, [=]() {
        signApk(window);
    });

    return action;
}

QAction *ActionProvider::getInstallApk(MainWindow *window) const
{
    return getInstallApk({}, window);
}

QAction *ActionProvider::getInstallApk(const QString &serial, MainWindow *window) const
{
    auto action = new QAction(QIcon::fromTheme("apk-install"), {}, window);

    auto translate = [=]() { action->setText(tr("&Install External APK...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    action->setShortcut(QKeySequence("Ctrl+Shift+I"));
    if (serial.isEmpty()) {
        connect(action, &QAction::triggered, window, [=]() {
            installApk(window);
        });
    } else {
        connect(action, &QAction::triggered, window, [=]() {
            installApk(serial, window);
        });
    }

    return action;
}

QAction *ActionProvider::getVisitWebPage(QObject *parent) const
{
    auto action = new QAction(QIcon::fromTheme("help-website"), {}, parent);

    auto translate = [=]() { action->setText(tr("Visit &Website")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, this, &ActionProvider::visitWebPage);
    return action;
}

QAction *ActionProvider::getVisitSourcePage(QObject *parent) const
{
    auto action = new QAction(QIcon::fromTheme("help-source"), {}, parent);

    auto translate = [=]() { action->setText(tr("&Source Code")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, this, &ActionProvider::visitSourcePage);
    return action;
}

QAction *ActionProvider::getVisitDonatePage(QObject *parent) const
{
    auto action = new QAction(QIcon::fromTheme("help-donate"), {}, parent);

    auto translate = [=]() { action->setText(tr("Make a &Donation")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, this, &ActionProvider::visitDonatePage);
    return action;
}

QAction *ActionProvider::getExit(QWidget *widget) const
{
    auto action = new QAction(QIcon::fromTheme("application-exit"), {}, widget);
    action->setShortcut(QKeySequence::Quit);
    action->setMenuRole(QAction::QuitRole);

    auto translate = [=]() { action->setText(tr("E&xit")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, widget, [=]() {
        exit(widget);
    });
    return action;
}

QAction *ActionProvider::getCheckUpdates(QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("help-update"), {}, parent);

    auto translate = [=]() { action->setText(tr("Check for &Updates")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, parent, [=]() {
        checkUpdates(parent);
    });

    return action;
}

QAction *ActionProvider::getResetSettings(QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("edit-delete"), {}, parent);

    auto translate = [=]() { action->setText(tr("&Reset Settings...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, parent, [=]() {
        resetSettings(parent);
    });

    return action;
}

QAction *ActionProvider::getOpenOptions(QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("configure"), {}, parent);
    action->setShortcut(QKeySequence("Ctrl+P"));
    action->setMenuRole(QAction::PreferencesRole);

    auto translate = [=]() { action->setText(tr("&Options...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, parent, [=]() {
        openOptions(parent);
    });

    return action;
}

QAction *ActionProvider::getOpenDeviceManager(QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("smartphone"), {}, parent);
    action->setShortcut(QKeySequence("Ctrl+D"));

    //: This string refers to multiple devices (as in "Manager of devices").
    auto translate = [=]() { action->setText(tr("&Device Manager...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, parent, [=]() {
        openDeviceManager(parent);
    });

    return action;
}

QAction *ActionProvider::getOpenKeyManager(QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("apk-sign"), {}, parent);
    action->setShortcut(QKeySequence("Ctrl+K"));

    //: This string refers to multiple keys (as in "Manager of keys").
    auto translate = [=]() { action->setText(tr("&Key Manager...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    connect(action, &QAction::triggered, parent, [=]() {
        openKeyManager(parent);
    });

    return action;
}

QAction *ActionProvider::getOpenAndroidExplorer(MainWindow *window) const
{
    return getOpenAndroidExplorer({}, window);
}

QAction *ActionProvider::getOpenAndroidExplorer(const QString &serial, MainWindow *window) const
{
    auto action = new QAction(QIcon::fromTheme("tool-androidexplorer"), {}, window);
    action->setShortcut(QKeySequence("Ctrl+Shift+X"));

    auto translate = [=]() { action->setText(tr("&Android Explorer...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    if (serial.isEmpty()) {
        connect(action, &QAction::triggered, window, [=]() {
            openAndroidExplorer(window);
        });
    } else {
        connect(action, &QAction::triggered, window, [=]() {
            openAndroidExplorer(serial, window);
        });
    }

    return action;
}

QAction *ActionProvider::getTakeScreenshot(QWidget *parent) const
{
    return getTakeScreenshot({}, parent);
}

QAction *ActionProvider::getTakeScreenshot(const QString &serial, QWidget *parent) const
{
    auto action = new QAction(QIcon::fromTheme("camera-photo"), {}, parent);

    auto translate = [=]() { action->setText(tr("Take &Screenshot...")); };
    connect(this, &ActionProvider::languageChanged, action, translate);
    translate();

    if (serial.isEmpty()) {
        connect(action, &QAction::triggered, parent, [=]() {
            takeScreenshot(parent);
        });
    } else {
        connect(action, &QAction::triggered, parent, [=]() {
            takeScreenshot(serial, parent);
        });
    }

    return action;
}

QMenu *ActionProvider::getLanguages(QWidget *parent) const
{
    auto menu = new QMenu(parent);
    auto actions = new QActionGroup(parent);
    actions->setExclusive(true);

    auto translate = [=]() {
        // Set title
        menu->setTitle(tr("&Language"));
        // Set icon
        const QString currentLocale = app->settings->getLanguage();
        QIcon flag(Utils::getLocaleFlag(currentLocale));
        menu->setIcon(flag);
        // Set check mark
        for (QAction *action : actions->actions()) {
            if (action->property("locale") == currentLocale) {
                action->setChecked(true);
                break;
            }
        }
    };
    connect(this, &ActionProvider::languageChanged, menu, translate);
    translate();

    QList<Language> languages = app->getLanguages();
    for (const Language &language : languages) {
        const QString localeTitle = language.getTitle();
        const QString localeCode = language.getCode();
        const QIcon localeFlag = language.getFlag();
        QAction *action = actions->addAction(localeFlag, localeTitle);
        action->setCheckable(true);
        action->setProperty("locale", localeCode);
        connect(action, &QAction::triggered, [=]() {
            app->setLanguage(localeCode);
        });
    }
    menu->addActions(actions->actions());

    return menu;
}

bool ActionProvider::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        emit languageChanged();
        return true;
    }
    return QObject::event(event);
}
