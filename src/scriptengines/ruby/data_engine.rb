=begin
 *   Copyright 2008 by Richard Dale <richard.j.dale@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
=end

require 'plasma_applet'

module PlasmaScripting
  class DataEngine < Qt::Object
    slots :updateAllSources, "removeSource(QString)"
    signals "sourceAdded(QString)", "sourceRemoved(QString)"

    attr_accessor :data_engine_script

    def initialize(parent, args = nil)
      super(parent)
      @data_engine_script = parent
      connect(@data_engine_script.dataEngine, SIGNAL("sourceAdded(QString)"), self, SIGNAL("sourceAdded(QString)"))
      connect(@data_engine_script.dataEngine, SIGNAL("sourceRemoved(QString)"), self, SIGNAL("sourceRemoved(QString)"))
    end

    def init
    end

    def sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
    end

    def setData(*args)
      if args.length == 2 && !args[1].kind_of?(Qt::Variant)
        args[1] = Qt::Variant.fromValue(args[1])
      elsif args.length == 3 && !args[2].kind_of?(Qt::Variant)
        args[2] = Qt::Variant.fromValue(args[2])
      end
      @data_engine_script.setData(*args)
    end

    def removeAllData(source)
      @data_engine_script.removeAllData(source)
    end

    def removeData(source, key)
      @data_engine_script.removeData(source, key)
    end

    def setMaxSourceCount(limit)
      @data_engine_script.setMaxSourceCount(limit)
    end

    def maxSourceCount=(limit)
      setMaxSourceCount(limit)
    end

    def setMinimumPollingInterval(minimumMs)
      @data_engine_script.setMinimumPollingInterval(minimumMs)
    end

    def minimumPollingInterval=(minimumMs)
      setMinimumPollingInterval(minimumMs)
    end

    def minimumPollingInterval
      @data_engine_script.minimumPollingInterval
    end

    def setPollingInterval(frequency)
      @data_engine_script.setPollingInterval(frequency)
    end

    def pollingInterval=(frequency)
      setPollingInterval(frequency)
    end

    def removeAllSources
      @data_engine_script.removeAllSources
    end

    def sources
      return []
    end

    def removeSource(source)
      @data_engine_script.dataEngine.removeSource(source)
    end

    def updateAllSources
      @data_engine_script.dataEngine.updateAllSources
    end
  end
end

module PlasmaScriptengineRuby
  class DataEngine < Plasma::DataEngineScript
    def initialize(parent, args)
      super(parent)
    end

    def camelize(str)
      str.gsub(/(^|[._-])(.)/) { $2.upcase }
    end

    def init
      puts "RubyAppletScript::DataEngine#init mainScript: #{mainScript}"
      program = Qt::FileInfo.new(mainScript)
      $: << program.path
      load Qt::File.encodeName(program.filePath).to_s
      moduleName = camelize(Qt::Dir.new(package.path).dirName)
      className = camelize(program.baseName)
      puts "RubyAppletScript::DataEngine#init instantiating: #{moduleName}::#{className}"
      klass = Object.const_get(moduleName.to_sym).const_get(className.to_sym)
      @data_engine_script = klass.new(self)
      @data_engine_script.init
      return true
    end

    def sources
      @data_engine_script.sources
    end

    def sourceRequestEvent(name)
      @data_engine_script.sourceRequestEvent(name)
    end

    def updateSourceEvent(source)
      @data_engine_script.updateSourceEvent(source)
    end
  end
end

# kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;

