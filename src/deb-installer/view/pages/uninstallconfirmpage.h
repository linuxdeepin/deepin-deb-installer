/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
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

#ifndef UNINSTALLCONFIRMPAGE_H
#define UNINSTALLCONFIRMPAGE_H

#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/installprocessinfoview.h"

#include <DLabel>
#include <DPushButton>
#include <QWidget>

class UninstallConfirmPage : public QWidget
{
    Q_OBJECT

public:
    explicit UninstallConfirmPage(QWidget *parent = nullptr);

    /**
     * @brief setPackage 设置要卸载的包的名称
     * @param packageName 包的名称
     */
    void setPackage(const QString &packageName);

    /**
     * @brief setRequiredList 设置依赖当前包的依赖列表
     * @param requiredList 依赖列表
     */
    void setRequiredList(const QStringList &requiredList);

signals:

    /**
     * @brief signalUninstallAccepted 卸载确认
     */
    void signalUninstallAccepted() const;

    /**
     * @brief signalUninstallCanceled 卸载取消
     */
    void signalUninstallCanceled() const;


private slots:

    /**
     * @brief slotShowDetail 显示卸载信息
     */
    void slotShowDetail();

    /**
     * @brief slotHideDetail 隐藏卸载信息
     */
    void slotHideDetail();

private:
    DLabel                  *m_icon                = nullptr;
    DLabel                  *m_tips                = nullptr;
    QWidget                 *m_infoWrapperWidget   = nullptr;
    InfoControlButton       *m_infoControl         = nullptr;
    InstallProcessInfoView  *m_dependsInfomation   = nullptr;
    DPushButton             *m_cancelBtn           = nullptr;
    DPushButton             *m_confirmBtn          = nullptr;

    QStringList             m_requiredList         = {};
    QString                 m_description          = "";
};
#endif  // UNINSTALLCONFIRMPAGE_H
