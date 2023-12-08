cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_BUILD_RPATH "/Users/nfm/lib/speechsdk/MicrosoftCognitiveServicesSpeech.xcframework/macos-arm64_x86_64")

set(CMAKE_CXX_FLAGS_DEBUG  "-g")

project(TBD_TRANSLATOR VERSION 0.0.1)

add_executable(tbdtranslator
    src/main.cpp
    src/languid_config.cpp
    src/buffer.cpp
    src/speech_recognizer.cpp
    src/wav_header_remover.cpp
    src/languid_config.cpp
    src/languid_controller.cpp
    src/audio_stream_normalizer.cpp
    src/vorbis_encoder.cpp
    src/file_writer.cpp
    src/icecast_publisher.cpp)



target_include_directories(tbdtranslator
    PRIVATE
        /Users/nfm/include/MicrosoftCognitiveServicesSpeech/
        /usr/local/include/
        /usr/local/include/opus/)

add_library(MicrosoftCognitiveServicesSpeech SHARED IMPORTED)
set_target_properties(MicrosoftCognitiveServicesSpeech PROPERTIES IMPORTED_LOCATION /Users/nfm/lib/speechsdk/MicrosoftCognitiveServicesSpeech.xcframework)

find_library(
    LIB_OGG
    NAMES ogg
    REQUIRED
)

find_library(
    LIB_VORBIS
    NAMES vorbis
    REQUIRED
)

find_library(
    LIB_VORBIS_ENC
    NAMES vorbisenc
    REQUIRED
)

find_library(
    LIB_SHOUT
    NAMES shout
    REQUIRED
)

target_link_libraries(tbdtranslator
    PRIVATE
        MicrosoftCognitiveServicesSpeech
        ${LIB_OGG}
        ${LIB_VORBIS_ENC}
        ${LIB_VORBIS}
        ${LIB_SHOUT})