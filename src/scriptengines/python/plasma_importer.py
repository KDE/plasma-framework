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

import sys
import os
import imp

PY3 = sys.version_info[0] == 3


class PlasmaImporter(object):
    def __init__(self):
        self.toplevel = {}
        self.toplevelcount = {}
        self.marker = '<plasma>' + str(id(self))
        sys.path.append(self.marker)
        sys.path_hooks.append(self.hook)

    def hook(self,path):
      if path==self.marker:
          return self
      else:
          raise ImportError()

    def register_top_level(self,name,path):
        if name not in self.toplevel:
            self.toplevelcount[name] = 1
            self.toplevel[name] = path
        else:
            self.toplevelcount[name] += 1

    def unregister_top_level(self,name):
        value = self.toplevelcount[name]-1
        self.toplevelcount[name] = value
        if value==0:
            del self.toplevelcount[name]
            del self.toplevel[name]
            for key in list(sys.modules.keys()):
                if key==name or key.startswith(name+"."):
                    del sys.modules[key]

    def find_module(self,fullname, path=None):
        location = self.find_pymod(fullname)
        if location is not None:
            return self
        else:
            return None

    # Find the given module in the plasma directory.
    # Result a tuple on success otherwise None. The tuple contains the location
    # of the module/package in disk and a boolean indicating if it is a package
    # or module
    def find_pymod(self,modulename):
        parts = modulename.split('.')
        toppackage = parts[0]
        rest = parts[1:]

        if toppackage in self.toplevel:
            path = self.toplevel[toppackage]
            if len(rest)==0:
                # Simple top level Plasma package
                return (path,True)
            else:
                restpath = apply(os.path.join,rest)
                fullpath = os.path.join(path,'contents','code',restpath)
                if os.path.isdir(fullpath):
                    return (fullpath,True)
                elif os.path.exists(fullpath+'.py'):
                    return (fullpath+'.py',False)
        else:
            return None

    def _get_code(self,location):
        return open(location).read()

    def load_module(self, fullname):
        location,ispkg = self.find_pymod(fullname)
        if ispkg:   # Package dir.
            initlocation = os.path.join(location,'__init__.py')
            code = None
            if os.path.isfile(initlocation):
                code = open(initlocation)
        else:
            code = open(location)

        mod = sys.modules.setdefault(fullname, imp.new_module(fullname))
        mod.__file__ = location #"<%s>" % self.__class__.__name__
        mod.__loader__ = self
        if ispkg:
            mod.__path__ = [self.marker]
        if code is not None:
            try:
                if PY3:
                    exec(code in mod.__dict__)
                else:
                    exec code in mod.__dict__
            finally:
                code.close()
        return mod

#plasma_importer = PlasmaImporter()
#plasma_importer.register_top_level('plasma_pyclock','/home/sbe/devel/python_plasma_2/test/plasma-pyclock')
#from  plasma_pyclock import main
