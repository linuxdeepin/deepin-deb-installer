// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uab_process_controller.h"

#include <QRegularExpression>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "utils/ddlog.h"

#include "uab_backend.h"
#include "process/Pty.h"

namespace Uab {

// linglong cli command
const QString kLinglongBin = "ll-cli";
const QString kLinglongJson = "--json";
const QString kLinglongInstall = "install";
const QString kLinglongUninstall = "uninstall";

// ll-cli output json tag
const QString kJsonPercentage = "percentage";
const QString kJsonState = "state";
const QString kJsonCode = "code";
const QString kJsonMessage = "messsage";

// transport install/uninstall task to higher level permission process.
static const QString kPkexecBin = "pkexec";
static const QString kInstallProcessorBin = "deepin-deb-installer-dependsInstall";
static const QString kParamUab = "--uab";
static const QString kParamInstall = "--install";
static const QString kParamRemove = "--remove";

const int kInitedIndex = -1;

/**
 * @class UabProcessController
 * @brief Uab process controller. It manages the process of installing/uninstalling uab packages.
 *        Its uses `deepin-deb-installer-dependsInstall` command to install/uninstall uab packages
 *        with higher level permission.
 */
UabProcessController::UabProcessController(QObject *parent)
    : QObject{parent}
{
    qCDebug(appLog) << "Creating UabProcessController";
    setProcessType(BackendCli);
    qCDebug(appLog) << "Process controller type set to BackendCli";
}

void UabProcessController::setProcessType(ProcessType type)
{
    qCDebug(appLog) << "Setting process type from" << m_type << "to" << type;
    if (type == m_type) {
        qCDebug(appLog) << "Process type is the same, return";
        return;
    }
    m_type = type;
    qCDebug(appLog) << "Process type set to" << type;
}

UabProcessController::ProcessType UabProcessController::processType() const
{
    qCDebug(appLog) << "Getting process type:" << m_type;
    return m_type;
}

UabProcessController::ProcFlags UabProcessController::procFlag() const
{
    qCDebug(appLog) << "Getting process flag:" << m_procFlag;
    return m_procFlag;
}

bool UabProcessController::isRunning() const
{
    qCDebug(appLog) << "Checking if process is running";
    if (m_procFlag & (Uninstalling | Installing | Processing)) {
        qCDebug(appLog) << "Process is running (flag set)";
        return true;
    }

    if (m_process && QProcess::NotRunning != m_process->state()) {
        qCDebug(appLog) << "Process is running (QProcess state)";
        return true;
    }

    qCDebug(appLog) << "Process is not running";
    return false;
}

bool UabProcessController::reset()
{
    qCDebug(appLog) << "Resetting process controller";
    if (isRunning() || !ensureProcess()) {
        qCWarning(appLog) << "Cannot reset - process running or cannot ensure process";
        return false;
    }

    m_currentIndex = kInitedIndex;
    m_procList.clear();
    qCDebug(appLog) << "Process controller reset successfully";

    return true;
}

bool Uab::UabProcessController::markInstall(const UabPackage::Ptr &installPtr)
{
    qCDebug(appLog) << "Marking package for installation:" << (installPtr ? installPtr->info()->id : "null");
    if (isRunning() || !ensureProcess() || !installPtr || !installPtr->isValid()) {
        qCWarning(appLog) << "Cannot mark for installation - process running, not ready, or invalid package";
        return false;
    }

    m_procList.append(qMakePair(Installing, installPtr));
    qCDebug(appLog) << "Package marked for installation:" << installPtr->info()->id;
    return true;
}

bool Uab::UabProcessController::markUninstall(const UabPackage::Ptr &uninstallPtr)
{
    qCDebug(appLog) << "Marking package for uninstallation:" << (uninstallPtr ? uninstallPtr->info()->id : "null");
    if (isRunning() || !ensureProcess() || !uninstallPtr || !uninstallPtr->isValid()) {
        qCWarning(appLog) << "Cannot mark for uninstallation - process running, not ready, or invalid package";
        return false;
    }

    m_procList.append(qMakePair(Uninstalling, uninstallPtr));
    qCDebug(appLog) << "Package marked for uninstallation:" << uninstallPtr->info()->id;
    return true;
}

bool UabProcessController::commitChanges()
{
    qCDebug(appLog) << "Committing changes, process list size:" << m_procList.size();
    if (isRunning() || !ensureProcess() || m_procList.isEmpty()) {
        qCWarning(appLog) << "Cannot commit changes - process running/not ready or empty list";
        return false;
    }

    m_procFlag = Processing;
    m_currentIndex = kInitedIndex;
    qCDebug(appLog) << "Starting first process in list";
    if (!nextProcess()) {
        qCWarning(appLog) << "Failed to start first process";
        m_procFlag = Error;
        return false;
    }

    Q_EMIT processStart();
    qCDebug(appLog) << "Process started successfully";
    return true;
}

bool Uab::UabProcessController::ensureProcess()
{
    qCDebug(appLog) << "Ensuring process";
    if (!m_process) {
        qCDebug(appLog) << "Creating new process";
        m_process = new Konsole::Pty(this);

        connect(m_process, &Konsole::Pty::receivedData, this, &UabProcessController::onReadOutput);
        connect(
            m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UabProcessController::onFinished);
    } else if (QProcess::NotRunning != m_process->state()) {
        qCWarning(appLog) << "Unable to restart uab process is still running";
        return false;
    }

    return true;
}

/**
   @brief Parse json data from `ll-cli --json install/uninstall [path]` output.
    Here are output example (linglong-bin 1.6.2).
   @code
    // progress
    [
        {
            "message": "prepare for installing uab",
            "percentage": "0",
            "state": "preInstall"
        }
    ]

    // successed
    {
        "message": "install uab successfully",
        "percentage": "100",
        "state": "Success"
    }

    // failed
    {"code":-1,"message":"./libs/linglong/src/linglong/cli/cli.cpp:125 download status:
   \n./libs/linglong/src/linglong/repo/ostree_repo.cpp:915 import layer dir: main:org.deepin.editor/6.5.2.1/x86_64 exists."}

   @endcode
 */
void UabProcessController::parseProgressFromJson(const QByteArray &jsonData)
{
    qCDebug(appLog) << "Parsing progress from JSON data:" << jsonData;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (doc.isArray()) {
        qCDebug(appLog) << "JSON data is an array";
        // maybe percentage info
        const QJsonObject obj = doc.array().first().toObject();
        if (obj.contains(kJsonPercentage)) {
            float progress = obj.value(kJsonPercentage).toDouble();
            qCDebug(appLog) << "Found progress in JSON array:" << progress;
            updateWholeProgress(progress);
        }

    } else if (doc.isObject()) {
        qCDebug(appLog) << "JSON data is an object";
        // maybe percentage / code info
        const QJsonObject obj = doc.object();

        if (obj.contains(kJsonPercentage)) {
            float progress = obj.value(kJsonPercentage).toDouble();
            qCDebug(appLog) << "Found progress in JSON object:" << progress;
            updateWholeProgress(progress);

        } else if (UabError == obj.value(kJsonCode).toInt()) {
            const QString errorString = obj.value(kJsonMessage).toString();
            qCWarning(appLog) << "Found error in JSON object:" << errorString;
            auto currUabPtr = currentPackagePtr();
            if (currUabPtr) {
                currUabPtr->setProcessError(Pkg::UnknownError, errorString);
            }

            qCWarning(appLog) << "Uab process error:" << errorString;
        }

        // TODO(renbin): signature verify error, etc.
    } else {
        qCWarning(appLog) << "JSON data is not an array or object";
    }
}

void UabProcessController::parseProgressFromRawOutput(const QByteArray &output)
{
    qCDebug(appLog) << "Parsing progress from raw output:" << output;
    // Detect progress change, filter ANSI code, receive data such as:
    // "\r\x1B[K\x1B[?25l0% prepare for installing uab\x1B[?25h"
    // "\r\x1B[K\x1B[?25l100% install uab successfully\x1B[?25h\n"
    //
    // pattern: ^\r?(\x1B\[\W?\d*\w)+(\d+)%
    static const QRegularExpression kRegExp("^\\r?(\\x1B\\[\\W?\\d*\\w)+(\\d+)%");
    QRegularExpressionMatch match = kRegExp.match(QString(output), 0, QRegularExpression::PartialPreferFirstMatch);
    if (match.hasMatch()) {
        // get (\\d+) capture
        float progress = match.captured(match.lastCapturedIndex()).toFloat();
        qCDebug(appLog) << "Found progress in raw output:" << progress;
        updateWholeProgress(progress);
    }
}

void UabProcessController::updateWholeProgress(float currentTaskProgress)
{
    qCDebug(appLog) << "Updating whole progress with current task progress:" << currentTaskProgress;
    const int count = m_procList.size();
    if (count > 1) {
        currentTaskProgress = (m_currentIndex * 100.0f + currentTaskProgress) / static_cast<float>(count);
        qCDebug(appLog) << "Calculated whole progress:" << currentTaskProgress;
    }

    Q_EMIT progressChanged(currentTaskProgress);
}

void UabProcessController::onReadOutput(const char *buffer, int length, bool isCommandExec)
{
    qCDebug(appLog) << "Read output from process, length:" << length << "isCommandExec:" << isCommandExec;
    Q_UNUSED(isCommandExec)
    QByteArray output = QByteArray::fromRawData(buffer, length);
    Q_EMIT processOutput(output);

    // e.g: ll-cli --json install /path/to/file
    parseProgressFromJson(output);
}

void UabProcessController::onFinished(int exitCode, int exitStatus)
{
    qCDebug(appLog) << "Process finished with exit code:" << exitCode << "exit status:" << exitStatus;
    Q_UNUSED(exitStatus)

    qCDebug(appLog) << "Process finished with exit code:" << exitCode;
    const bool exitSuccess = UabSuccess == exitCode;

    // continue next process
    if (exitSuccess) {
        qCDebug(appLog) << "Process completed successfully, updating backend";
        // update uab backend
        commitCurrentChangeToBackend();

        qCDebug(appLog) << "Attempting to start next process";
        // continue next process, if error occurred, terminate current install/uninstall.
        if (nextProcess()) {
            return;
        }
    }

    qCWarning(appLog) << "Process failed or no more processes, error flag set";
    m_procFlag = Error;
    Q_EMIT processFinished(false);
}

void UabProcessController::onDBusProgressChanged(int progress, const QString &message)
{
    qCDebug(appLog) << "D-Bus progress changed:" << progress << "message:" << message;
    updateWholeProgress(static_cast<float>(progress));
    Q_EMIT processOutput(message);
}

bool UabProcessController::nextProcess()
{
    qCDebug(appLog) << "Next process";
    m_procFlag.setFlag(Installing, false);
    m_procFlag.setFlag(Uninstalling, false);

    // check process finish
    m_currentIndex++;
    if (m_currentIndex >= m_procList.size()) {
        qCDebug(appLog) << "Current index is greater than or equal to process list size, set process flag to Finish";
        m_procFlag = Finish;
        Q_EMIT processFinished(true);
        return true;
    }

    if (!checkIndexValid()) {
        qCWarning(appLog) << "Invalid process index:" << m_currentIndex << "for package:" << (currentPackagePtr() ? currentPackagePtr()->info()->id : "null");
        return false;
    }

    const auto &currentProc = m_procList.at(m_currentIndex);
    switch (currentProc.first) {
        case Installing:
            qCDebug(appLog) << "Installing process";
            return (BackendCli == m_type) ? installBackendCliImpl(currentProc.second) : installCliImpl(currentProc.second);
        case Uninstalling:
            qCDebug(appLog) << "Uninstalling process";
            return (BackendCli == m_type) ? uninstallBackendCliImpl(currentProc.second) : uninstallCliImpl(currentProc.second);
        default:
            break;
    }

    qCWarning(appLog) << "Invalid process type:" << currentProc.first << "for package:" << (currentProc.second ? currentProc.second->info()->id : "null");
    return false;
}

bool UabProcessController::installBackendCliImpl(const UabPackage::Ptr &installPtr)
{
    qCDebug(appLog) << "Preparing backend CLI install for package:" << installPtr->info()->id << installPtr->info()->version;
    if (!installPtr || !installPtr->isValid() || installPtr->info()->filePath.isEmpty()) {
        qCWarning(appLog) << "Invalid package for installation";
        return false;
    }

    qCDebug(appLog) << "Preparing backend CLI install for package:"
                 << installPtr->info()->id << installPtr->info()->version;
    m_procFlag.setFlag(Installing);

    // e.g.: pkexec deepin-deb-installer-dependsInstall --uab --install [file to package].uab
    qCDebug(appLog) << "Starting install process with pkexec";
    m_process->start(
        kPkexecBin, {kPkexecBin, kInstallProcessorBin, kParamUab, kParamInstall, installPtr->info()->filePath}, {}, 0, false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5[uab package]")
                                      .arg(kInstallProcessorBin)
                                      .arg(kParamUab)
                                      .arg(kParamInstall)
                                      .arg(installPtr->info()->id)
                                      .arg(installPtr->info()->version);
    qCInfo(appLog) << "Install command:" << recordCommand;

    return true;
}

bool UabProcessController::uninstallBackendCliImpl(const UabPackage::Ptr &uninstallPtr)
{
    qCDebug(appLog) << "Preparing backend CLI uninstall for package:" << uninstallPtr->info()->id << uninstallPtr->info()->version;
    if (!uninstallPtr || !uninstallPtr->isValid()) {
        qCWarning(appLog) << "Invalid package for uninstallation";
        return false;
    }

    qCDebug(appLog) << "Preparing backend CLI uninstall for package:"
                << uninstallPtr->info()->id << uninstallPtr->info()->version;
    m_procFlag.setFlag(Uninstalling);

    // e.g.: pkexec deepin-deb-installer-dependsInstall --uab --remove [id/version]
    qCDebug(appLog) << "Starting uninstall process with pkexec";
    const QString mergeInfo = QString("%1/%2").arg(uninstallPtr->info()->id).arg(uninstallPtr->info()->version);
    m_process->start(kPkexecBin, {kPkexecBin, kInstallProcessorBin, kParamUab, kParamRemove, mergeInfo}, {}, 0, false);

    const QString recordCommand =
        QString("command: %1 %2 %3 %4").arg(kInstallProcessorBin).arg(kParamUab).arg(kParamRemove).arg(mergeInfo);
    Q_EMIT processOutput(recordCommand);
    qCInfo(appLog) << "Uninstall command:" << recordCommand;

    return true;
}

bool Uab::UabProcessController::installCliImpl(const Uab::UabPackage::Ptr &installPtr)
{
    qCDebug(appLog) << "Preparing CLI install for package:" << installPtr->info()->id << installPtr->info()->version;
    if (!installPtr || !installPtr->isValid() || installPtr->info()->filePath.isEmpty()) {
        return false;
    }

    m_procFlag.setFlag(Installing);

    // e.g.: ll-cli --json install ./path/to/file/uab_package.uab
    m_process->start(kLinglongBin, {kLinglongBin, kLinglongJson, kLinglongInstall, installPtr->info()->filePath}, {}, 0, false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5[uab package]")
                                      .arg(kLinglongBin)
                                      .arg(kLinglongJson)
                                      .arg(kLinglongInstall)
                                      .arg(installPtr->info()->id)
                                      .arg(installPtr->info()->version);
    Q_EMIT processOutput(recordCommand);
    qCInfo(appLog) << recordCommand;

    return true;
}

bool Uab::UabProcessController::uninstallCliImpl(const Uab::UabPackage::Ptr &uninstallPtr)
{
    qCDebug(appLog) << "Preparing CLI uninstall for package:" << uninstallPtr->info()->id << uninstallPtr->info()->version;
    if (!uninstallPtr || !uninstallPtr->isValid()) {
        return false;
    }

    m_procFlag.setFlag(Uninstalling);

    // e.g.: ll-cli --json uninstall org.deepin.package/1.0.0
    m_process->start(kLinglongBin,
                     {kLinglongBin,
                      kLinglongJson,
                      kLinglongUninstall,
                      QString("%1/%2").arg(uninstallPtr->info()->id).arg(uninstallPtr->info()->version)},
                     {},
                     0,
                     false);

    const QString recordCommand = QString("command: %1 %2 %3 %4/%5")
                                      .arg(kLinglongBin)
                                      .arg(kLinglongJson)
                                      .arg(kLinglongUninstall)
                                      .arg(uninstallPtr->info()->id)
                                      .arg(uninstallPtr->info()->version);
    Q_EMIT processOutput(recordCommand);
    qCInfo(appLog) << recordCommand;

    return true;
}

bool UabProcessController::checkIndexValid()
{
    const bool isValid = 0 <= m_currentIndex && m_currentIndex < m_procList.size();
    // qCDebug(appLog) << "Checking if index" << m_currentIndex << "is valid:" << isValid;
    return isValid;
}

UabPackage::Ptr UabProcessController::currentPackagePtr()
{
    qCDebug(appLog) << "Getting current package pointer at index:" << m_currentIndex;
    if (!checkIndexValid()) {
        qCWarning(appLog) << "Invalid index, cannot get current package pointer";
        return {};
    }

    return m_procList[m_currentIndex].second;
}

void UabProcessController::commitCurrentChangeToBackend()
{
    qCDebug(appLog) << "Committing current change to backend";
    if (!checkIndexValid()) {
        qCWarning(appLog) << "Invalid index, cannot commit change";
        return;
    }

    const auto &currentProc = m_procList.at(m_currentIndex);
    UabPkgInfo::Ptr infoPtr;
    if (!currentProc.second || !currentProc.second->isValid()) {
        qCWarning(appLog) << "Invalid package, cannot commit change";
        return;
    }
    infoPtr = currentProc.second->info();

    switch (currentProc.first) {
        case Installing:
            qCDebug(appLog) << "Committing installation for package:" << infoPtr->id;
            Uab::UabBackend::instance()->packageInstalled(infoPtr);
            break;
        case Uninstalling:
            qCDebug(appLog) << "Committing uninstallation for package:" << infoPtr->id;
            Uab::UabBackend::instance()->packageRemoved(infoPtr);
            break;
        default:
            qCWarning(appLog) << "Unhandled process type:" << currentProc.first;
            break;
    }
}

}  // namespace Uab
