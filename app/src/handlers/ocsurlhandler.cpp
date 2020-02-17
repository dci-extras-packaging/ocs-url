#include "ocsurlhandler.h"

#include <QUrlQuery>
#include <QDesktopServices>

#include "qtil_file.h"
#include "qtil_dir.h"
#include "qtil_networkresource.h"
#include "qtil_package.h"

#include "handlers/confighandler.h"

OcsUrlHandler::OcsUrlHandler(const QString &ocsUrl, ConfigHandler *configHandler, QObject *parent)
    : QObject(parent), ocsUrl_(ocsUrl), configHandler_(configHandler)
{
    parse();
}

QString OcsUrlHandler::ocsUrl() const
{
    return ocsUrl_;
}

QJsonObject OcsUrlHandler::metadata() const
{
    return metadata_;
}

void OcsUrlHandler::process()
{
    // ocss scheme is a reserved name, so the process of ocss
    // is the same process of the ocs scheme currently.

    if (!isValid()) {
        QJsonObject result;
        result["status"] = QString("error_validation");
        result["message"] = tr("Invalid OCS-URL");
        emit finishedWithError(result);
        return;
    }

    auto url = metadata_["url"].toString();
    auto *resource = new Qtil::NetworkResource(url, QUrl(url), true, this);
    connect(resource, &Qtil::NetworkResource::downloadProgress, this, &OcsUrlHandler::downloadProgress);
    connect(resource, &Qtil::NetworkResource::finished, this, &OcsUrlHandler::networkResourceFinished);
    resource->get();
    emit started();
}

bool OcsUrlHandler::isValid() const
{
    QString scheme = metadata_["scheme"].toString();
    QString command = metadata_["command"].toString();
    QString url = metadata_["url"].toString();
    QString type = metadata_["type"].toString();
    QString filename = metadata_["filename"].toString();

    if ((scheme == "ocs" || scheme == "ocss")
            && (command == "download" || command == "install")
            && QUrl(url).isValid()
            && configHandler_->getAppConfigInstallTypes().contains(type)
            && !filename.isEmpty()) {
        return true;
    }
    return false;
}

void OcsUrlHandler::openDestination() const
{
    auto type = metadata_["type"].toString();
    QDesktopServices::openUrl(QUrl("file://" + configHandler_->getAppConfigInstallTypes()[type].toObject()["destination"].toString()));
}

void OcsUrlHandler::networkResourceFinished(Qtil::NetworkResource *resource)
{
    if (!resource->isFinishedWithNoError()) {
        QJsonObject result;
        result["status"] = QString("error_network");
        result["message"] = resource->reply()->errorString();
        emit finishedWithError(result);
        resource->deleteLater();
        return;
    }

    if (metadata_["command"].toString() == "download") {
        saveDownloadedFile(resource);
    }
    else if (metadata_["command"].toString() == "install") {
        installDownloadedFile(resource);
    }
}

void OcsUrlHandler::parse()
{
    QUrl url(ocsUrl_);
    QUrlQuery query(url);

    metadata_["scheme"] = QString("ocs");
    metadata_["command"] = QString("download");
    metadata_["url"] = QString("");
    metadata_["type"] = QString("downloads");
    metadata_["filename"] = QString("");

    if (!url.scheme().isEmpty()) {
        metadata_["scheme"] = url.scheme();
    }

    if (!url.host().isEmpty()) {
        metadata_["command"] = url.host();
    }

    if (query.hasQueryItem("url") && !query.queryItemValue("url").isEmpty()) {
        metadata_["url"] = query.queryItemValue("url", QUrl::FullyDecoded);
    }

    if (query.hasQueryItem("type") && !query.queryItemValue("type").isEmpty()) {
        metadata_["type"] = query.queryItemValue("type", QUrl::FullyDecoded);
    }

    if (query.hasQueryItem("filename") && !query.queryItemValue("filename").isEmpty()) {
        metadata_["filename"] = QUrl(query.queryItemValue("filename", QUrl::FullyDecoded)).fileName();
    }

    if (!metadata_["url"].toString().isEmpty() && metadata_["filename"].toString().isEmpty()) {
        metadata_["filename"] = QUrl(metadata_["url"].toString()).fileName();
    }
}

void OcsUrlHandler::saveDownloadedFile(Qtil::NetworkResource *resource)
{
    QJsonObject result;

    auto type = metadata_["type"].toString();
    Qtil::Dir destDir(configHandler_->getAppConfigInstallTypes()[type].toObject()["destination"].toString());
    destDir.make();
    Qtil::File destFile(destDir.path() + "/" + metadata_["filename"].toString());

    if (!resource->saveData(destFile.path())) {
        result["status"] = QString("error_save");
        result["message"] = tr("Failed to save data");
        emit finishedWithError(result);
        resource->deleteLater();
        return;
    }

    result["status"] = QString("success_download");
    result["message"] = tr("The file has been downloaded");
    emit finishedWithSuccess(result);

    resource->deleteLater();
}

void OcsUrlHandler::installDownloadedFile(Qtil::NetworkResource *resource)
{
    QJsonObject result;

    Qtil::File tempFile(Qtil::Dir::tempPath() + "/" + metadata_["filename"].toString());

    if (!resource->saveData(tempFile.path())) {
        result["status"] = QString("error_save");
        result["message"] = tr("Failed to save data");
        emit finishedWithError(result);
        resource->deleteLater();
        return;
    }

    Qtil::Package package(tempFile.path());
    auto type = metadata_["type"].toString();
    Qtil::Dir destDir(configHandler_->getAppConfigInstallTypes()[type].toObject()["destination"].toString());
    destDir.make();
    Qtil::File destFile(destDir.path() + "/" + metadata_["filename"].toString());

    if (type == "bin"
            && package.installAsProgram(destFile.path())) {
        result["message"] = tr("The file has been installed as program");
    }
    else if ((type == "plasma_plasmoids" || type == "plasma4_plasmoids" || type == "plasma5_plasmoids")
             && package.installAsPlasmapkg("plasmoid")) {
        result["message"] = tr("The plasmoid has been installed");
    }
    else if ((type == "plasma_look_and_feel" || type == "plasma5_look_and_feel")
             && package.installAsPlasmapkg("lookandfeel")) {
        result["message"] = tr("The plasma look and feel has been installed");
    }
    else if ((type == "plasma_desktopthemes" || type == "plasma5_desktopthemes")
             && package.installAsPlasmapkg("theme")) {
        result["message"] = tr("The plasma desktop theme has been installed");
    }
    else if (type == "kwin_effects"
             && package.installAsPlasmapkg("kwineffect")) {
        result["message"] = tr("The KWin effect has been installed");
    }
    else if (type == "kwin_scripts"
             && package.installAsPlasmapkg("kwinscript")) {
        result["message"] = tr("The KWin script has been installed");
    }
    else if (type == "kwin_tabbox"
             && package.installAsPlasmapkg("windowswitcher")) {
        result["message"] = tr("The KWin window switcher has been installed");
    }
    else if (package.installAsArchive(destDir.path())) {
        result["message"] = tr("The archive file has been extracted");
    }
    else if (package.installAsFile(destFile.path())) {
        result["message"] = tr("The file has been installed");
    }
    else {
        result["status"] = QString("error_install");
        result["message"] = tr("Failed to installation");
        emit finishedWithError(result);
        tempFile.remove();
        return;
    }

    result["status"] = QString("success_install");
    emit finishedWithSuccess(result);

    tempFile.remove();
    resource->deleteLater();
}
