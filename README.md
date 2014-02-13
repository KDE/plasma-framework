# Plasma Framework
The plasma framework provides the foundations that can be used to build a primary user interface, from graphical to logical components.

## Introduction
The plasma framework is made by a library component and a runtime component:
* libplasma: this is a library for the business logic, it manages:
** rendering of SVG themes
** loading of files from a certain filesystem structure: packages
** data access: dataengines
** loading of the plugin structure of the workspace: containments and applets

* QML script engine: given the data provided by an Applet or a Containment, it maps to loading of an actual QML representation of it
* QML components: sets of QML components to provide a widget set, and to access to libplasma components, such as SVG themes and dataengines.
* Runtime environment: the plasma-shell executable. The runtime that will load the actual workspace, it's completely controlled by QML, by itself doesn't have any UI.


## Links

- Wiki: <http://community.kde.org/Plasma#Plasma_Workspaces_2>
- Mailing list: <https://mail.kde.org/mailman/listinfo/plasma-devel>
- IRC channel: #kde-devel on Freenode
- Git repository: <https://projects.kde.org/projects/playground/libs/plasma-framework/repository>
