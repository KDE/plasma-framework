# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2008 Simon Edwards <simon@simonzone.com>
# SPDX-FileCopyrightText: 2009 Petri Damstén <damu@iki.fi>
#
# SPDX-License-Identifier: LGPL-2.0-or-later

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyKDE4.plasma import Plasma
import plasma_importer
import os.path

class PythonRunnerScript(Plasma.RunnerScript):
    importer = None

    def __init__(self, parent):
        Plasma.RunnerScript.__init__(self,parent)
        if PythonRunnerScript.importer is None:
            PythonRunnerScript.importer = plasma_importer.PlasmaImporter()
        self.initialized = False

    def init(self):
        self.moduleName = str(self.runner().package().metadata().pluginName())
        self.pluginName = 'runner_' + self.moduleName.replace('-','_')

        PythonRunnerScript.importer.register_top_level(self.pluginName, str(self.runner().package().path()))

        # import the code at the file name reported by mainScript()
        relpath = os.path.relpath(str(self.mainScript()),str(self.runner().package().path()))
        if relpath.startswith("contents/code/"):
            relpath = relpath[14:]
        if relpath.endswith(".py"):
            relpath = relpath[:-3]
        relpath = relpath.replace("/",".")
        self.module = __import__(self.pluginName+'.'+relpath)
        basename = os.path.basename(str(self.mainScript()))
        basename = os.path.splitext(basename)[0]
        self.pyrunner = self.module.__dict__[basename].CreateRunner(None)
        self.pyrunner.setRunner(self.runner())
        self.pyrunner.setRunnerScript(self)
        self.connect(self, SIGNAL('prepare()'), self, SLOT('prepare()'))
        self.connect(self, SIGNAL('teardown()'), self, SLOT('teardown()'))
        self.connect(self, SIGNAL('createRunOptions(QWidget*)'),
                     self, SLOT('createRunOptions(QWidget*)'))
        self.connect(self, SIGNAL('reloadConfiguration()'),
                     self, SLOT('reloadConfiguration()'))
        self.pyrunner.init()

        self.initialized = True
        return True

    def __dtor__(self):
        PythonRunnerScript.importer.unregister_top_level(self.pluginName)
        self.pyrunner = None

    def match(self, search):
        self.pyrunner.match(search)

    def run(self, search, action):
        self.pyrunner.run(search, action)

    @pyqtSignature("prepare()")
    def prepare(self):
        self.pyrunner.prepare()

    @pyqtSignature("teardown()")
    def teardown(self):
        self.pyrunner.teardown()

    @pyqtSignature("createRunOptions(QWidget*)")
    def createRunOptions(self, widget):
        self.pyrunner.createRunOptions(widget)

    @pyqtSignature("reloadConfiguration()")
    def reloadConfiguration(self):
        self.pyrunner.reloadConfiguration()

def CreatePlugin(widget_parent, parent, component_data):
    return PythonRunnerScript(parent)
