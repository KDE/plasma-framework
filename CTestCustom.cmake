# This file contains all the specific settings that will be used
# when running 'make Experimental' or 'make ExperimentalBuild'

# Change the maximum warnings that will be displayed
# on the report page (default 50)
set(CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS 3000)

set(CTEST_CUSTOM_WARNING_EXCEPTION "kdecore/network/k3socket[a-z]+\\.h"
                                   "kdecore/network/k3clientsocketbase\\.h"
                                   "Warning: No relevant classes found. No output generated."  # moc didn't find anything to do
                                   "yy" "YY"                                                   # ignore the warnings in generated code
   )
set(CTEST_CUSTOM_ERROR_EXCEPTION "ICECC")
set(CTEST_CUSTOM_COVERAGE_EXCLUDE ".moc$" "moc_" "ui_")
