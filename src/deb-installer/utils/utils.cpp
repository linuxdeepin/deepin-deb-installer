// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "qtcompat.h"
#include "ddlog.h"

#include <mutex>

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QTextCodec>
#include <QProcess>
#include <QStorageInfo>
#include <QDBusInterface>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QThread>

#include <DSysInfo>
#include <DDciIcon>

#include <QApt/Package>

DWIDGET_USE_NAMESPACE

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Utils constructed";
}

QString Utils::loadFontFamilyByType(FontType fontType)
{
    qCDebug(appLog) << "Loading font family for type:" << fontType;
    Q_UNUSED(fontType);
    QFont font;
    return font.family();
}

QFont Utils::loadFontBySizeAndWeight(const QString &fontFamily, int fontSize, int fontWeight)
{
    qCDebug(appLog) << "Loading font by size and weight:" << fontFamily << fontSize << fontWeight;
    Q_UNUSED(fontSize)

    QFont font(fontFamily);
    font.setWeight((QFont::Weight)fontWeight);

    return font;
}

void Utils::bindFontBySizeAndWeight(QWidget *widget, const QString &fontFamily, int fontSize, int fontWeight)
{
    qCDebug(appLog) << "Binding font to widget:" << widget->objectName() << fontFamily << fontSize << fontWeight;
    if (nullptr == widget) {
        qCWarning(appLog) << "Cannot bind font to null widget";
        return;
    }
    QFont font = loadFontBySizeAndWeight(fontFamily, fontSize, fontWeight);
    widget->setFont(font);

    DFontSizeManager::SizeType sizeType = DFontSizeManager::T6;
    switch (fontSize) {
        case 10: {
            sizeType = DFontSizeManager::T10;
        } break;
        case 11: {
            sizeType = DFontSizeManager::T9;
        } break;
        case 12: {
            sizeType = DFontSizeManager::T8;
        } break;
        case 13: {
            sizeType = DFontSizeManager::T7;
        } break;
        case 14: {
            sizeType = DFontSizeManager::T6;
        } break;
        case 17: {
            sizeType = DFontSizeManager::T5;
        } break;
        case 20: {
            sizeType = DFontSizeManager::T4;
        } break;
        case 24: {
            sizeType = DFontSizeManager::T3;
        } break;
        case 30: {
            sizeType = DFontSizeManager::T2;
        } break;
        case 40: {
            sizeType = DFontSizeManager::T1;
        } break;
    }
    qCDebug(appLog) << "Binding font size type" << sizeType << "and weight" << fontWeight;

    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(widget, sizeType, fontWeight);
}

