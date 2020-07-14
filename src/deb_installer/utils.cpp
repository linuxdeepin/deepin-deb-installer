/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 * Maintainer: rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

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
QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

QString Utils::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

bool Utils::isFontMimeType(const QString &filePath)
{
    const QString mimeName = QMimeDatabase().mimeTypeForFile(filePath).name();

    if (mimeName.startsWith("font/") ||
            mimeName.startsWith("application/x-font")) {
        return true;
    }
    return false;
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}

QString Utils::loadFontFamilyByType(FontType fontType)
{
    QString fontFileName = "";
    switch (fontType) {
    case SourceHanSansMedium:
        fontFileName = ":/font/SourceHanSansCN-Medium.ttf";
        break;
    case SourceHanSansNormal:
        fontFileName = ":/font/SourceHanSansCN-Normal.ttf";
        break;
    case DefautFont:
        QFont font;
        return font.family();
    }

    if (m_fontNameCache.contains(fontFileName)) {
        return m_fontNameCache.value(fontFileName);
    }

    QString fontFamilyName = "";
    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Open font file error";
        return fontFamilyName;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        fontFamilyName = loadedFontFamilies.at(0);
    }
    fontFile.close();

    m_fontNameCache.insert(fontFileName, fontFamilyName);
    return fontFamilyName;
}

QFont Utils::loadFontBySizeAndWeight(QString fontFamily, int fontSize, int fontWeight)
{
    Q_UNUSED(fontSize)

    QFont font(fontFamily);
    font.setWeight(fontWeight);

    return font;
}

void Utils::bindFontBySizeAndWeight(QWidget *widget, QString fontFamily, int fontSize, int fontWeight)
{
    QFont font = loadFontBySizeAndWeight(fontFamily, fontSize, fontWeight);
    widget->setFont(font);

    DFontSizeManager::SizeType sizeType = DFontSizeManager::T6;
    switch (fontSize) {
    case 10: {
        sizeType = DFontSizeManager::T10;
    }
    break;
    case 11: {
        sizeType = DFontSizeManager::T9;
    }
    break;
    case 12: {
        sizeType = DFontSizeManager::T8;
    }
    break;
    case 13: {
        sizeType = DFontSizeManager::T7;
    }
    break;
    case 14: {
        sizeType = DFontSizeManager::T6;
    }
    break;
    case 17: {
        sizeType = DFontSizeManager::T5;
    }
    break;
    case 20: {
        sizeType = DFontSizeManager::T4;
    }
    break;
    case 24: {
        sizeType = DFontSizeManager::T3;
    }
    break;
    case 30: {
        sizeType = DFontSizeManager::T2;
    }
    break;
    case 40: {
        sizeType = DFontSizeManager::T1;
    }
    break;
    }

    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(widget, sizeType, fontWeight);
}

int Utils::returnfileIsempty(QString strfilepath, QString strfilename)
{
    QDir dir(strfilepath);
    QString filename = strfilename + ".postinst";
    do {
        if (!dir.exists()) {
            qDebug() << "文件夹不存在";
            return 0;
        }
        dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList list = dir.entryInfoList();
        int file_count = list.count();
        qDebug() << "file_count                 " << file_count;
        if (file_count <= 0) {
            qDebug() << "当前文件夹为空";
            return 0;
        }
        QStringList string_list;
        for (int i = 0; i < list.count(); i++) {
            QFileInfo file_info = list.at(i);
            if (file_info.fileName() == filename) {
                qDebug() << "文件路径：  " << file_info.path() << "           " << "文件名：  " << file_info.fileName();
                break;
            }
        }
    } while (0);
    return 1;
}

