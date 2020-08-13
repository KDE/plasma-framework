# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2008 Simon Edwards <simon@simonzone.com>
#
# SPDX-License-Identifier: LGPL-2.0-or-later

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