QString Utils::fromSpecialEncoding(const QString &inputStr)
{
    qCDebug(appLog) << "Converting from special encoding";
    bool bFlag = inputStr.contains(REG_EXP("[\\x{4e00}-\\x{9fa5}]+"));
    if (bFlag) {
        qCDebug(appLog) << "Input string contains Chinese characters, returning as is";
        return inputStr;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    if (codec) {
        QString unicodeStr = codec->toUnicode(inputStr.toLatin1());
        qCDebug(appLog) << "Successfully converted to Unicode";
        return unicodeStr;
    } else {
        qCWarning(appLog) << "UTF-8 codec not found, returning original string";
        return inputStr;
    }
}
bool Utils::Return_Digital_Verify(const QString &strfilepath, const QString &strfilename)
{
    qCDebug(appLog) << "Return digital verify, strfilepath:" << strfilepath << ", strfilename:" << strfilename;
    QDir dir(strfilepath);
    if (!dir.exists()) {
        qCDebug(appLog) << "文件夹不存在";
        return false;
    }
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    int file_count = list.count();
    qCDebug(appLog) << "file_count                 " << file_count;
    if (file_count <= 0) {
        qDebug() << "当前文件夹为空";
        return false;
    }
    for (int i = 0; i < list.count(); i++) {
        QFileInfo file_info = list.at(i);
        if (file_info.fileName() == strfilename) {
            qCDebug(appLog) << "文件路径：  " << file_info.path() << "           "
                     << "文件名：  " << file_info.fileName();
            return true;
        }
    }
    return false;
}

Utils::VerifyResultCode Utils::Digital_Verify(const QString &filepath_name)
{
    qCDebug(appLog) << "Digital verify, filepath_name:" << filepath_name;
    QString verifyfilepath = "/usr/bin/";
    QString verifyfilename = "deepin-deb-verify";
    bool result_verify_file = Return_Digital_Verify(verifyfilepath, verifyfilename);
    qCDebug(appLog) << "result_verify_file" << result_verify_file;
    if (result_verify_file) {
        QProcess proc;
        QString program = "/usr/bin/deepin-deb-verify";
        proc.start(program, {filepath_name});
        proc.waitForFinished(-1);
        const QString output1 = proc.readAllStandardError();
        qCInfo(appLog) << "签名校验结果：" << output1;
        for (const auto &item : output1.split('\n')) {
            if (item.toLatin1() == "[INFO] signature verified!") {
                return VerifySuccess;
            }
            if (item.toLatin1() == "cannot find signinfo in deb file") {
                return DebfileInexistence;
            }
            if (item.toLatin1() == "extract deb_file failed!") {
                return ExtractDebFail;
            }
            if (item.toLatin1() == "verify deb file failed!") {
                return DebVerifyFail;
            }
        }
    }
    return OtherError;
}

QString Utils::holdTextInRect(const QFont &font, const QString &srcText, const QSize &size)
{
    qCDebug(appLog) << "Hold text in rect, font:" << font.family() << ", srcText:" << srcText << ", size:" << size;

    if (srcText.isEmpty() || size.width() <= 0 || size.height() <= 0) {
        qCDebug(appLog) << "Invalid input parameters, returning empty or original text";
        return srcText;
    }

    QFontMetrics fm(font);
    const int lineHeight = fm.height();
    const int maxLines = size.height() / lineHeight;

    if (maxLines <= 0) {
        qCDebug(appLog) << "Height too small for any line, returning empty";
        return QString();
    }

    // Use QTextLayout for proper word wrapping
    QTextLayout textLayout(srcText, font);
    textLayout.beginLayout();

    QString resultText;
    int currentLine = 0;
    const int textWidth = size.width();

    while (currentLine < maxLines) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(textWidth);
        const int lineStart = line.textStart();
        const int lineLength = line.textLength();

        // Check if we're at the last allowed line and there's more text
        if (currentLine == maxLines - 1 && lineStart + lineLength < srcText.length()) {
            // Elide the remaining text
            QString remainingText = srcText.mid(lineStart);
            QString elidedText = fm.elidedText(remainingText, Qt::ElideRight, textWidth);
            if (!resultText.isEmpty()) {
                resultText += "\n";
            }
            resultText += elidedText;
            break;
        }

        // Add current line to result
        if (!resultText.isEmpty()) {
            resultText += "\n";
        }
        resultText += srcText.mid(lineStart, lineLength);

        currentLine++;
    }

    textLayout.endLayout();

    qCDebug(appLog) << "Hold text in rect, text:" << resultText;
    return resultText;
}

QString Utils::holdTextInRect(const QFont &font, const QString &srcText, const int &width)
{
    qCDebug(appLog) << "Hold text in rect, font:" << font.family() << ", srcText:" << srcText << ", width:" << width;

    if (srcText.isEmpty() || width <= 0) {
        qCDebug(appLog) << "Invalid input parameters, returning original text";
        return srcText;
    }

    const int effectiveWidth = width - 12;
    if (effectiveWidth <= 0) {
        qCDebug(appLog) << "Width too small after margin, returning original text";
        return srcText;
    }

    // Use QTextLayout for proper word wrapping
    QTextLayout textLayout(srcText, font);
    textLayout.beginLayout();

    QString resultText;

    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(effectiveWidth);
        const int lineStart = line.textStart();
        const int lineLength = line.textLength();

        // Add current line to result
        if (!resultText.isEmpty()) {
            resultText += "\n";
        }
        resultText += srcText.mid(lineStart, lineLength);
    }

    textLayout.endLayout();

    qCDebug(appLog) << "Hold text in rect, text:" << resultText;
    return resultText;
}

