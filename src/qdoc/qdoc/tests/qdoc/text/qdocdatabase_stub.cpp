// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/*
 * Link-seam test double for QDocDatabase.
 *
 * atom.cpp references QDocDatabase::qdocDB() because LinkAtom's
 * constructors resolve square-bracket parameters against the forest.
 * Linking the real qdocdatabase.cpp would pull Tree, Node, and the rest
 * of the driver into tst_QDoc, which defeats testing Text and Atom in
 * isolation. This translation unit satisfies the closure with an empty
 * forest instead: findTree() is inline and returns nullptr for every
 * name, so LinkAtom falls through to the genus keywords, which is all
 * the tests need.
 *
 * If qdocdatabase.cpp is ever added to tst_QDoc's sources, the linker
 * reports duplicate symbols; delete this file at that point.
 */

#include <qdoc/qdocdatabase.h>

QT_BEGIN_NAMESPACE

QDocDatabase *QDocDatabase::s_qdocDB = nullptr;

QDocForest::~QDocForest() = default;

QDocDatabase::QDocDatabase() : m_forest(this) { }

QDocDatabase *QDocDatabase::qdocDB()
{
    if (s_qdocDB == nullptr)
        s_qdocDB = new QDocDatabase;
    return s_qdocDB;
}

QT_END_NAMESPACE

