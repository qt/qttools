// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
       QDesignerPropertySheetExtension *propertySheet;
       QExtensionManager manager = formEditor->extensionManager();

       propertySheet = qt_extension<QDesignerPropertySheetExtension*>(manager, widget);

       if(propertySheet) {...}
//! [0]


//! [1]
   Q_DECLARE_EXTENSION_INTERFACE(MyExtension, "com.mycompany.myproduct.myextension")
//! [1]


