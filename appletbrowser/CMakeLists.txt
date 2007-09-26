
set(
  plasma_appletbrowser_SRCS
  kcategorizeditemsview.cpp
  kcategorizeditemsviewmodels.cpp
  kcategorizeditemsviewdelegate.cpp
  appletbrowserwindow.cpp
  plasmaappletitemmodel.cpp
)

kde4_add_ui_files(
  plasma_appletbrowser_SRCS
  kcategorizeditemsviewbase.ui
  appletbrowserwindowbase.ui
)

