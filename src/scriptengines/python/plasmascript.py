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

# Plasma applet API for Python

from PyQt4.QtCore import QObject
from PyQt4.QtGui import QGraphicsWidget
from PyKDE4.plasma import Plasma # Plasma C++ namespace

#import sip
#import gc

class Applet(QObject):
    ''' Subclass Applet in your module and return an instance of it in a global function named
    applet(). Implement the following functions to breathe life into your applet:
        * paint - Draw the applet given a QPainter and some options
    It provides the same API as Plasma.Applet; it just has slightly less irritating event names. '''
    def __init__(self, parent=None):
        # this should be set when the applet is created
        QObject.__init__(self, parent)
        #sip.settracemask(0x3f)
        self.applet = None
        self.applet_script = None
        self._forward_to_applet = True

    def __getattr__(self, key):
        # provide transparent access to the real applet instance
        if self._forward_to_applet:
            try:
                return getattr(self.applet_script, key)
            except:
                return getattr(self.applet, key)
        else:
            raise AttributeError(key)

    def _enableForwardToApplet(self):
        self._forward_to_applet = True
    def _disableForwardToApplet(self):
        self._forward_to_applet = False

    # Events
    def setApplet(self,applet):
        self.applet = applet

    def setAppletScript(self,appletScript):
        self.applet_script = appletScript

    def init(self):
        pass

    def configChanged(self):
        pass

    def paintInterface(self, painter, options, rect):
        pass

    def constraintsEvent(self, flags):
        pass

    def showConfigurationInterface(self):
        self.dlg = self.standardConfigurationDialog()
        self.createConfigurationInterface(self.dlg)
        self.addStandardConfigurationPages(self.dlg)
        self.dlg.show()

    def createConfigurationInterface(self, dlg):
        pass

    def contextualActions(self):
        return []

    def shape(self):
        return QGraphicsWidget.shape(self.applet)

    def initExtenderItem(self, item):
        print ("Missing implementation of initExtenderItem in the applet " + \
              item.config().readEntry('SourceAppletPluginName', '').toString() + \
              "!\nAny applet that uses extenders should implement initExtenderItem to " + \
              "instantiate a widget.")

    def saveState(self, config):
        pass

###########################################################################
class DataEngine(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.dataengine = None
        self.dataengine_script = None

    def setDataEngine(self,dataEngine):
        self.dataEngine = dataEngine

    def setDataEngineScript(self,dataEngineScript):
        self.data_engine_script = dataEngineScript

    def __getattr__(self, key):
        # provide transparent access to the real dataengine script instance
        try:
            return getattr(self.data_engine_script, key)
        except:
            return getattr(self.dataEngine, key)

    def init(self):
        pass

    def sources(self):
        return []

    def sourceRequestEvent(self,name):
        return False

    def updateSourceEvent(self,source):
        return False

    def serviceForSource(self,source):
        return Plasma.DataEngineScript.serviceForSource(self.data_engine_script,source)

###########################################################################
class Wallpaper(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.wallpaper = None
        self.wallpaper_script = None

    def setWallpaper(self,wallpaper):
        self.wallpaper = wallpaper

    def setWallpaperScript(self,wallpaperScript):
        self.wallpaper_script = wallpaperScript

    def __getattr__(self, key):
        # provide transparent access to the real wallpaper script instance
        try:
            return getattr(self.wallpaper_script, key)
        except:
            return getattr(self.wallpaper, key)

    def init(self, config):
        pass

    def paint(self,painter, exposedRect):
        pass

    def save(self,config):
        pass

    def createConfigurationInterface(self,parent):
        return None

    def mouseMoveEvent(self,event):
        pass

    def mousePressEvent(self,event):
        pass

    def mouseReleaseEvent(self,event):
        pass

    def wheelEvent(self,event):
        pass

    def renderCompleted(self, image):
        pass

    def urlDropped(self, url):
        pass

###########################################################################
class Runner(QObject):
    def __init__(self, parent=None):
        QObject.__init__(self, parent)
        self.runner = None
        self.runner_script = None

    def setRunner(self,runner):
        self.runner = runner

    def setRunnerScript(self,runnerScript):
        self.runner_script = runnerScript

    def __getattr__(self, key):
        # provide transparent access to the real runner script instance
        try:
            return getattr(self.runner_script, key)
        except:
            return getattr(self.runner, key)

    def init(self):
        pass

    def match(self, search):
        pass

    def run(self, search, action):
        pass

    def prepare(self):
        pass

    def teardown(self):
        pass

    def createRunOptions(self, widget):
        pass

    def reloadConfiguration(self):
        pass
