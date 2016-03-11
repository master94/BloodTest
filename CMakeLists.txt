set(NAME BloodSampleAnalysis)

project(${NAME})

find_package(OpenCV REQUIRED)

add_executable(${NAME} main.cpp)
target_link_libraries(${NAME} ${OpenCV_LIBS})