/*!
  @brief check `packagePath` is readable, firstly check the mount device information,
    if it is a gvfs or cifs file system(the remote mount system after V6), throw false.
    At the same time, check if the file has permission to install, if not, throw an exception.

  @param[in] packagePath pacakge's path.
  @note only local package can be installed, remote package (samba/ftp) can not be installed.

  @todo Add the check for removable devices.
*/
Pkg::PackageReadability Utils::checkPackageReadable(const QString &packagePath)
{
    qCDebug(appLog) << "Check package readable, packagePath:" << packagePath;
    // Determine whether the route information is a local path
    QStorageInfo info(packagePath);
    QString device = info.device();
    QString fsType = info.fileSystemType();
    // Blacklist identifies, gvfs/cifs as the file system
    // that currently manages the remote directory
    if (fsType.startsWith("gvfs") || fsType.startsWith("cifs")
        || device.startsWith("gvfs") || device.startsWith("cifs")) {
        qCWarning(appLog) << "Disable open remote file, the devices is" << device;
        return Pkg::PkgNotInLocal;
    }

    QFileInfo debFileIfo(packagePath);
    // check package file readable
    QFile outfile(packagePath.toUtf8());
    outfile.open(QFile::ReadOnly);

    // error occurs
    if (!outfile.isOpen()) {
        QFile::FileError error = outfile.error();
        if (error == QFile::FileError::NoError) {
            qCWarning(appLog) << "Package has permission but cannot open!";
            return Pkg::PkgNotInLocal;
        }

        qCWarning(appLog) << "Package has no read permission!";
        return Pkg::PkgNoPermission;
    }

    // file can open, has permission, in local
    outfile.close();
    return Pkg::PkgReadable;
}

int Utils::compareVersion(const QString &v1, const QString &v2)
{
    qCDebug(appLog) << "Comparing versions:" << v1 << "and" << v2;
    return QApt::Package::compareVersion(v1, v2);
}

Pkg::PackageType Utils::detectPackage(const QString &filePath)
{
    qCDebug(appLog) << "Detecting package type for:" << filePath;
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(filePath);
    const QFileInfo info(filePath);

    if (info.suffix().toLower() == "deb" || mime.name().startsWith("application/vnd.debian.binary-package")) {
        qCDebug(appLog) << "Detected DEB package";
        return Pkg::Deb;
    }
    if (info.suffix().toLower() == "uab" || mime.name().startsWith("application/vnd.linyaps.uab")) {
        qCDebug(appLog) << "Detected UAB package";
        return Pkg::Uab;
    }

    qCWarning(appLog) << "Unknown package type for:" << filePath;
    return Pkg::UnknownPackage;
}

/**
 * @brief Get the icon based on the package \a type .
 *  not threadsafe.
 */
QIcon Utils::packageIcon(Pkg::PackageType type)
{
    qCDebug(appLog) << "Package icon, type:" << type;
    if (Pkg::Uab == type) {
        // linglong uab package
        static QIcon kUabIcon = QIcon::fromTheme("application/x-executable");
        if (kUabIcon.isNull()) {
            // using DDciIcon to support new svg version (dsvg use librsvg backend)
            Dtk::Gui::DDciIcon dciIcon(QString(":/icons/deepin/uab/uos-application-bundle.dci"));
            QList<int> availibleSizes = dciIcon.availableSizes(DDciIcon::Light);
            if (!availibleSizes.isEmpty()) {
                kUabIcon = QIcon(dciIcon.pixmap(qApp->devicePixelRatio(), availibleSizes.first(), DDciIcon::Light));
            }
        }
        qCDebug(appLog) << "return uab package icon, type:" << type;
        return kUabIcon;

    } else {
        // default, deb package
        static const QIcon kDebIcon = QIcon::fromTheme("application-x-deb");
        qCDebug(appLog) << "return deb package icon, type:" << type;
        return kDebIcon;
    }
}

/**
   @brief Check if debian package contains DebConf templates config file
 */
bool Utils::checkPackageContainsDebConf(const QString &filePath)
{
    qCDebug(appLog) << "Checking if package contains DebConf templates:" << filePath;
    // create template dir
    QTemporaryDir templateDir;
    if (!templateDir.isValid()) {
        qWarning() << "check error mkdir failed, error:" << templateDir.errorString();
        return false;
    }

    QProcess dpkgProcess;
    dpkgProcess.start("dpkg", {"-e", filePath, templateDir.path()});
    dpkgProcess.waitForFinished();
    const QByteArray errorOutput = dpkgProcess.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "DebListModel:"
                   << "Failed to decompress the main control file" << errorOutput;
    }

    const bool exists = QFile::exists(templateDir.filePath("templates"));
    qCDebug(appLog) << "Templates file exists:" << exists;
    return exists;
}

