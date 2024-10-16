cmake_minimum_required(VERSION 3.29)
project(Vic3MapUtils)

set(CMAKE_CXX_STANDARD 20)

add_executable(Vic3MapUtils src/main.cpp
		src/SplineNetwork/SplineNetwork.cpp
		src/SplineNetwork/SplineNetwork.hpp)

include(FetchContent)

FetchContent_Declare(
		fmt
		GIT_REPOSITORY https://github.com/fmtlib/fmt
		GIT_TAG 0c9fce2ffefecfdce794e1859584e25877b7b592 # 11.0.2
)
FetchContent_MakeAvailable(fmt)
target_link_libraries(Vic3MapUtils fmt::fmt)
