/*
 * Copyright (C) 2017 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     WangZhengYang <wangzhengyang@uniontech.com>
 *
 * Maintainer: WangZhengYang <wangzhengyang@uniontech.com>
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

#pragma once

#include <QString>
#include <QStringList>
#include <qapt/debfile.h>

#include <vector>

struct DependGraphNode
{
    QString packageName;
    QString packagePath;
    QByteArray md5;
    QList<QApt::DependencyItem> depends;
    std::vector<DependGraphNode*> dependsInGraph;
};

class DependGraph
{
public:
    DependGraph() = default;
    ~DependGraph();

    void addNode(const QString &packagePath, const QByteArray &md5, const QString &packageName, const QList<QApt::DependencyItem> &depends);
    std::pair<QList<QString>, QList<QByteArray>> getBestInstallQueue() const;

    void reset();
    void remove(const QByteArray &md5);

private:
    std::vector<DependGraphNode*> nodes;
};
