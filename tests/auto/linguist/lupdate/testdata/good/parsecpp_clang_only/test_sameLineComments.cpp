// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0


























#include <QtCore/QtCore>

class CommentOnSameLineMetaDataComment
{
    Q_OBJECT
    //= other metaData
    void hello(int something /*= metaData1 */, QString str = tr("meta data comment before translation. , and = in between"));
    void hellobis(/*= metaData2 */QString str = tr("meta data comment before translation. = in between"));
    void hellobiss()
    {
        hellobis(/*= metaData3*/ tr("meta data comment before translation"));
    }
};
class CommentOnSameLineExtraComment
{
    Q_OBJECT
    //: other extra comment
    void hello(int something /*: extra1 */, QString str = tr("extra comment before translation. , and = in between"));
    void hellobis(/*: extra2 */QString str = tr("extra comment before translation. = in between"));
    void hellobiss()
    {
        hellobis(/*: extra3*/ tr("extra comment before translation"));
    }
    //: other extra comment
    QString toto/*: extra4*/; QString titi = tr("extra comment before translation. ; in between");
};
