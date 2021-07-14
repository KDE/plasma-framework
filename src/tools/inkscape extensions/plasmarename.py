#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later
'''


import sys
sys.path.append('/usr/share/inkscape/extensions')

import inkex

from simplestyle import *
from simpletransform import *
import pathmodifier

class PlasmaNamesEffect(pathmodifier.PathModifier):
    """
    Renames 9 selected elements as a plasma theme frame
    """
    def __init__(self):
        pathmodifier.PathModifier.__init__(self)

        # Define string option "--prefix" with "-p" shortcut and default value "World".
        self.OptionParser.add_option('-p', '--prefix', action = 'store',
          type = 'string', dest = 'prefix', default = '',
          help = 'Prefix of the svg elements')

    def nodeBBox(self, node):
        path = node
        return computeBBox([path])


    def effect(self):
        # Get script's "--prefix" option value.
        prefix = self.options.prefix

        #9 elements: is a frame. 4 elements: is a border hint
        positions = []
        if len(self.selected) == 9:
            positions = ['topleft', 'left', 'bottomleft', 'top', 'center', 'bottom', 'topright', 'right', 'bottomright']
        elif len(self.selected) == 4:
            positions = ['hint-left-margin', 'hint-top-margin', 'hint-bottom-margin', 'hint-right-margin']
        else:
            return

        #some heuristics to normalize the values, find the least coords and size
        minX = 9999
        minY = 9999
        minWidth = 9999
        minHeight = 9999
        for id, node in self.selected.iteritems():
            nodeBox = self.nodeBBox(node)
            minX = min(minX, int(nodeBox[0]))
            minY = min(minY, int(nodeBox[2]))
            minWidth = min(minWidth, int(nodeBox[1] - nodeBox[0]))
            minHeight = min(minHeight, int(nodeBox[3] - nodeBox[2]))


        nodedictionary = {}
        for id, node in self.selected.iteritems():
            nodeBox = self.nodeBBox(node)
            x = int(nodeBox[0])/minWidth - minX
            y = int(nodeBox[2])/minHeight - minY
            nodedictionary[x*1000 + y] = node

        keys = nodedictionary.keys();
        keys.sort();
        i = 0

        for (k) in keys:
            name = ''
            if prefix:
                name = '%s-%s' % (prefix, positions[i])
            else:
                name = '%s' % (positions[i])
            nodedictionary[k].set('id', name)
            i = i+1

# Create effect instance and apply it.
effect = PlasmaNamesEffect()
effect.affect()