/**
   @return Check if the current mode is development mode, the status will be called once and stored.
 */
bool Utils::isDevelopMode()
{
    qCDebug(appLog) << "Check if the current mode is development mode";
    static bool kIsDevelopMode = false;
    static std::once_flag kDevelopOnceFlag;
    std::call_once(kDevelopOnceFlag, [&]() {
    // Add for judge OS Version
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 2, 2, 2))
        qCDebug(appLog) << "system code(UOS): " << Dtk::Core::DSysInfo::uosEditionType();
        switch (Dtk::Core::DSysInfo::uosEditionType()) {
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 4, 10, 0))
            case Dtk::Core::DSysInfo::UosEducation:
            case Dtk::Core::DSysInfo::UosDeviceEdition:
#endif
            case Dtk::Core::DSysInfo::UosProfessional:
            case Dtk::Core::DSysInfo::UosHome: {
                // Check if current is develop mode
                QDBusInterface *dbusInterFace = new QDBusInterface(
                    "com.deepin.sync.Helper", "/com/deepin/sync/Helper", "com.deepin.sync.Helper", QDBusConnection::systemBus());
                bool deviceMode = dbusInterFace->property("DeveloperMode").toBool();
                qCDebug(appLog) << "DebListModel:"
                        << "system editon:" << Dtk::Core::DSysInfo::uosEditionName() << "develop mode:" << deviceMode;
                kIsDevelopMode = deviceMode;
                delete dbusInterFace;
                break;
            }
            case Dtk::Core::DSysInfo::UosCommunity:   // The community edition does not need signature verification
            case Dtk::Core::DSysInfo::UosEnterprise:  // Server Version
                kIsDevelopMode = true;
                break;
            default:
                kIsDevelopMode = true;
                break;
        }
#else
        qCDebug(appLog) << "system code(Deepin): " << Dtk::Core::DSysInfo::deepinType();
        switch (Dtk::Core::DSysInfo::deepinType()) {
        case Dtk::Core::DSysInfo::DeepinDesktop:
            qCDebug(appLog) << "DeepinDesktop";
            kIsDevelopMode = true;
            break;
        case Dtk::Core::DSysInfo::DeepinPersonal:
        case Dtk::Core::DSysInfo::DeepinProfessional:
            qCDebug(appLog) << "DeepinProfessional";
            // Check if develop mode
            QDBusInterface *dbusInterFace = new QDBusInterface("com.deepin.deepinid", "/com/deepin/deepinid", "com.deepin.deepinid");
            bool deviceMode = dbusInterFace->property("DeviceUnlocked").toBool();
            qInfo() << "DebListModel:" << "system editon:" << Dtk::Core::DSysInfo::uosEditionName() << "develop mode:" << deviceMode;
            kIsDevelopMode = deviceMode;
            delete dbusInterFace;
            break;
        case Dtk::Core::DSysInfo::isCommunityEdition():
        case Dtk::Core::DSysInfo::DeepinServer:
            qCDebug(appLog) << "DeepinServer";
            kIsDevelopMode = true;
            break;
        default:
            qCDebug(appLog) << "Default";
            kIsDevelopMode = true;
            break;
        }

#endif
    });

    return kIsDevelopMode;
}

/**
 * @return word wrap \a text acording to \a textWidth, this function not threadsafe
 */
QString Utils::formatWrapText(const QString &text, int textWidth)
{
    qCDebug(appLog) << "Format wrap text, text:" << text << ", textWidth:" << textWidth;
    // GUI thread only
    if (QThread::currentThread() != qApp->thread()) {
        qCDebug(appLog) << "Not in GUI thread, return text";
        return text;
    }

    if (text.isEmpty() || !textWidth) {
        qCDebug(appLog) << "Text is empty or textWidth is 0, return text";
        return text;
    }

    QString tipsText;

    static QTextDocument kFormatDoc;
    kFormatDoc.setTextWidth(textWidth);
    kFormatDoc.setPlainText(text);
    // call size trigger internal layout
    kFormatDoc.size();

    QTextBlock block = kFormatDoc.firstBlock();
    while (block.isValid()) {
        if (QTextLayout *textLay = block.layout()) {
            for (int i = 0; i < textLay->lineCount(); ++i) {
                QTextLine line = textLay->lineAt(i);
                line.textStart();

                if (!tipsText.isEmpty()) {
                    tipsText.append('\n');
                }
                tipsText.append(text.mid(line.textStart(), line.textLength()));
            }
        }

        block = block.next();
    }

    qCDebug(appLog) << "Format wrap text, tipsText:" << tipsText;
    return tipsText;
}

