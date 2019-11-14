
file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.hpp)

init_target (simple_jpeg_broadcast_server)

find_package(OpenCV
  COMPONENTS
  core
  imgcodecs
  videoio
)

if(OpenCV)
    include_directories(${OpenCV_INCLUDE_DIRS})

    build_executable (${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})

    link_boost ()
    final_target ()

    target_link_libraries(${TARGET_NAME} ${OpenCV_LIBS})
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "examples")
endif(OpenCV)
