# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("mozjpeg")

add_library(libturbojpeg STATIC IMPORTED)
add_library(libyuv STATIC IMPORTED)

set_target_properties(libturbojpeg PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libturbojpeg.a)
set_target_properties(libyuv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${ANDROID_ABI}/libyuv.a)

add_library( # Sets the name of the library.
        mozjpeg
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        mozjpeg.cpp)
target_include_directories(mozjpeg PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/libyuv)
find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        mozjpeg
        ${log-lib} libturbojpeg -ljnigraphics libyuv)