/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