/**
 * @return parse app black list from config file.
 */
QStringList Utils::parseBlackList()
{
    qCDebug(appLog) << "Parse black list";
    static const QString kBlackFileV20 {"/usr/share/udcp/appblacklist.txt"};
    static const QString kBlackFileV25 {"/var/lib/udcp/appblacklist.txt"};
    static const int kV25Version = 25;

    const int sysVersion = Dtk::Core::DSysInfo::majorVersion().toInt();
    QString blackFile = (sysVersion >= kV25Version) ? kBlackFileV25 : kBlackFileV20;
    QFile blackListFile(blackFile);

    if (blackListFile.exists()) {
        blackListFile.open(QFile::ReadOnly);
        QString blackApplications = blackListFile.readAll();
        blackApplications.replace(" ", "");
        blackApplications = blackApplications.replace("\n", "");
        blackListFile.close();
        return blackApplications.split(",");
    }

    qCDebug(appLog) << "Black File not Found, " << blackFile;
    return {};
}

/**
 * @brief Mark whether the wine pre-dependency is being installed.
 *        Not thread-safe.
 */
static bool kWinePreDependsStatus = false;

bool GlobalStatus::winePreDependsInstalling()
{
    qCDebug(appLog) << "Wine pre-dependency installing, status:" << kWinePreDependsStatus;
    return kWinePreDependsStatus;
}

void GlobalStatus::setWinePreDependsInstalling(bool b)
{
    qCDebug(appLog) << "Set wine pre-dependency installing, status:" << b;
    kWinePreDependsStatus = b;
}

DebApplicationHelper *DebApplicationHelper::instance()
{
    // qCDebug(appLog) << "Get deb application helper instance";
    static DebApplicationHelper *phelper = new DebApplicationHelper;

    return phelper;
}

#define CAST_INT static_cast<int>

static QColor light_qpalette[QPalette::NColorRoles]{
    QColor("#414d68"),                           // WindowText
    QColor("#e5e5e5"),                           // Button
    QColor("#e6e6e6"),                           // Light
    QColor("#e5e5e5"),                           // Midlight
    QColor("#001A2E"),                           // Dark  -- changed origin is #e3e3e3
    QColor("#e4e4e4"),                           // Mid
    QColor("#414d68"),                           // Text
    Qt::black,                                   // BrightText
    QColor("#414d68"),                           // ButtonText
    Qt::white,                                   // Base
    QColor("#f8f8f8"),                           // Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),       // Shadow
    QColor("#0081ff"),                           // Highlight
    QColor(0, 45, 255, CAST_INT(0.5 * 255)),     // HighlightedText   //old Qt::white
    QColor("#0082fa"),                           // Link
    QColor("#ad4579"),                           // LinkVisited
    QColor(0, 0, 0, CAST_INT(0.03 * 255)),       // AlternateBase
    Qt::white,                                   // NoRole
    QColor(255, 255, 255, CAST_INT(0.8 * 255)),  // ToolTipBase
    QColor("#526A7F")                            // ToolTipText -- changed origin is Qt::black
};

static QColor dark_qpalette[QPalette::NColorRoles]{
    QColor("#c0c6d4"),                            // WindowText
    QColor("#444444"),                            // Button
    QColor("#484848"),                            // Light
    QColor("#474747"),                            // Midlight
    QColor("#C0C6D4"),                            // Dark      -- changed origin is #414141
    QColor("#434343"),                            // Mid
    QColor("#c0c6d4"),                            // Text
    Qt::white,                                    // BrightText
    QColor("#c0c6d4"),                            // ButtonText
    QColor(255, 255, 255, CAST_INT(0.05 * 255)),  // Base --- changed origin is #282828
    QColor("#252525"),                            // Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),        // Shadow
    QColor("#095EFF"),                            // Highlight         //old : #0081ff
    QColor("#0059D2"),                            // HighlightedText   //old: b8d3ff
    QColor("#0082fa"),                            // Link
    QColor("#ad4579"),                            // LinkVisited
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),        // AlternateBase
    Qt::black,                                    // NoRole
    QColor(45, 45, 45, CAST_INT(0.8 * 255)),      // ToolTipBase
    QColor("#6D7C88")                             // ToolTipText -- changed origin is #c0c6d4
};

