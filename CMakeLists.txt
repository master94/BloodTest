set(NAME BloodSampleAnalysis)

project(${NAME})

add_definitions(-std=c++11)
find_package(OpenCV REQUIRED)

add_executable(${NAME} main.cpp analysis.cpp)
target_link_libraries(${NAME} ${OpenCV_LIBS})
