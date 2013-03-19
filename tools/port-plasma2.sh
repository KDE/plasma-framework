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

# The following is useful for porting QML plasmoids

# Blindly change all 0.1 imports to 2.0
# Be a bit more specific for draganddrop (1.0 -> 2.0 in this case)

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/org\.kde\.plasma\.core 0\.1/org.kde.plasma.core 2.0/g' $FS
    perl -p -i -e 's/org\.kde\.plasma\.components 0\.1/org.kde.plasma.components 2.0/g' $FS
    perl -p -i -e 's/org\.kde\.plasma\.extras 0\.1/org.kde.plasma.extras 2.0/g' $FS

    perl -p -i -e 's/org\.kde\.qtextracomponents 0\.1/org.kde.qtextracomponents 2.0/g' $FS
    perl -p -i -e 's/org\.kde\.runnermodel 0\.1/org.kde.runnermodel 2.0/g' $FS
    perl -p -i -e 's/org\.kde\.locale 0\.1/org.kde.locale 2.0/g' $FS

    perl -p -i -e 's/org\.kde\.draganddrop 1\.0/org.kde.draganddrop 2.0/g' $FS
done


# Qt 4.7, QtQuick 1.0 and QtQuick 1.1 imports become QtQuick 2.0

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/Qt 4.7/QtQuick 2.0/g' $FS
    perl -p -i -e 's/QtQuick 1.0/QtQuick 2.0/g' $FS
    perl -p -i -e 's/QtQuick 1.1/QtQuick 2.0/g' $FS
done

exit


# Change plugin definitions, this is useful if you want to
# port a bunch of plugins from 1.0 and 0.1 to 2.0

# for FS in `find $PWD  -name '*.cpp'`; do
#     perl -p -i -e 's/\(uri\, 0\, 1\, \"/\(uri\, 2\, 0\, \"/g' $FS
#     perl -p -i -e 's/\(uri\, 1\, 0\, \"/\(uri\, 2\, 0\, \"/g' $FS
# done
# for FS in `find $PWD  -name 'qmldir'`; do
#     perl -p -i -e 's/ 0\.1 / 2.0 /g' $FS
# done

#exit

# Change CMake install locations to QtQuick import paths
for FS in `find $PWD -type f -name 'CMakeLists.txt'`; do
    perl -p -i -e 's/IMPORTS_INSTALL_DIR/QML_INSTALL_DIR/g' $FS
done

#exit;


# Change a whole bunch of classes that have more or less been renamed
# in QtQuick2 - this is useful if you have C++ classes that you're
# exporting to the QML runtime

for FS in `find $PWD  -name '*.h' -o -name '*.cpp'`; do

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
    perl -p -i -e 's/QGraphicsSceneWheel/QWheelEvent/g' $FS
    perl -p -i -e 's/QGraphicsSceneHoverEvent/QHoverEvent/g' $FS

done



