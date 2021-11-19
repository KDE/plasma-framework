/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// ButtonsPage

PlasmaComponents.Page {
    id: editorPage

    property string shader
    property alias shaderText: editor.text
    property string pageName: "Editor"
    property string icon: "accessories-text-editor"

    anchors {
        fill: parent
        margins: _s
    }

    Image {
        id: imageItem
        anchors.fill: parent
        //source: "../images/elarun-small.png"
    }

    ShaderEffectSource {
        id: effectSource
        sourceItem: imageItem
        //hideSource: hideSourceCheckbox.checked
        hideSource: true
    }

    ShaderEffect {
        id: mainShader
        anchors.fill: editorPage
        property ShaderEffectSource source: effectSource
        property real f: 0
        property real f2: 0
        property int intensity: 1
        smooth: true
    }
    PlasmaComponents.ToolButton {
        iconSource: "dialog-close"
        width: _h
        height: width
        visible: !(mainShader.fragmentShader == "" && mainShader.vertexShader == "")
        anchors { top: parent.top; right: parent.right; }
        onClicked: {
            mainShader.fragmentShader = "";
            mainShader.vertexShader = "";
            editorPage.shader = ""
            vertexPage.shader = ""
        }
    }


    PlasmaExtras.Heading {
        id: heading
        level: 1
        anchors {
            top: parent.top;
            left: parent.left
            right: parent.right
        }
        text: pageName
    }
    PlasmaComponents.ButtonColumn {
        anchors {
            right: parent.right
            top: heading.top
        }
        PlasmaComponents.RadioButton {
            id: fragmentRadio
            text: "Fragment / Pixel Shader"
        }
        PlasmaComponents.RadioButton {
            text: "Vertex Shader"
        }
    }

//     PlasmaComponents.TextArea {
//         id: editor
//         anchors {
//             top: heading.bottom;
//             topMargin: _s
//             left: parent.left
//             right: parent.right
//             bottom: applyButton.top
//             bottomMargin: _s
//
//         }
// //         text: { "void main(void) {\
// //         gl_FragColor = vec4(1.0, 0.0, 0.0, 0.3);\
// //     }"
// //         }
//         text:"
//         void main(void) {
//             gl_FragColor = vec4(0.2, 0.8, 0.6, 0.3);
//         }
//         "
//
// //         width: parent.width
// //         parent.height-height: _h*2
//     }

    PlasmaComponents.Button {
        id: applyButton
        text: "Upload Shader"
        onClicked: {
            shader = editor.text
            if (fragmentRadio.checked) {
                print("Uploading new fragment shader: \n" + shader);
                mainShader.fragmentShader = shader
            } else {
                print("Uploading new vertex shader: \n" + shader);
                mainShader.vertexShader = shader;
            }
        }

        anchors {
            right: parent.right
            bottom: parent.bottom

        }



    }
//     PlasmaComponents.CheckBox {
//         id: hideSourceCheckbox
//         text: "Hide Source Item"
//         anchors { bottom: parent.bottom; left: parent.left; margins: _s; }
//         onCheckedChanged: effectSource.hideSource = checked
//     }

}
