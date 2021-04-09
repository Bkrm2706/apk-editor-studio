#ifndef PROJECT_H
#define PROJECT_H

#include "apk/filesystemmodel.h"
#include "apk/iconitemsmodel.h"
#include "apk/logmodel.h"
#include "apk/manifestmodel.h"
#include "apk/projectstate.h"
#include "apk/resourceitemsmodel.h"
#include "base/command.h"
#include <QIcon>

class Keystore;

class ProjectContentsPath
{
public:
    ProjectContentsPath() = default;
    explicit ProjectContentsPath(const QString &path);

    QString get() const;
    void set(const QString &path);

private:
    QString path;
};

class Project : public QObject
{
    Q_OBJECT

public:
    Project(const QString &path);
    ~Project() override;

    QString getTitle() const;
    QString getOriginalPath() const;
    QString getContentsPath() const;
    QString getPackageName() const;
    QIcon getThumbnail() const;
    const ProjectState &getState() const;

    bool getWithSources() const;

    void setApplicationIcon(const QString &path, QWidget *parent = nullptr);
    bool setPackageName(const QString &name);

    void journal(const QString &brief, LogEntry::Type type = LogEntry::Information);
    void journal(const QString &brief, const QString &descriptive, LogEntry::Type type = LogEntry::Information);

    Manifest *manifest;

    ResourceItemsModel resourcesModel;
    FileSystemModel filesystemModel;
    IconItemsModel iconsProxy;
    ManifestModel manifestModel;
    LogModel logModel;

    class ProjectCommand : public Commands
    {
    public:
        ProjectCommand(Project *project);
    };

    class LoadUnpackedCommand : public Command
    {
    public:
        LoadUnpackedCommand(Project *project) : project(project) {}
        void run() override;

    private:
        Project *project;
    };

    Command *createUnpackCommand();
    Command *createPackCommand(const QString &target);
    Command *createZipalignCommand(const QString &apk = QString());
    Command *createSignCommand(const Keystore *keystore, const QString &apk = QString());
    Command *createInstallCommand(const QString &serial, const QString &apk = QString());

signals:
    void stateUpdated();

private:
    ProjectState state;

    QString originalPath;
    ProjectContentsPath contentsPath;
    QIcon thumbnail;

    bool withSources = false;
    bool withResources = false;
    bool withBrokenResources = false;
};

#endif // PROJECT_H
