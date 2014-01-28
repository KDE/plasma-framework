# -*- coding: utf-8 -*-
#
# Copyright 2008 Simon Edwards <simon@simonzone.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details
#
# You should have received a copy of the GNU Library General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyKDE4.plasma import Plasma
import plasma_importer
import os.path

class PythonDataEngineScript(Plasma.DataEngineScript):
    importer = None

    def __init__(self, parent):
        Plasma.DataEngineScript.__init__(self,parent)
        if PythonDataEngineScript.importer is None:
            PythonDataEngineScript.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        self.moduleName = str(self.dataEngine().pluginName())
        self.pluginName = 'dataengine_' + self.moduleName.replace('-','_')

        PythonDataEngineScript.importer.register_top_level(self.pluginName, str(self.dataEngine().package().path()))

        # import the code at the file name reported by mainScript()
        relpath = os.path.relpath(str(self.mainScript()),str(self.dataEngine().package().path()))
        if relpath.startswith("contents/code/"):
            relpath = relpath[14:]
        if relpath.endswith(".py"):
            relpath = relpath[:-3]
        relpath = relpath.replace("/",".")
        self.module = __import__(self.pluginName+'.'+relpath)
        # The script may not necessarily be called "main"
        # So we use __dict__ to look up the right name
        basename = os.path.basename(str(self.mainScript()))
        basename = os.path.splitext(basename)[0]

        self.pydataengine = self.module.__dict__[basename].CreateDataEngine(None)
        self.pydataengine.setDataEngine(self.dataEngine())
        self.pydataengine.setDataEngineScript(self)
        self.pydataengine.init()

        self.initialized = True
        return True

    def __dtor__(self):
        PythonDataEngineScript.importer.unregister_top_level(self.pluginName)
        self.pydataengine = None

    def sources(self):
        return self.pydataengine.sources()

    def serviceForSource(self,source):
        return self.pydataengine.serviceForSource(source)

    def sourceRequestEvent(self,name):
        return self.pydataengine.sourceRequestEvent(name)

    def updateSourceEvent(self,source):
        return self.pydataengine.updateSourceEvent(source)

def CreatePlugin(widget_parent, parent, component_data):
    return PythonDataEngineScript(parent)
