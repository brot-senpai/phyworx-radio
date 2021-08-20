cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "-g") 
#set(CMAKE_CXX_FLAGS -DNDEBUG)
project(Phyworx LANGUAGES C CXX)

find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)


set(COMMON_DIR ${CMAKE_SOURCE_DIR}/common)

include_directories(
  /usr/local/include/
  #/usr/local/include/freetype2
  /usr/include/freetype2
  ${Vulkan_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
  ${COMMON_DIR}/imgui
  ${COMMON_DIR}/include
  ${COMMON_DIR}/sdr
  ${COMMON_DIR}/plotter
)
link_directories(${COMMON_DIR} )
file(GLOB_RECURSE SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/src/*.c
)
file(GLOB_RECURSE HEADER_FILES
  ${CMAKE_SOURCE_DIR}/include/*.h 
  ${CMAKE_SOURCE_DIR}/include/*.hpp
)
file(GLOB_RECURSE SHADER_FILES
  ${CMAKE_SOURCE_DIR}/shaders/*.frag 
  ${CMAKE_SOURCE_DIR}/shaders/*.vert 
)

file(GLOB_RECURSE COMMON_HEADERS
  ${COMMON_DIR}/include/*.h 
  ${COMMON_DIR}/include/*.hpp 
)

file(GLOB_RECURSE COMMON_SRC
  ${COMMON_DIR}/src/*.c 
  ${COMMON_DIR}/src/*.cpp 
)

file(GLOB_RECURSE IMGUI_HEADERS
  ${COMMON_DIR}/imgui/*.h 
  ${COMMON_DIR}/imgui/*.hpp 
)

file(GLOB_RECURSE IMGUI_SRC
  ${COMMON_DIR}/imgui/*.c 
  ${COMMON_DIR}/imgui/*.cpp 
)

file(GLOB_RECURSE SDR_HEADERS
  ${COMMON_DIR}/sdr/*.h 
  ${COMMON_DIR}/sdr/*.hpp 
)

file(GLOB_RECURSE SDR_SRC
  ${COMMON_DIR}/sdr/*.c 
  ${COMMON_DIR}/sdr/*.cpp 
)

file(GLOB_RECURSE PLOTTER_HEADERS
  ${COMMON_DIR}/plotter/*.h 
  ${COMMON_DIR}/plotter/*.hpp 
)

file(GLOB_RECURSE PLOTTER_SRC
  ${COMMON_DIR}/plotter/*.c 
  ${COMMON_DIR}/plotter/*.cpp 
)

add_library(plotter ${PLOTTER_HEADERS} ${PLOTTER_SRC})
target_include_directories(plotter PUBLIC ${COMMON_DIR}/plotter)

add_library(sdr ${SDR_HEADERS} ${SDR_SRC})
target_include_directories(sdr PUBLIC ${COMMON_DIR}/sdr)

add_library(imgui ${IMGUI_HEADERS} ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${COMMON_DIR}/imgui)


add_library(common ${COMMON_HEADERS} ${COMMON_SRC})
target_include_directories(common PUBLIC ${COMMON_DIR}/include)


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} )
set(LIBS  common  sdr fftw3 plotter glfw imgui LimeSuite ${Vulkan_LIBRARIES} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} ${LIBS})


