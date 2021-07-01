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
/*  TRANSLATOR MULTIPLELINE
 first line
 second line
 */

/*  TRANSLATOR
    MultipleLineSecondTry
    first line
    second line
 */

/*  TRANSLATOR
    MultipleLineThirdTry first
    line
    second line
 */

/*  TRANSLATOR
    MultipleLineFourthTry first
    line
    second line
 */


/*  TRANSLATOR MultipleAppearance first appearance */

/*  TRANSLATOR MultipleAppearance second appearance */

/* TRANSLATOR GluedNumber1 blabla */
/* TRANSLATOR GluedNumber2 blabla */
// TRANSLATOR GluedNumber3 blabla
/*TRANSLATOR
GluedNumber4
        //     whatever
        we
        have
        here
*/
/*  TRANSLATOR GluedNumber5
        //     whatever2
        we2
        have2
        here2
*/

/* TRANSLATOR GluedOnSameLineOne blabla*/ /* TRANSLATOR GluedOnSameLineTwo blabla*/
// TRANSLATORMISSINGSPACE this will not be picked up by clang cpp parser
// oooTRANSLATOR SomethingBeforeTranslator this should not be picked up
// ooo TRANSLATOR SomethingBeforeTranslatorTwo this should not be picked up
/* ooo TRANSLATOR SomethingBeforeTranslatorThree this should not be picked up */
/* ~ TRANSLATOR SomethingBeforeTranslatorFour this
  should not be picked up */
// The following comment should be ignored because the comment part is empty
/* TRANSLATOR onlyContext */
