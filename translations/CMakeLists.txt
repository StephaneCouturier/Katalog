# Shared translation sources for all Katalog variants.
#
# .ts source files live here and are shared across qt_widgets/, qt_quick/, etc.
# Each variant's CMakeLists.txt references these files via ${CMAKE_SOURCE_DIR}/translations/.
#
# Translation targets (lupdate/lrelease) are currently defined in qt_widgets/CMakeLists.txt
# because qt6_add_translations() is tied to an executable target. When qt_quick/ is added,
# its CMakeLists.txt will reference the same .ts files here.
