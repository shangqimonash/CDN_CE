cmake_minimum_required(VERSION 3.16)
project(CDN_CE)

FIND_PACKAGE(OpenSSL REQUIRED)

# build Thrift dependencies
set(THRIFT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Thrift/server.thrift)
set(THRIFT_LIB_PATH /usr/local/lib/)

link_directories(${THRIFT_LIB_PATH})

exec_program("thrift --gen cpp -o ${CMAKE_CURRENT_SOURCE_DIR} ${THRIFT_FILE}")

add_library(gen-cpp-thrift STATIC
        gen-cpp/ComputingEngine.cpp
        gen-cpp/EdgeServer.cpp
        gen-cpp/server_constants.cpp
        gen-cpp/server_types.cpp)

target_link_libraries(gen-cpp-thrift -lthrift)

# load header
INCLUDE_DIRECTORIES(.  Core DPF Util)

# set executable outputs
ADD_EXECUTABLE(AESTest Test/AESTest.cpp)
ADD_EXECUTABLE(DPFTest Test/DPFTest.cpp DPF/DPF.cpp)
ADD_EXECUTABLE(ComputingEngine Core/ComputingEngine.cpp Core/ComputingEngineHandler.cpp Core/EdgeConnector.cpp DPF/DPF.cpp Util/Utils.cpp gen-cpp)

# use SSE/SSE2 instruction set
TARGET_COMPILE_OPTIONS(AESTest PUBLIC -maes -msse2)
TARGET_COMPILE_OPTIONS(DPFTest PUBLIC -maes -msse2)
TARGET_COMPILE_OPTIONS(ComputingEngine PUBLIC -maes -msse2)

TARGET_LINK_LIBRARIES(AESTest OpenSSL::Crypto)
TARGET_LINK_LIBRARIES(DPFTest OpenSSL::Crypto)
TARGET_LINK_LIBRARIES(ComputingEngine OpenSSL::Crypto gen-cpp-thrift)