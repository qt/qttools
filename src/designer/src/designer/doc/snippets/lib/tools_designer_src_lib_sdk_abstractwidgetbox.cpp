// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
        QDesignerWidgetBoxInterface *widgetBox = 0:
        widgetBox = formEditor->widgetBox();

        widgetBox->load();
//! [0]


//! [1]
        QString originalFile = widgetBox->fileName();

        widgetBox->setFileName("myWidgetBox.xml");
        widgetBox->save();
//! [1]


//! [2]
        widgetBox->setFileName(originalFile);
        widgetBox->load();
//! [2]


//! [3]
        if (widgetBox->filename() != "myWidgetBox.xml") {
            widgetBox->setFileName("myWidgetBox.xml");
            widgetBox->load();
        }
//! [3]


