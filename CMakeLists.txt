cmake_minimum_required(VERSION 3.1)
project(telegram_module)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
find_package(CURL REQUIRED) 
find_package(nlohmann_json REQUIRED)
if (NOT OPENSSL_FOUND)
  find_package(OpenSSL REQUIRED)
  find_package(ZLIB REQUIRED)
endif()
add_subdirectory(connector_manager)
link_directories(lib)
include_directories(${CURL_INCLUDE_DIR})
include_directories("include")
include_directories("connector_manager/include")
add_executable(${PROJECT_NAME} main.cpp telegram.cpp )
target_link_libraries(${PROJECT_NAME} libtdclient.a libtdcore.a libtdapi.a libtdactor.a libtdnet.a libtdutils.a libtddb.a libtdsqlite.a)
target_link_libraries(${PROJECT_NAME} ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} ${CMAKE_DL_LIBS} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} connector_manager nlohmann_json::nlohmann_json)