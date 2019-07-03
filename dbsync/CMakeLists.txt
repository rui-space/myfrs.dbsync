project(dbsync)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++0x")
find_package(CURL)
include_directories(/usr/local/mycpp/include)
target_link_libraries (${PROJECT_NAME} /usr/local/mycpp/libmycpp.so rt uuid pthread ssl odbc jsoncpp ${CURL_LIBRARY} )

# Install section
install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /opt/lynx/${PROJECT_NAME})
install (FILES "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.service" DESTINATION /etc/systemd/system)

# Packaging section
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME}")
set(CPACK_PACKAGE_DESCRIPTION "database sync")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "lynx@lynxattractions.com")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS 1)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst;${CMAKE_CURRENT_SOURCE_DIR}/debian/prerm;${CMAKE_CURRENT_SOURCE_DIR}/debian/conffiles")

include(CPack)
