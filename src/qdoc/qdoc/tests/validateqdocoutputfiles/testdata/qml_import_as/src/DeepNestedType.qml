// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import TestModule.SubModule.Deep as X

/*!
    \qmltype DeepNestedType
    \inqmlmodule TestModule
    \brief Example demonstrating deeply nested qualified type names.

    This type tests QDoc's ability to handle qualified type names
    with multiple segments like \c{X.Level1.Level2.Leaf}.

    \qml
    import TestModule.SubModule.Deep as X

    DeepNestedType {
        // This should properly link X.Level1.Level2.Leaf as a complete type name
        nested: X.Level1.Level2.Leaf {
            value: 42
            deepProperty: "nested value"
        }

        // Test property bindings with multi-segment names
        Layout.alignment: Qt.AlignCenter
        anchors.margins.left: 10
    }
    \endqml

    The key test case is \c{X.Level1.Level2.Leaf} which has 4 segments:
    \list
        \li \c{X} - Module alias
        \li \c{Level1} - First level type/namespace
        \li \c{Level2} - Second level type/namespace
        \li \c{Leaf} - Final type name
    \endlist
*/

BaseType {
    id: deepNested

    /*!
        \qmlproperty Item DeepNestedType::nested
        A property that uses deeply nested type names for testing QDoc linking.
    */
    property alias nested: nestedItem

    /*!
        \qmlproperty string DeepNestedType::testProp
        Property to test multi-segment property access.
    */
    property string testProp: "test"

    // Example of deeply nested qualified type name
    X.Level1.Level2.Leaf {
        id: nestedItem
        value: 42

        // Test multi-segment property access
        Layout.preferredWidth: 100
        Layout.preferredHeight: 50
    }

    // Additional test with object binding
    someProperty: X.Level1.Level2.AnotherLeaf {
        deepProperty: "object binding test"
    }
}
