//
//  PropGroup.qml
//
//  Created by Sam Gateau on 3/2/2019
//  Copyright 2019 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or https://www.apache.org/licenses/LICENSE-2.0.html
//

import QtQuick 2.7

Item {
    Global { id: global }
    id: root

    property var label: "group"

    property alias isUnfold: headerFolderIcon.icon
    property alias propItemsPanel: propItemsContainer
    default property alias extHeader: headerContainer.data

    // Header Item
    Item {
        id: header
        height: global.slimHeight
        anchors.left: parent.left           
        anchors.right: parent.right

        // First in the header, the folder button / indicator
        Item {
            id: headerFolder
            anchors.left: header.left
            width: headerFolderIcon.width * 2
            anchors.verticalCenter: header.verticalCenter
            height: parent.height
            
            PropCanvasIcon {
                id: headerFolderIcon
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                fillColor: global.colorOrangeAccent 
                filled: root.propItemsPanel.height > 4

                MouseArea{
                    id: mousearea
                    anchors.fill: parent
                    onClicked: {
                        root.isUnfold = !root.isUnfold
                    }
                }
            }
        }

        // Next the header container
        // by default containing a Label showing the root.label
        Item {
            id: headerContainer
            anchors.left: headerFolder.right
            anchors.right: header.right
            anchors.verticalCenter: header.verticalCenter
            height: parent.height

            PropLabel {
                id: labelControl
                anchors.left: headerContainer.left
                anchors.right: headerContainer.right
                anchors.verticalCenter: headerContainer.verticalCenter
                text: root.label
                horizontalAlignment: Text.AlignHCenter
            }

        }
    }

    // The Panel container 
    Rectangle {
        visible: root.isUnfold

        color: "transparent"
        border.color: global.colorBorderLight
        border.width: global.valueBorderWidth
        radius: global.valueBorderRadius

        anchors.margins: 0
        anchors.left: parent.left           
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: root.bottom  

        Column {
            id: propItemsContainer
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 0
            anchors.rightMargin: 0
               
            clip: true

            // Where the propItems are added
        }
    }

    height: header.height + isUnfold * propItemsContainer.height
//    anchors.leftMargin: global.horizontalMargin
//    anchors.rightMargin: global.horizontalMargin
    anchors.margins: 0
    anchors.left: parent.left           
    anchors.right: parent.right

    
    // Prop Group is designed to author an array of ProItems, they are defined with an array of the tuplets describing each individual item:
    // [ ..., PropItemInfo, ...]
    // PropItemInfo {
    //    type: "PropXXXX", object: JSobject, property: "propName"      
    // }
    //
    function updatePropItems(propItemsModel) {
        for (var i = 0; i < propItemsModel.length; i++) {
            var proItem = propItemsModel[i];
            // valid object
            if (proItem['object'] !== undefined && proItem['object'] !== null ) {
                // valid property
                if (proItem['property'] !== undefined && proItem.object[proItem.property] !== undefined) {
                    // check type
                    if (proItem['type'] === undefined) {
                        proItem['type'] = typeof(proItem.object[proItem.property])
                    }
                    switch(proItem.type) {
                        case 'boolean':
                        case 'PropBool': {
                            var component = Qt.createComponent("PropBool.qml");
                            component.createObject(propItemsContainer, {
                                "label": proItem.property,
                                "object": proItem.object,
                                "property": proItem.property
                            })
                        } break;
                        case 'number':
                        case 'PropScalar': {
                            var component = Qt.createComponent("PropScalar.qml");
                            component.createObject(propItemsContainer, {
                                "label": proItem.property,
                                "object": proItem.object,
                                "property": proItem.property,
                                "min": (proItem["min"] !== undefined ? proItem.min : 0.0),                   
                                "max": (proItem["max"] !== undefined ? proItem.max : 1.0),                                       
                                "integer": (proItem["integral"] !== undefined ? proItem.integral : false),
                            })
                        } break;
                        case 'PropEnum': {
                            var component = Qt.createComponent("PropEnum.qml");
                            component.createObject(propItemsContainer, {
                                "label": proItem.property,
                                "object": proItem.object,
                                "property": proItem.property,
                                "enums": (proItem["enums"] !== undefined ? proItem.enums : ["Undefined Enums !!!"]), 
                            })
                        } break;
                        case 'object': {
                            var component = Qt.createComponent("PropItem.qml");
                            component.createObject(propItemsContainer, {
                                "label": proItem.property,
                                "object": proItem.object,
                                "property": proItem.property,
                             })
                        } break;
                        case 'printLabel': {
                            var component = Qt.createComponent("PropItem.qml");
                            component.createObject(propItemsContainer, {
                                "label": proItem.property
                             })
                        } break;
                    }
                } else {
                    console.log('Invalid property: ' + JSON.stringify(proItem));
                }
            } else if (proItem['type'] === 'printLabel') {
                var component = Qt.createComponent("PropItem.qml");
                component.createObject(propItemsContainer, {
                    "label": proItem.label
                })     
            } else {
                console.log('Invalid object: ' + JSON.stringify(proItem));
            }
        }
    }
    Component.onCompleted: {
    }
}
