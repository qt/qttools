// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [6]
QHelpEngineCore helpEngine("mycollection.qhc");
...

// get all file references for the identifier
QList<QHelpLink> links =
    helpEngine.documentsForIdentifier(QLatin1String("MyDialog::ChangeButton"));

// If help is available for this keyword, get the help data
// of the first file reference.
if (links.count()) {
    QByteArray helpData = helpEngine->fileData(links.constBegin()->url);
    // show the documentation to the user
    if (!helpData.isEmpty())
        displayHelp(helpData);
}
//! [6]


