cmake_minimum_required(VERSION 3.15)
project(BoostUDP)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем путь к Boost
include(${CMAKE_BINARY_DIR}/build/generators/conan_toolchain.cmake)

# Добавляем исходники
add_executable(server src/server.cpp)
add_executable(client src/client.cpp)

# Линкуем Boost с проектами
find_package(Boost REQUIRED)
target_link_libraries(server PRIVATE Boost::boost)
target_link_libraries(client PRIVATE Boost::boost)
