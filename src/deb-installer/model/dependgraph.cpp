// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dependgraph.h"
#include "utils/ddlog.h"

#include <QSet>
#include <QDebug>

#include <algorithm>

struct DependGraphNode
{
    QString packageName;
    QString packagePath;
    QByteArray md5;
    QList<QApt::DependencyItem> depends;
    std::vector<DependGraphNode *> dependsInGraph;
};

DependGraph::~DependGraph()
{
    reset();
}

void DependGraph::addNode(const QString &packagePath,
                          const QByteArray &md5,
                          const QString &packageName,
                          const QList<QApt::DependencyItem> &depends)
{
    qCDebug(appLog) << "Adding node:" << packageName << "path:" << packagePath;
    auto node = new DependGraphNode;
    node->packagePath = packagePath;
    node->packageName = packageName;
    node->depends = depends;
    node->md5 = md5;

    // 建立依赖关系图
    std::vector<DependGraphNode *> dependsInGraph;

    // 1.搜索当前节点的潜在依赖关系
    for (auto &eachDepend : depends) {
        for (auto &eachOrDepend : eachDepend) {
            auto pkgName = eachOrDepend.packageName();
            auto iter = std::find_if(
                nodes.begin(), nodes.end(), [pkgName](const auto &eachNode) { return eachNode->packageName == pkgName; });
            if (iter != nodes.end()) {
                dependsInGraph.push_back(*iter);
            }
        }
    }

    // 2.为其他节点添加潜在依赖关系
    for (auto &eachNode : nodes) {
        bool finded = false;
        for (auto &eachDepends : eachNode->depends) {
            for (auto &eachOrDepend : eachDepends) {
                if (eachOrDepend.packageName() == packageName) {
                    eachNode->dependsInGraph.push_back(node);
                    break;
                }
            }
            if (finded) {
                break;
            }
        }
    }

    node->dependsInGraph = dependsInGraph;
    nodes.push_back(node);
}

bool isCircularDepend(DependGraphNode *currentNode, DependGraphNode *dependNode)
{
    auto iter = std::find_if(dependNode->dependsInGraph.begin(),
                             dependNode->dependsInGraph.end(),
                             [currentNode](DependGraphNode *node) { return currentNode->packageName == node->packageName; });
    return iter != dependNode->dependsInGraph.end();
}

void addDepend(QList<QString> &paths, QList<QByteArray> &md5s, QStringList &result, DependGraphNode *node)
{
    if (result.contains(node->packageName)) {
        return;
    }

    for (auto &eachDependNode : node->dependsInGraph) {
        if (eachDependNode->dependsInGraph.empty()) {
            if (result.contains(eachDependNode->packageName)) {
                continue;
            }
            result.push_back(eachDependNode->packageName);
            paths.push_back(eachDependNode->packagePath);
            md5s.push_back(eachDependNode->md5);
        } else {
            if (isCircularDepend(node, eachDependNode)) {  // 检查循环依赖
                qCWarning(appLog) << "Circular dependency detected between" << node->packageName << "and" << eachDependNode->packageName;
                continue;
            }
            addDepend(paths, md5s, result, eachDependNode);
            if (result.contains(eachDependNode->packageName)) {
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
    qCDebug(appLog) << "Calculating best install order for" << nodes.size() << "packages";
    QStringList result;
    QList<QString> paths;
    QList<QByteArray> md5s;
    for (size_t i = 0; i != nodes.size(); ++i) {
        if (result.contains(nodes[i]->packageName)) {
            continue;
        }
        // 添加前置依赖
        addDepend(paths, md5s, result, nodes[i]);
        // 添加本体
        if (result.contains(nodes[i]->packageName)) {
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
    qCDebug(appLog) << "Resetting dependency graph with" << nodes.size() << "nodes";
    for (auto &node : nodes) {
        delete node;
    }
    nodes.clear();
}

void DependGraph::remove(const QByteArray &md5)
{
    qCDebug(appLog) << "Removing node with md5:" << md5;
    for (size_t i = 0; i != nodes.size(); ++i) {
        if (nodes[i]->md5 == md5) {
            // 删除包需要更新依赖图(bug 179891)
            removeInGraph(nodes[i]);
            delete nodes[i];
            nodes.erase(nodes.begin() + static_cast<int>(i));
            qCDebug(appLog) << "Node removed, remaining nodes:" << nodes.size();
            break;
        }
    }
}

void DependGraph::removeInGraph(const DependGraphNode *dependnode)
{
    QSet<DependGraphNode*> visited;

    for (auto eachNode : nodes) {
        std::vector<DependGraphNode*> stack = { eachNode };

        while (!stack.empty()) {
            auto* node = stack.back();
            stack.pop_back();

            if (!node || visited.contains(node))
                continue;
            visited.insert(node);

            auto& deps = node->dependsInGraph;
            deps.erase(std::remove(deps.begin(), deps.end(), dependnode), deps.end());
            
            // Use std::copy to push all dependent nodes into the stack
            std::copy(deps.begin(), deps.end(), std::back_inserter(stack));
        }
    }
}
