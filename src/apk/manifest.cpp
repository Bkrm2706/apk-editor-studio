#include "apk/manifest.h"
#include <QTextStream>
#include <QDebug>

Manifest::Manifest(const QString &xmlPath, const QString &ymlPath)
{
    // XML:

    xmlFile = new QFile(xmlPath);
    if (xmlFile->open(QFile::ReadWrite)) {
        QTextStream stream(xmlFile);
        stream.setCodec("UTF-8");
        xml.setContent(stream.readAll());
        manifestNode = xml.firstChildElement("manifest");
        auto applicationNode = manifestNode.firstChildElement("application");
        scopes.append(new ManifestScope(applicationNode));
        auto applicationChild = applicationNode.firstChildElement();
        while (!applicationChild.isNull()) {
            if (QStringList({"application", "activity"}).contains(applicationChild.nodeName())) {
                if (applicationChild.hasAttribute("android:icon") ||
                    applicationChild.hasAttribute("android:roundIcon") ||
                    applicationChild.hasAttribute("android:banner")) {
                        scopes.append(new ManifestScope(applicationChild));
                }
            }
            applicationChild = applicationChild.nextSiblingElement();
        }
        packageName = manifestNode.attribute("package");
    }

    // YAML:

    regexMinSdk.setPatternOptions(QRegularExpression::MultilineOption);
    regexTargetSdk.setPatternOptions(QRegularExpression::MultilineOption);
    regexVersionCode.setPatternOptions(QRegularExpression::MultilineOption);
    regexVersionName.setPatternOptions(QRegularExpression::MultilineOption);
    regexMinSdk.setPattern("(?<=^  minSdkVersion: ')\\d+(?='$)");
    regexTargetSdk.setPattern("(?<=^  targetSdkVersion: ')\\d+(?='$)");
    regexVersionCode.setPattern("(?<=^  versionCode: ')\\d+(?='$)");
    regexVersionName.setPattern("(?<=^  versionName: ).+(?=$)");

    ymlFile = new QFile(ymlPath);
    if (ymlFile->open(QFile::ReadWrite)) {
        QTextStream stream(ymlFile);
        stream.setCodec("UTF-8");
        yml = stream.readAll();
        minSdk = regexMinSdk.match(yml).captured().toInt();
        targetSdk = regexTargetSdk.match(yml).captured().toInt();
        versionCode = regexVersionCode.match(yml).captured().toInt();
        versionName = regexVersionName.match(yml).captured();
    }
}

Manifest::~Manifest()
{
    delete xmlFile;
    delete ymlFile;
    qDeleteAll(scopes);
}

int Manifest::getMinSdk() const
{
    return minSdk;
}

int Manifest::getTargetSdk() const
{
    return targetSdk;
}

int Manifest::getVersionCode() const
{
    return versionCode;
}

const QString &Manifest::getVersionName() const
{
    return versionName;
}

const QString &Manifest::getPackageName() const
{
    return packageName;
}

void Manifest::setApplicationLabel(const QString &value)
{
    scopes.first()->label().setValue(value);
    saveXml();
}

void Manifest::setMinSdk(int value)
{
    value = qMax(0, value);
    minSdk = value;
    yml.replace(regexMinSdk, QString::number(value));
    saveYml();
}

void Manifest::setTargetSdk(int value)
{
    value = qMax(1, value);
    targetSdk = value;
    yml.replace(regexTargetSdk, QString::number(value));
    saveYml();
}

void Manifest::setVersionCode(int value)
{
    value = qMax(0, value);
    versionCode = value;
    yml.replace(regexVersionCode, QString::number(value));
    saveYml();
}

void Manifest::setVersionName(const QString &value)
{
    versionName = value;
    yml.replace(regexVersionName, value);
    saveYml();
}

void Manifest::setPackageName(const QString &newPackageName)
{
    const auto originalPackageName = getPackageName();
    xmlFile->seek(0);
    const QString data(xmlFile->readAll());
    QString newData(data);
    newData.replace(originalPackageName, newPackageName);
    if (newData != data) {
        xmlFile->resize(0);
        xmlFile->write(newData.toUtf8());
        xmlFile->flush();
    }
    packageName = newPackageName;
}

QList<Permission> Manifest::getPermissionList() const
{
    QList<Permission> permissions;
    const auto manifestChildNodes = manifestNode.childNodes();
    for (int i = 0; i < manifestChildNodes.count(); ++i) {
        const auto element = manifestChildNodes.at(i).toElement();
        if (element.tagName() == "uses-permission") {
            permissions.append(Permission(element));
        }
    }
    return permissions;
}

Permission Manifest::addPermission(const QString &permission)
{
    auto element = xml.createElement("uses-permission");
    element.setAttribute("android:name", permission);
    manifestNode.appendChild(element);
    saveXml();
    return Permission(element);
}

void Manifest::removePermission(const Permission &permission)
{
    manifestNode.removeChild(permission.getNode());
    saveXml();
}

bool Manifest::saveXml()
{
    if (xmlFile->isWritable()) {
        xmlFile->resize(0);
        QTextStream stream(xmlFile);
        xml.save(stream, 4);
        return true;
    } else {
        qWarning() << "Error: Could not save AndroidManifest.xml";
        return false;
    }
}

bool Manifest::saveYml()
{
    if (ymlFile->isWritable()) {
        ymlFile->resize(0);
        QTextStream stream(ymlFile);
        stream.setCodec("UTF-8");
        stream << yml;
        return true;
    } else {
        qWarning() << "Error: Could not save apktool.yml";
        return false;
    }
}
