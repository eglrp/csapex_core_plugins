cmake_minimum_required(VERSION 2.8.3)
project(csapex_vision)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS utils_vision csapex csapex_ros csapex_core_plugins cv_bridge)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

find_package(OpenCV REQUIRED)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
   INCLUDE_DIRS include src
   LIBRARIES csapex_vision
   CATKIN_DEPENDS csapex utils_vision csapex_core_plugins
   DEPENDS OpenCV
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(include
  ${catkin_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS}
)

qt5_add_resources(QT_RESOURCES res/csapex_vision_resources.qrc)

add_library(csapex_vision
    src/core/filter.cpp
    src/core/encoding.cpp
    src/core/cv_mat_message.cpp
    src/core/roi.cpp
    src/core/roi_message.cpp
    src/core/cv_pyramid_message.cpp
    src/core/yaml_io.cpp

    ${QT_RESOURCES}
)
target_link_libraries(csapex_vision
    yaml-cpp ${QT_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${catkin_LIBRARIES})



add_library(csapex_vision_plugin_core
    src/plugin/register_plugin.cpp
    src/core/image_renderer.cpp
    src/core/image_provider.cpp
    src/plugin/image_provider_dir.cpp
    src/plugin/image_provider_img.cpp
    src/plugin/image_provider_mov.cpp
    src/plugin/image_provider_set.cpp
)
target_link_libraries(csapex_vision_plugin_core csapex_vision
    yaml-cpp ${QT_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${catkin_LIBRARIES})


add_library(csapex_vision_plugin_node
    src/plugin/static_roi.cpp
    src/plugin/rotate_image.cpp
    src/plugin/grab_cut.cpp
    src/plugin/camera.cpp
    src/plugin/image_padding.cpp
    src/plugin/copymakeborder.cpp
    src/plugin/color_segmentation.cpp
    src/plugin/filter_apply_mask.cpp

    src/plugin/filter_static_mask.cpp

    src/plugin/mask_refinement.cpp

    src/plugin/hough_circle.cpp
    src/plugin/vectorize_pyramid.cpp

    src/plugin/image_roi.cpp
    src/plugin/grid_arranged_rois.cpp

    src/plugin/row.cpp
    src/plugin/col.cpp
    src/plugin/minmax_loc.cpp
    src/plugin/set_color.cpp
    src/plugin/reshape.cpp
    src/plugin/mean_dev_normalization.cpp

    src/plugin/levels.cpp
    src/plugin/blur.cpp
    src/plugin/gaussian_blur.cpp
    src/plugin/bilateral.cpp
    src/plugin/mean_dev.cpp

    src/plugin/load_filestorage.cpp
)
target_link_libraries(csapex_vision_plugin_node csapex_vision
    yaml-cpp ${QT_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${catkin_LIBRARIES})


add_library(csapex_vision_plugin_qt
    src/plugin/filter_static_mask_painter.cpp
    src/plugin/filter_static_mask_adapter.cpp
    src/plugin/mask_refinement_adapter.cpp
    src/plugin/image_roi_adapter.cpp
)
target_link_libraries(csapex_vision_plugin_qt csapex_vision
    yaml-cpp ${QT_LIBRARIES} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${catkin_LIBRARIES})


#add_executable(vision_evaluator src/deprecated.cpp)

#
# INSTALL
#

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS csapex_vision
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

install(TARGETS csapex_vision_plugin_core
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(TARGETS csapex_vision_plugin_node
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(TARGETS csapex_vision_plugin_qt
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})

install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
