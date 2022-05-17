// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

























#include <QtCore/QtCore>
#include "test_isolatedComments.h"
#include "test_QT_TR_NOOP_context_with_comments.h"

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
