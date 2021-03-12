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

#ifndef PACKAGEDEPENDSSTATUS_H
#define PACKAGEDEPENDSSTATUS_H
#include <QObject>

class PackageDependsStatus
{
public:

    /**
     * @brief ok 组装依赖的状态，将传入的包的依赖状态设置为ok
     * @return 某个包的依赖状态为ok
     */
    static PackageDependsStatus ok();
    /**
     * @brief available 组装依赖的状态，将传入的包名的依赖状态设置为Available
     * @param package 当前依赖包的包名。
     * @return 某个包的依赖状态为Available
     */
    static PackageDependsStatus available(const QString &package);

    /**
     * @brief _break 组装break依赖状态。将传入的包的依赖状态设置为break
     * @param package 包的名字
     * @return 某个包的依赖状态为break
     */
    static PackageDependsStatus _break(const QString &package);

    PackageDependsStatus();
    PackageDependsStatus(const int status, const QString &package);

    /**
     * @brief operator = 重写=操作符
     * @param other 要赋值的依赖的状态
     * @return 赋值后的依赖的状态
     */
    PackageDependsStatus &operator=(const PackageDependsStatus &other);

    /**
     * @brief max   状态的比较
     * @param other
     * @return
     */
    PackageDependsStatus max(const PackageDependsStatus &other);
    PackageDependsStatus maxEq(const PackageDependsStatus &other);
    PackageDependsStatus min(const PackageDependsStatus &other);
    PackageDependsStatus minEq(const PackageDependsStatus &other);

    bool isBreak() const;
    bool isAuthCancel() const;
    bool isAvailable() const;

public:
    int status;
    QString package;
};
#endif // PACKAGEDEPENDSSTATUS_H
