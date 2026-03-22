// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "anchorid.h"

#include "functionnode.h"
#include "node.h"
#include "propertynode.h"
#include "typedefnode.h"

using namespace Qt::Literals::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \internal
    Computes the raw anchor base for \a node based on its type.

    QML properties receive a \c{-prop} or \c{-attached-prop} suffix,
    signals receive \c{-signal}, methods receive \c{-method} with an
    optional overload number, and so on. The returned string isn't
    sanitized — each caller applies its own cleanup or finalization
    (such as Generator::cleanRef() or HrefResolverConfig::cleanRefFn).

    Both HrefResolver::anchorForNode() and XmlGenerator::refForNode()
    delegate to this function for the shared node-type dispatch,
    ensuring consistent anchor naming across the IR extraction and
    legacy generation paths.
*/
QString computeAnchorId(const Node *node)
{
    QString ref;

    switch (node->nodeType()) {
    case NodeType::Enum:
    case NodeType::QmlEnum:
        ref = node->name() + "-enum"_L1;
        break;
    case NodeType::Typedef: {
        const auto *tdf = static_cast<const TypedefNode *>(node);
        if (tdf->associatedEnum())
            return computeAnchorId(tdf->associatedEnum());
    } Q_FALLTHROUGH();
    case NodeType::TypeAlias:
        ref = node->name() + "-typedef"_L1;
        break;
    case NodeType::Function: {
        const auto *fn = static_cast<const FunctionNode *>(node);
        switch (fn->metaness()) {
        case FunctionNode::QmlSignal:
            ref = fn->name() + "-signal"_L1;
            break;
        case FunctionNode::QmlSignalHandler:
            ref = fn->name() + "-signal-handler"_L1;
            break;
        case FunctionNode::QmlMethod:
            ref = fn->name() + "-method"_L1;
            if (fn->overloadNumber() != 0)
                ref += '-'_L1 + QString::number(fn->overloadNumber());
            break;
        default:
            if (const auto *p = fn->primaryAssociatedProperty(); p && fn->doc().isEmpty()) {
                return computeAnchorId(p);
            } else {
                ref = fn->name();
                if (fn->overloadNumber() != 0)
                    ref += '-'_L1 + QString::number(fn->overloadNumber());
            }
            break;
        }
    } break;
    case NodeType::SharedComment: {
        if (!node->isPropertyGroup())
            break;
    } Q_FALLTHROUGH();
    case NodeType::QmlProperty:
        if (node->isAttached())
            ref = node->name() + "-attached-prop"_L1;
        else
            ref = node->name() + "-prop"_L1;
        break;
    case NodeType::Property:
        ref = node->name() + "-prop"_L1;
        break;
    case NodeType::Variable:
        ref = node->name() + "-var"_L1;
        break;
    default:
        break;
    }

    return ref;
}

QT_END_NAMESPACE
