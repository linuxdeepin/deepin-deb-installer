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

#include "dependgraph.h"

DependGraph::~DependGraph()
{
    reset();
}

void DependGraph::addNode(const QString &packagePath, const QByteArray &md5, const QString &packageName, const QList<QApt::DependencyItem> &depends)
{
    auto node = new DependGraphNode;
    node->packagePath = packagePath;
    node->packageName = packageName;
    node->depends = depends;
    node->md5 = md5;

    //建立依赖关系图
    std::vector<DependGraphNode*> dependsInGraph;

    //1.搜索当前节点的潜在依赖关系
    for(auto &eachDepend : depends) {
        for(auto &eachOrDepend : eachDepend) {
            for(auto &eachNode : nodes) {
                if(eachNode->packageName == eachOrDepend.packageName()) {
                    dependsInGraph.push_back(eachNode);
                    break;
                }
            }
        }
    }

    //2.为其他节点添加潜在依赖关系
    for(auto &eachNode : nodes) {
        bool finded = false;
        for(auto &eachDepends : eachNode->depends) {
            for(auto &eachOrDepend : eachDepends) {
                if(eachOrDepend.packageName() == packageName) {
                    eachNode->dependsInGraph.push_back(node);
                    break;
                }
            }
            if(finded) {
                break;
            }
        }
    }

    node->dependsInGraph = dependsInGraph;
    nodes.push_back(node);
}

void addDepend(QList<QString> &paths, QList<QByteArray> &md5s, QStringList &result, DependGraphNode *node)
{
    if(result.contains(node->packageName)) {
        return;
    }

    for(auto &eachDependNode : node->dependsInGraph) {
        if(eachDependNode->dependsInGraph.empty()) {
            if(result.contains(eachDependNode->packageName)) {
                continue;
            }
            result.push_back(eachDependNode->packageName);
            paths.push_back(eachDependNode->packagePath);
            md5s.push_back(eachDependNode->md5);
        } else {
            addDepend(paths, md5s, result, eachDependNode);
            if(result.contains(eachDependNode->packageName)) {
                continue;
            }
            result.push_back(eachDependNode->packageName);
            paths.push_back(eachDependNode->packagePath);
            md5s.push_back(eachDependNode->md5);
        }
    }
}

std::pair<QList<QString>, QList<QByteArray>> DependGraph::getBestInstallQueue() const
{
    QStringList result;
    QList<QString> paths;
    QList<QByteArray> md5s;
    for(size_t i = 0;i != nodes.size();++i) {
        if(result.contains(nodes[i]->packageName)) {
            continue;
        }
        //添加前置依赖
        addDepend(paths, md5s, result, nodes[i]);
        //添加本体
        if(result.contains(nodes[i]->packageName)) {
            continue;
        }
        result.push_back(nodes[i]->packageName);
        paths.push_back(nodes[i]->packagePath);
        md5s.push_back(nodes[i]->md5);
    }
    return std::make_pair(paths, md5s);
}

void DependGraph::reset()
{
    for(auto &node : nodes) {
        delete node;
    }
    nodes.clear();
}

void DependGraph::remove(const QByteArray &md5)
{
    for(size_t i = 0;i != nodes.size();++i) {
        if(nodes[i]->md5 == md5) {
            delete nodes[i];
            nodes.erase(nodes.begin() + static_cast<int>(i));
            break;
        }
    }
}
