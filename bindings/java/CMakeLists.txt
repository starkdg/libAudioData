cmake_minimum_required(VERSION 2.8)
project(AudioData-jni)
set(audioData-jni_VERSION_MAJOR 0)
set(audioData-jni_VERSION_MINOR 1)

find_package(JNI)

include_directories(${JNI_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})

add_library(AudioData-jni SHARED org_phash_AudioData.cpp)

target_link_libraries(AudioData-jni AudioData)

install(FILES org_phash_AudioData.h DESTINATION include)
install(TARGETS AudioData-jni DESTINATION lib)