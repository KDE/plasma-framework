#!/bin/sh

# Brutal, amateurish bash-perl script to help with porting
# different aspects to Qt5, QtQuick 2.0 and Plasma 2. This
# is not a complete "solution", but will do some of the
# monkey work for you.
#
# Run this script with care, it will blindly iron over all
# the subdirectories of $PWD without warning. Be advised to
# use this in a clean git branch, carefully review all
# changes made.


#echo "For your own safety, this script does nothing unless you edit it."
#exit

# Change a whole bunch of classes that have more or less been renamed
# in QtQuick2 - this is useful if you have C++ classes that you're
# exporting to the QML runtime

for FS in `find $PWD  -name '*.h' -o -name '*.cpp'`; do

    perl -p -i -e 's/QGraphicsWidget/QQuickItem/g' $FS

    perl -p -i -e 's/QGraphicsObject/QQuickItem/g' $FS
    perl -p -i -e 's/QDeclarativeItem/QQuickItem/g' $FS
    perl -p -i -e 's/QDeclarativeView/QQuickView/g' $FS
    perl -p -i -e 's/QDeclarativeImageProvider/QQuickImageProvider/g' $FS
    perl -p -i -e 's/QDeclarativeComponent/QQmlComponent/g' $FS
    perl -p -i -e 's/QDeclarativeContext/QQmlContext/g' $FS
    perl -p -i -e 's/QDeclarativeEngine/QQmlEngine/g' $FS
    perl -p -i -e 's/QDeclarativeError/QQmlError/g' $FS
    perl -p -i -e 's/QDeclarativeExpression/QQmlExpression/g' $FS
    perl -p -i -e 's/QDeclarativeExtensionPlugin/QQmlExtensionPlugin/g' $FS
    perl -p -i -e 's/QDeclarativeInfo/QQmlInfo/g' $FS
    perl -p -i -e 's/QDeclarativeListReference/QQmlListReference/g' $FS
    perl -p -i -e 's/QDeclarativeNetworkAccessManagerFactory/QQmlNetworkAccessManagerFactory/g' $FS
    perl -p -i -e 's/QDeclarativeParserStatus/QQmlParserStatus/g' $FS
    perl -p -i -e 's/QDeclarativeProperty/QQmlProperty/g' $FS
    perl -p -i -e 's/QDeclarativePropertyMap/QQmlPropertyMap/g' $FS
    perl -p -i -e 's/QDeclarativePropertyValueSource/QQmlPropertyValueSource/g' $FS
    perl -p -i -e 's/QDeclarativeScriptString/QQmlScriptString/g' $FS

    perl -p -i -e 's/QGraphicsSceneMouseEvent/QMouseEvent/g' $FS
    perl -p -i -e 's/QGraphicsSceneContextMenuEvent/QContextMenuEvent/g' $FS
    perl -p -i -e 's/QGraphicsSceneWheel/QWheelEvent/g' $FS
    perl -p -i -e 's/QGraphicsSceneHoverEvent/QHoverEvent/g' $FS
    perl -p -i -e 's/QWheelEventEvent/QWheelEvent/g' $FS

    # Fix up includes
    perl -p -i -e 's/\#include \<QtGui\/QQuickItem\>/\#include \<QtQuick\/QQuickItem\>/g' $FS
    perl -p -i -e 's/\#include \<QtGui\/QDialog\>/\#include \<QtWidgets\/QDialog\>/g' $FS
    perl -p -i -e 's/\#include \<QtGui\/QMenu\>/\#include \<QtWidgets\/QMenu\>/g' $FS
    perl -p -i -e 's/\#include \<QtGui\/QWidget\>/\#include \<QtWidgets\/QWidget\>/g' $FS
    perl -p -i -e 's/\#include \<QtDeclarative\/QQuickItem\>/\#include \<QtQuick\/QQuickItem\>/g' $FS
    perl -p -i -e 's/\#include \<QtDeclarative\/QQmlContext\>/\#include \<QtQml\/QQmlContext\>/g' $FS
    perl -p -i -e 's/\#include \<QtDeclarative\/QQmlEngine\>/\#include \<QtQml\/QQmlEngine\>/g' $FS
    perl -p -i -e 's/\#include \<QtDeclarative\/QQmlComponent\>/\#include \<QtQml\/QQmlComponent\>/g' $FS
    perl -p -i -e 's/\#include \<QtDeclarative\/QQmlError\>/\#include \<QtQml\/QQmlError\>/g' $FS


done



