cmake_minimum_required(VERSION 2.8.3)
project(csapex_point_cloud)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
add_definitions("-O3")

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
        csapex
        csapex_ros
        csapex_transform
        csapex_opencv
        cslibs_indexed_storage
)

find_package(pcl_ros REQUIRED)

find_package(Qt5 COMPONENTS Core OpenGL Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)

find_package(PCL REQUIRED COMPONENTS common filters segmentation io)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})


catkin_package(
  INCLUDE_DIRS include ${PCL_INCLUDE_DIRS}
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS csapex
#  DEPENDS system_lib
)


include_directories(
  include
  src/math/
  ${catkin_INCLUDE_DIRS}
  ${pcl_ros_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
    src/msg/point_cloud_message.cpp
    src/msg/model_message.cpp
    src/msg/normals_message.cpp
    src/msg/indices_message.cpp
    src/msg/binary_io.cpp

    # qtcretor visibility
    include/csapex_point_cloud/msg/point_cloud_message.h
    include/csapex_point_cloud/msg/model_message.h
    include/csapex_point_cloud/msg/normals_message.h
    include/csapex_point_cloud/msg/indices_message.h

)
target_link_libraries(${PROJECT_NAME}
    yaml-cpp ${catkin_LIBRARIES})


add_library(${PROJECT_NAME}_plugin_core
    src/register_plugin.cpp
)

target_link_libraries(${PROJECT_NAME}_plugin_core
    yaml-cpp ${QT_LIBRARIES} ${catkin_LIBRARIES} Qt5::OpenGL
    ${PROJECT_NAME} ${PCL_COMMON_LIBRARIES} ${PCL_FILTERS_LIBRARIES} ${PCL_SEGMENTATION_LIBRARIES}
)

add_library(${PROJECT_NAME}_plugin_node
    src/operations/merge_clouds.cpp
    src/operations/transform_cloud.cpp
    src/operations/extract_timestamp.cpp
    src/operations/row.cpp
    src/operations/col.cpp
    src/operations/set_timestamp.cpp
    src/operations/color_point_cloud.cpp
    src/operations/scale_intensity.cpp
    src/operations/ray_angles.cpp
    src/operations/estimate_normals.cpp
    src/operations/estimate_center.cpp
    src/operations/merge_indices.cpp

    src/visualization/point_count.cpp
    src/visualization/cloud_renderer.cpp
    src/visualization/clusters_to_markerarray.cpp
    src/visualization/model_to_marker.cpp

    src/sampling/voxel_grid.cpp

    src/labeling/cloud_labeler.cpp

    src/conversion/label_clustered_pointcloud.cpp
    src/conversion/color_labeled_point_cloud.cpp
    src/conversion/labeled_cloud_to_indices.cpp
    src/conversion/pointcloud_to_depthimage.cpp
    src/conversion/pointcloud_to_colorimage.cpp
    src/conversion/pointcloud_to_intensityimage.cpp
    src/conversion/pointcloud_to_pointmatrix.cpp
    src/conversion/pointcloud_to_image.cpp
    src/conversion/pointmatrix_to_pointcloud.cpp
    src/conversion/image_to_point_cloud.cpp
    src/conversion/pointcloud_to_xyz.cpp
    src/conversion/indices_to_mask.cpp
    src/conversion/indices_to_rois.cpp
    src/conversion/to_vector_of_indices.cpp
    src/conversion/normals_to_colorimage.cpp

    src/filter/conditional_outlier_removal.cpp
    src/filter/statistical_outlier_removal.cpp
    src/filter/radius_outlier_removal.cpp
    src/filter/threshold_outlier_revoval.cpp
    src/filter/pointcloud_validity.cpp
    src/filter/coordinate_swapper.cpp
    src/filter/indexed_pointcloud.cpp
    src/filter/index_filter.cpp
    src/filter/edge_filter.cpp
    src/filter/roi_extractor.cpp
    src/filter/passthrough.cpp
    src/filter/passthrough_by_cluster.cpp
    src/filter/passthrough_by_indices.cpp
    src/filter/crop_box.cpp
    src/filter/filter_rois_by_distance.cpp
    src/filter/filter_indices_by_roi_classifciation.cpp

    src/segmentation/label_pointcloud.cpp
    src/segmentation/split_clustered_cloud.cpp
    src/segmentation/merge_clusters.cpp
    src/segmentation/extract_clusters.cpp
    src/segmentation/sac_fit.cpp
    src/segmentation/plane_segmentation.cpp

    src/clustering/cluster.cpp
    src/clustering/cluster.hpp
    src/clustering/cluster_pcl.cpp
    src/clustering/cluster_pcl.h
    src/clustering/polar_clustering.hpp
    src/clustering/data/voxel_data.hpp
    src/clustering/data/voxel_index.hpp
    src/clustering/data/feature_color.hpp
    src/clustering/data/feature_distribution.hpp
    src/clustering/data/feature_normal.hpp
    src/clustering/data/feature_helpers.hpp
    src/clustering/storage/cluster_op.hpp
    src/clustering/storage/storage_ops.hpp
    src/clustering/validator/noop_validator.hpp
    src/clustering/validator/distribution_validator.hpp
    src/clustering/validator/color_validator.hpp
    src/clustering/validator/color_differences.hpp
    src/clustering/validator/normal_validator.hpp

    src/provider/pcd_provider.cpp

    src/project_z.cpp
    src/cloud_test.cpp

    src/transform_from_models.cpp


    # qt creator header visibility
    include/csapex_point_cloud/math/distribution.hpp
    include/csapex_point_cloud/math/mean.hpp
    include/csapex_point_cloud/math/plane.hpp
)

target_link_libraries(${PROJECT_NAME}_plugin_node
    yaml-cpp ${QT_LIBRARIES} ${catkin_LIBRARIES} Qt5::OpenGL
    ${PROJECT_NAME} ${PCL_COMMON_LIBRARIES} ${PCL_FILTERS_LIBRARIES} ${PCL_SEGMENTATION_LIBRARIES} ${PCL_IO_LIBRARIES}
)

add_library(${PROJECT_NAME}_plugin_qt
    src/visualization/point_count_adapter.cpp
    src/visualization/cloud_renderer_adapter.cpp
    src/labeling/cloud_labeler_adapter.cpp
)

target_link_libraries(${PROJECT_NAME}_plugin_qt
    yaml-cpp ${QT_LIBRARIES} ${catkin_LIBRARIES} Qt5::OpenGL
    ${PROJECT_NAME} ${PCL_COMMON_LIBRARIES} ${PCL_FILTERS_LIBRARIES} ${PCL_SEGMENTATION_LIBRARIES}
)


#
# INSTALL
#

install(FILES plugins.xml
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_plugin_core ${PROJECT_NAME}_plugin_node ${PROJECT_NAME}_plugin_qt
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})

