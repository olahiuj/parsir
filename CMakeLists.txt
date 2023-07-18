cmake_minimum_required(VERSION 3.25)
project(parsir)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(parsir
    main.cc
    utils.cc
    lr1.cc)
