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
    perl -p -i -e 's/org\.kde\.draganddrop 1\.0/org.kde.draganddrop 2.0/g' $FS

    perl -p -i -e 's/theme\.mSize\./theme.mSize(theme.defaultFont)./g' $FS
    perl -p -i -e 's/theme\.defaultFont\.mSize\./theme.mSize(theme.defaultFont)./g' $FS
    perl -p -i -e 's/\(theme\.defaultFont\)\(theme\.defaultFont\)/(theme.defaultFont)/g' $FS

    perl -p -i -e 's/theme\.iconSizes/units.iconSizes/g' $FS
    perl -p -i -e 's/theme\.iconSizeSmall/units.iconSizes.small/g' $FS
    perl -p -i -e 's/theme\.smallIconSize/units.iconSizes.small/g' $FS
    perl -p -i -e 's/theme\.smallMediumIconSize/units.iconSizes.smallMedium/g' $FS
    perl -p -i -e 's/theme\.mediumIconSize/units.iconSizes.medium/g' $FS
    perl -p -i -e 's/theme\.largeIconSize/units.iconSizes.large/g' $FS
    perl -p -i -e 's/theme\.hugeIconSize/units.iconSizes.huge/g' $FS
    perl -p -i -e 's/theme\.enormousIconSize/units.iconSizes.enormous/g' $FS

    perl -p -i -e 's/units\.iconSizes\.dialog/units.iconSizes.medium/g' $FS
    perl -p -i -e 's/units\.toolbar\.dialog/units.iconSizes.smallMedium/g' $FS

    perl -p -i -e 's/theme\.smallSpacing/units.smallSpacing/g' $FS
    perl -p -i -e 's/theme\.largeSpacing/units.largeSpacing/g' $FS
    perl -p -i -e 's/theme\.defaultIconSize/units.iconSizes.default/g' $FS

    #    perl -p -i -e 's/Plasma\.Types\./PlasmaCore.Types./g' $FS
done

exit

# Qt 4.7, QtQuick 1.0 and QtQuick 1.1 imports become QtQuick 2.0

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/Qt 4.7/QtQuick 2.0/g' $FS
    perl -p -i -e 's/QtQuick 1.0/QtQuick 2.0/g' $FS
    perl -p -i -e 's/QtQuick 1.1/QtQuick 2.0/g' $FS
done

#exit


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

for FS in `find $PWD -type f -name '*.h'`; do
    perl -p -i -e 's/slots/Q_SLOTS/g' $FS
done

#exit;


# Some types have moved to the more QML-friendly Plasma::Types:: namespace

for FS in `find $PWD  -name '*.h' -o -name '*.cpp'`; do

    perl -p -i -e 's/Plasma\:\:PopupApplet/Plasma::Applet/g' $FS

    
    # Constraint
    perl -p -i -e 's/Plasma\:\:Constraint/Plasma::Types::Constraint/g' $FS
    perl -p -i -e 's/Plasma\:\:NoConstraint/Plasma::Types::NoConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:FormFactorConstraint/Plasma::Types::FormFactorConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:LocationConstraint/Plasma::Types::LocationConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:ScreenConstraint/Plasma::Types::ScreenConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:ImmutableConstraint/Plasma::Types::ImmutableConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:StartupCompletedConstraint/Plasma::Types::StartupCompletedConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:ContextConstraint/Plasma::Types::ContextConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:UiReadyConstraint/Plasma::Types::UiReadyConstraint/g' $FS
    perl -p -i -e 's/Plasma\:\:AllConstraints/Plasma::Types::AllConstraints/g' $FS
    
    # FormFactor
    perl -p -i -e 's/Plasma\:\:Planar/Plasma::Types::Planar/g' $FS
    perl -p -i -e 's/Plasma\:\:MediaCenter/Plasma::Types::MediaCenter/g' $FS
    perl -p -i -e 's/Plasma\:\:Horizontal/Plasma::Types::Horizontal/g' $FS
    perl -p -i -e 's/Plasma\:\:Vertical/Plasma::Types::Vertical/g' $FS
    perl -p -i -e 's/Plasma\:\:Application/Plasma::Types::Application/g' $FS

    # FormFactor
    perl -p -i -e 's/Plasma\:\:ImmutabilityType/Plasma::Types::ImmutabilityType/g' $FS
    perl -p -i -e 's/Plasma\:\:Mutable/Plasma::Types::Mutable/g' $FS
    perl -p -i -e 's/Plasma\:\:UserImmutable/Plasma::Types::UserImmutable/g' $FS
    perl -p -i -e 's/Plasma\:\:SystemImmutable/Plasma::Types::SystemImmutable/g' $FS

    # Location
    perl -p -i -e 's/Plasma\:\:Floating/Plasma::Types::Floating/g' $FS
    perl -p -i -e 's/Plasma\:\:Desktop/Plasma::Types::Desktop/g' $FS
    perl -p -i -e 's/Plasma\:\:FullScreen/Plasma::Types::FullScreen/g' $FS
    perl -p -i -e 's/Plasma\:\:TopEdge/Plasma::Types::TopEdge/g' $FS
    perl -p -i -e 's/Plasma\:\:BottomEdge/Plasma::Types::BottomEdge/g' $FS
    perl -p -i -e 's/Plasma\:\:LeftEdge/Plasma::Types::LeftEdge/g' $FS
    perl -p -i -e 's/Plasma\:\:RightEdge/Plasma::Types::RightEdge/g' $FS
    
    # ItemStatus
    perl -p -i -e 's/Plasma\:\:ItemStatus/Plasma::Types::ItemStatus/g' $FS
    perl -p -i -e 's/Plasma\:\:UnknownStatus/Plasma::Types::UnknownStatus/g' $FS
    perl -p -i -e 's/Plasma\:\:PassiveStatus/Plasma::Types::PassiveStatus/g' $FS
    perl -p -i -e 's/Plasma\:\:ActiveStatus/Plasma::Types::ActiveStatus/g' $FS
    perl -p -i -e 's/Plasma\:\:NeedsAttentionStatus/Plasma::Types::NeedsAttentionStatus/g' $FS
    perl -p -i -e 's/Plasma\:\:AcceptingInputStatus/Plasma::Types::AcceptingInputStatus/g' $FS
done

# make compactrepresentation come from Plasmoid.*
# size hints come from Layout
for FS in `find $PWD -type f -name '*.qml'`; do
    perl -p -i -e 's/property Component compactRepresentation/Plasmoid.compactRepresentation/g' $FS
    perl -p -i -e 's/property int minimumWidth/Layout.minimumWidth/g' $FS
    perl -p -i -e 's/property int minimumHeight/Layout.minimumHeight/g' $FS
    perl -p -i -e 's/property int maximumWidth/Layout.maximumWidth/g' $FS
    perl -p -i -e 's/property int maximumHeight/Layout.maximumHeight/g' $FS
    perl -p -i -e 's/property bool fillWidth/Layout.fillWidth/g' $FS
    perl -p -i -e 's/property bool fillHeight/Layout.fillHeight/g' $FS
done

for FS in `find $PWD -type f -name '*main.qml'`; do
    perl -p -i -e 's/QtQuick 2.0/QtQuick 2.0\nimport org.kde.plasma.plasmoid 2.0/g' $FS
done

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/org.kde.qtextracomponents/org.kde.kquickcontrolsaddons/g' $FS
done

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/QtExtras/KQuickControlsAddons/g' $FS
done

for FS in `find $PWD  -name '*.qml'`; do
    perl -p -i -e 's/QtExtra/KQuickControlsAddons/g' $FS
done

