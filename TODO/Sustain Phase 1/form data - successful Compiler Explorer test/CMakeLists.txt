project(multifile)

cmake_minimum_required(VERSION 3.5)

# Compiles both files, but causes Compiler Explorer to fail to run the executable:
#add_executable(multifile sanity.cpp example.cpp)

add_executable(multifile.exe example.cpp)
 