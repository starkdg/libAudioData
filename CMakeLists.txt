cmake_minimum_required(VERSION 2.8)
project(AudioData)

set(AudioData_VERSION_MAJOR 0)
set(AudioData_VERSION_MINOR 1)

if (CMAKE_HOST_WIN32)
  #  set windows variables here:
  message("win32 system detected")
  add_definitions(-g -Wall -O3)
  set(BUILD_DLL 1)
  set(HAVE_MPG123 1)
  set(HAVE_AMR 1)
  set(MPG123_LIB "mpg123")
endif(CMAKE_HOST_WIN32)

if (CMAKE_HOST_UNIX)
   # set unix system variables
   message("unix system detected")
   add_definitions(-g -Wall -O3 -shared -fPIC)
   find_library(HAVE_MPG123 mpg123)
   find_library(HAVE_AMR opencore-amrnb)
   find_library(HAVE_SNDFILE sndfile)
   find_library(HAVE_SAMPLERATE samplerate)
   if (HAVE_MPG123)
     message("found mpg123")
     set(MPG123_LIB "mpg123")
   endif(HAVE_MPG123)
   if (HAVE_AMR)
     message("found amr")
     set(AMR_LIB "opencore-amrnb")
   endif(HAVE_AMR)
   if (HAVE_SNDFILE)
     message("found sndfile")
     set(SND_LIB "sndfile")
   endif(HAVE_SNDFILE)
   if (HAVE_SAMPLERATE)
	 message("found samplerate src")
	 set(SAMPLERATE_LIB "samplerate")
   endif(HAVE_SAMPLERATE)
endif(CMAKE_HOST_UNIX)

configure_file(
  "${PROJECT_SOURCE_DIR}/AudioDataConfig.h.in"
  "${PROJECT_BINARY_DIR}/AudioDataConfig.h"
)

include_directories("${PROJECT_BINARY_DIR}")

set(RESOURCE_DIR "resources")

add_library(AudioData SHARED audiodata.c)
target_link_libraries(AudioData  ${SND_LIB} ${MPG123_LIB} ${AMR_LIB} ${SAMPLERATE_LIB})

add_executable(TestAudioData test_audiodata.c)
target_link_libraries(TestAudioData AudioData)

#tests
enable_testing()
add_test(NAME testproject COMMAND TestAudioData ${RESOURCE_DIR}/test.wav ${RESOURCE_DIR}/test2.mp3)

#install files
install(TARGETS AudioData LIBRARY DESTINATION lib)
install(FILES audiodata.h DESTINATION include)
install(FILES AudioDataConfig.h DESTINATION include)
