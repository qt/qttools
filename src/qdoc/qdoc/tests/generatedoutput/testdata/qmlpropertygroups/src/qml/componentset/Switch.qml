// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 1.0

/*!
    \qmltype ToggleSwitch
    \inqmlmodule UIComponents
    \brief A component that can be turned on or off.

    A toggle switch has two states: an \c on and an \c off state. The \c off
    state is when the \l on property is set to \c false.

    The ToggleSwitch component is part of the \l {UI Components} module.

    This documentation is part of the \l{componentset}{UIComponents} example.

*/
Item {
    id: toggleswitch
    width: background.width; height: background.height

    /*!
       Indicates the state of the switch. If \c false, then the switch is in
       the \c off state.

       \omit
            The \qmlproperty <type> <propertyname> is not necessary as QDoc
            will associate this property to the ToggleSwitch

           QDoc will not publish the documentation within omit and endomit.
       \endomit
    */
    property bool on: false


    /*!
        A method to toggle the switch. If the switch is \c on, the toggling it
        will turn it \c off. Toggling a switch in the \c off position will
        turn it \c on.
    */
    function toggle() {
        if (toggleswitch.state == "on")
            toggleswitch.state = "off";
        else
            toggleswitch.state = "on";
    }


    /*!
        \internal

        An internal function to synchronize the switch's internals. This
        function is not for public access. The \internal command will
        prevent QDoc from publishing this comment in the public API.
    */
    function releaseSwitch() {
        if (knob.x == 1) {
            if (toggleswitch.state == "off") return;
        }
        if (knob.x == 78) {
            if (toggleswitch.state == "on") return;
        }
        toggle();
    }

    Rectangle {
        id: background
        width: 130; height: 48
        radius: 48
        color: "lightsteelblue"
        MouseArea { anchors.fill: parent; onClicked: toggle() }
    }

    Rectangle {
        id: knob
        width: 48; height: 48
        radius: width
        color: "lightblue"

        MouseArea {
            anchors.fill: parent
            drag.target: knob; drag.axis: Drag.XAxis; drag.minimumX: 1; drag.maximumX: 78
            onClicked: toggle()
            onReleased: releaseSwitch()
        }
    }

    states: [
        State {
            name: "on"
            PropertyChanges { target: knob; x: 78 }
            PropertyChanges { target: toggleswitch; on: true }
        },
        State {
            name: "off"
            PropertyChanges { target: knob; x: 1 }
            PropertyChanges { target: toggleswitch; on: false }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "x"; easing.type: Easing.InOutQuad; duration: 200 }
    }
}