QString Utils::fromSpecialEncoding(const QString &inputStr)
{
    bool bFlag = inputStr.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
    if (bFlag) {
        return inputStr;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    if (codec) {
        QString unicodeStr =  codec->toUnicode(inputStr.toLatin1());
        return unicodeStr;
    } else {
        return inputStr;
    }
}

bool Utils::File_transfer(QString Sourcefilepath, QString Targetfilepath, QString strfilename)
{
    QDir dir(Targetfilepath);
    QString filename = strfilename + ".postinst";
    QString File_transfer_Action1 = "";
    QString File_transfer_Action2 = "";

    File_transfer_Action1 = "mkdir " + Targetfilepath;
    qDebug() << "创建文件夹：" << File_transfer_Action1;
    QFile file(Targetfilepath);
    if (!file.exists()) {
        system(File_transfer_Action1.toStdString().c_str());
    }
    File_transfer_Action2 = "cp " + Sourcefilepath + "/" + filename + " " + Targetfilepath;
    system(File_transfer_Action2.toStdString().c_str());
    qDebug() << "文件复制转移：" << File_transfer_Action2;
    return true;
}

bool Utils::Modify_transferfile(QString Targetfilepath, QString strfilename)
{
    QDir dir(Targetfilepath);
    QString filename = strfilename + ".postinst";
    QString File_modify_Action = "";
    File_modify_Action = "sed -i '1,$s/su /#su /g' " + Targetfilepath + "/" + filename;
    qDebug() << "修改文件内容：" << File_modify_Action;
    QFile file(Targetfilepath + "/" + filename);
    if (file.exists()) {
        system(File_modify_Action.toStdString().c_str());
    }
    return true;
}

bool Utils::Return_Digital_Verify(QString strfilepath, QString strfilename)
{
    QDir dir(strfilepath);
    if (!dir.exists()) {
        qDebug() << "文件夹不存在";
        return false;
    }
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList list = dir.entryInfoList();
    int file_count = list.count();
    qDebug() << "file_count                 " << file_count;
    if (file_count <= 0) {
        qDebug() << "当前文件夹为空";
        return false;
    }
    QStringList string_list;
    for (int i = 0; i < list.count(); i++) {
        QFileInfo file_info = list.at(i);
        if (file_info.fileName() == strfilename) {
            qDebug() << "文件路径：  " << file_info.path() << "           " << "文件名：  " << file_info.fileName();
            return true;
        }
    }
    return false;
}
int Utils::Digital_Verify(QString filepath_name)
{
    QString verifyfilepath = "/usr/bin/";
    QString verifyfilename = "deepin-deb-verify";
    bool result_verify_file = Return_Digital_Verify(verifyfilepath, verifyfilename);
    qDebug() << "result_verify_file" << result_verify_file;
    if (result_verify_file) {
        QProcess proc;
        QString program = "/usr/bin/deepin-deb-verify ";
        program = program + filepath_name;
        proc.start(program);
        qDebug() << "program:" << program;
        proc.waitForFinished();
        const QString output = proc.readAllStandardOutput();
        const QString output1 = proc.readAllStandardError();
        qDebug() << output1;
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

QString Utils::holdTextInRect(const QFont &font, QString srcText, const QSize &size)
{
    bool bContainsChinese = srcText.contains(QRegExp("[\\x4e00-\\x9fa5]+"));

    QString text;
    QString tempText;
    int totalHeight = size.height();
    int lineWidth = size.width() - font.pixelSize();

    int offset = bContainsChinese ? font.pixelSize() : 0;

    QFontMetrics fm(font);

    int calcHeight = 0;
    int lineHeight = fm.height();
    int lineSpace = 0;
    int lineCount = (totalHeight - lineSpace) / lineHeight;
    int prevLineCharIndex = 0;
    for (int charIndex = 0; charIndex < srcText.size() && lineCount >= 0; ++charIndex) {
        int fmWidth = fm.horizontalAdvance(tempText);
        if (fmWidth > lineWidth - offset || tempText.contains("\n")) {
            calcHeight += lineHeight + 3;
            if (calcHeight + lineHeight > totalHeight) {
                QString endString = srcText.mid(prevLineCharIndex);
                const QString &endText = fm.elidedText(endString, Qt::ElideRight, size.width());
                text += endText;

                lineCount = 0;
                break;
            }

            if (!bContainsChinese) {
                QChar currChar = tempText.at(tempText.length() - 1);
                QChar nextChar = srcText.at(srcText.indexOf(tempText) + tempText.length());
                if (currChar.isLetter() && nextChar.isLetter()) {
                    tempText += '-';
                }
                fmWidth = fm.horizontalAdvance(tempText);
                if (fmWidth > size.width()) {
                    --charIndex;
                    --prevLineCharIndex;
                    tempText = tempText.remove(tempText.length() - 2, 1);
                }
            }
            text += tempText;

            --lineCount;
            if (lineCount > 0) {
                if (!tempText.contains("\n"))
                    text += "\n";
            }
            tempText = srcText.at(charIndex);

            prevLineCharIndex = charIndex;
        } else {
            tempText += srcText.at(charIndex);
        }
    }

    if (lineCount > 0) {
        text += tempText;
    }

    return text;
}

DebApplicationHelper *DebApplicationHelper::instance()
{
    return qobject_cast<DebApplicationHelper *>(DGuiApplicationHelper::instance());
}

#define CAST_INT static_cast<int>

static QColor light_qpalette[QPalette::NColorRoles] {
    QColor("#414d68"),                              //WindowText
    QColor("#e5e5e5"),                              //Button
    QColor("#e6e6e6"),                              //Light
    QColor("#e5e5e5"),                              //Midlight
    QColor("#001A2E"),                              //Dark  -- changed origin is #e3e3e3
    QColor("#e4e4e4"),                              //Mid
    QColor("#414d68"),                              //Text
    Qt::black,                                      //BrightText
    QColor("#414d68"),                              //ButtonText
    Qt::white,                                      //Base
    QColor("#f8f8f8"),                              //Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),          //Shadow
    QColor("#0081ff"),                              //Highlight
    QColor(0, 45, 255, CAST_INT(0.5 * 255)),        //HighlightedText   //old Qt::white
    QColor("#0082fa"),                              //Link
    QColor("#ad4579"),                              //LinkVisited
    QColor(0, 0, 0, CAST_INT(0.03 * 255)),          //AlternateBase
    Qt::white,                                      //NoRole
    QColor(255, 255, 255, CAST_INT(0.8 * 255)),     //ToolTipBase
    QColor("#526A7F")                               //ToolTipText -- changed origin is Qt::black
};

static QColor dark_qpalette[QPalette::NColorRoles] {
    QColor("#c0c6d4"),                              //WindowText
    QColor("#444444"),                              //Button
    QColor("#484848"),                              //Light
    QColor("#474747"),                              //Midlight
    QColor("#C0C6D4"),                              //Dark      -- changed origin is #414141
    QColor("#434343"),                              //Mid
    QColor("#c0c6d4"),                              //Text
    Qt::white,                                      //BrightText
    QColor("#c0c6d4"),                              //ButtonText
    QColor(255, 255, 255, CAST_INT(0.05 * 255)),    //Base --- changed origin is #282828
    QColor("#252525"),                              //Window
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),          //Shadow
    QColor("#095EFF"),                              //Highlight         //old : #0081ff
    QColor("#0059D2"),                              //HighlightedText   //old: b8d3ff
    QColor("#0082fa"),                              //Link
    QColor("#ad4579"),                              //LinkVisited
    QColor(0, 0, 0, CAST_INT(0.05 * 255)),          //AlternateBase
    Qt::black,                                      //NoRole
    QColor(45, 45, 45, CAST_INT(0.8 * 255)),        //ToolTipBase
    QColor("#6D7C88")                               //ToolTipText -- changed origin is #c0c6d4
};

static QColor light_dpalette[DPalette::NColorTypes] {
    QColor(),                                   //NoType
    QColor(0, 0, 0, CAST_INT(255 * 0.03)),      //ItemBackground
    QColor("#414d68"),                          //TextTitle
    QColor("#609DC8"),                          //TextTips --- changed origin is #526A7F
    QColor("#FF5A5A"),                          //TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                          //TextLively  -- changed origin is #0082FA
    QColor("#417505"),                          //LightLively -- changed origin is #25b7ff
    QColor("#47790C"),                          //DarkLively -- changed origin is #0098ff
    QColor(0, 0, 0, CAST_INT(0.03 * 255))       //FrameBorder
};

static QColor dark_dpalette[DPalette::NColorTypes] {
    QColor(),                                       //NoType
    QColor(255, 255, 255, CAST_INT(255 * 0.05)),    //ItemBackground
    QColor("#c0c6d4"),                              //TextTitle
    QColor("#6D7C88"),                              //TextTips
    QColor("#9A2F2F"),                              //TextWarning -- changed origin is #FF5736
    QColor("#7C7C7C"),                              //TextLively -- changed origin is #0082FA
    QColor("#417505"),                              //LightLively -- changed origin is #0056c1
    QColor("#47790C"),                              //DarkLively  -- changed origin is #004c9c
    QColor(0, 0, 0, CAST_INT(0.08 * 255))           //FrameBorder
};

DPalette DebApplicationHelper::standardPalette(DGuiApplicationHelper::ColorType type) const
{
    DPalette pa;
    const QColor *qcolor_list, *dcolor_list;

    if (type == DarkType) {
        qcolor_list = dark_qpalette;
        dcolor_list = dark_dpalette;
    } else {
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
    // 记录此控件被设置过palette
    widget->setProperty("_d_set_palette", true);
    widget->setPalette(palette);
}

void DebApplicationHelper::resetPalette(QWidget *widget)
{
    widget->setProperty("_d_set_palette", QVariant());
    widget->setAttribute(Qt::WA_SetPalette, false);
}

DebApplicationHelper::DebApplicationHelper() {}

DebApplicationHelper::~DebApplicationHelper() {}

bool DebApplicationHelper::eventFilter(QObject *watched, QEvent *event)
{
    return DGuiApplicationHelper::eventFilter(watched, event);
}