static QColor light_dpalette[DPalette::NColorTypes]{
    QColor(),                               // NoType
    QColor(0, 0, 0, CAST_INT(255 * 0.03)),  // ItemBackground
    QColor("#414d68"),                      // TextTitle
    QColor("#609DC8"),                      // TextTips --- changed origin is #526A7F
    QColor("#FF5A5A"),                      // TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                      // TextLively  -- changed origin is #0082FA
    QColor("#417505"),                      // LightLively -- changed origin is #25b7ff
    QColor("#47790C"),                      // DarkLively -- changed origin is #0098ff
    QColor(0, 0, 0, CAST_INT(0.03 * 255))   // FrameBorder
};

static QColor dark_dpalette[DPalette::NColorTypes]{
    QColor(),                                     // NoType
    QColor(255, 255, 255, CAST_INT(255 * 0.05)),  // ItemBackground
    QColor("#c0c6d4"),                            // TextTitle
    QColor("#6D7C88"),                            // TextTips
    QColor("#9A2F2F"),                            // TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                            // TextLively -- changed origin is #0082FA
    QColor("#417505"),                            // LightLively -- changed origin is #0056c1
    QColor("#47790C"),                            // DarkLively  -- changed origin is #004c9c
    QColor(0, 0, 0, CAST_INT(0.08 * 255))         // FrameBorder
};

DPalette DebApplicationHelper::standardPalette(DGuiApplicationHelper::ColorType type) const
{
    qCDebug(appLog) << "Get deb application helper instance";
    DPalette pa;
    const QColor *qcolor_list, *dcolor_list;

    if (type == DarkType) {
        qCDebug(appLog) << "Dark type";
        qcolor_list = dark_qpalette;
        dcolor_list = dark_dpalette;
    } else {
        qCDebug(appLog) << "Light type";
        qcolor_list = light_qpalette;
        dcolor_list = light_dpalette;
    }

    for (int i = 0; i < DPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);

        QColor color = qcolor_list[i];
        pa.setColor(DPalette::Active, role, color);
        generatePaletteColor(pa, role, type);
    }

    for (int i = 0; i < DPalette::NColorTypes; ++i) {
        DPalette::ColorType role = static_cast<DPalette::ColorType>(i);

        QColor color = dcolor_list[i];
        pa.setColor(DPalette::Active, role, color);
        generatePaletteColor(pa, role, type);
    }

    return *const_cast<const DPalette *>(&pa);
}

DPalette DebApplicationHelper::palette(const QWidget *widget, const QPalette &base) const
{
    // qCDebug(appLog) << "Get palette, widget:" << widget << ", base:" << base;
    Q_UNUSED(base)

    DPalette palette;

    do {
        // 存在自定义palette时应该根据其自定义的palette获取对应色调的DPalette
        const QPalette &wp = widget->palette();

        palette = standardPalette(toColorType(wp));

        // 关注控件palette改变的事件
        const_cast<QWidget *>(widget)->installEventFilter(const_cast<DebApplicationHelper *>(this));
    } while (false);

    return palette;
}

void DebApplicationHelper::setPalette(QWidget *widget, const DPalette &palette)
{
    // qCDebug(appLog) << "Set palette, widget:" << widget << ", palette:" << palette;
    // 记录此控件被设置过palette
    if (nullptr == widget)
        return;
    widget->setProperty("_d_set_palette", true);
    widget->setPalette(palette);
}

void DebApplicationHelper::resetPalette(QWidget *widget)
{
    // qCDebug(appLog) << "Reset palette, widget:" << widget;
    if (nullptr == widget)
        return;
    widget->setProperty("_d_set_palette", QVariant());
    widget->setAttribute(Qt::WA_SetPalette, false);
}
