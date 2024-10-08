cmake_minimum_required(VERSION 3.29)

project(wnpcli)
file(READ "VERSION" PROJECT_VERSION)

find_package(libwnp REQUIRED)

set(SRC_FILES
  src/daemon.c
  src/wnpcli.c
  deps/cargs.c
)

add_compile_definitions(WNPCLI_VERSION="${PROJECT_VERSION}")

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps>
)

target_link_libraries(${PROJECT_NAME} PRIVATE libwnp::libwnp)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "webnowplaying@keifufu.dev")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "wnpcli - A Command Line Interface for WebNowPlaying")
set(CPACK_PACKAGE_VENDOR "keifufu")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(CPACK_GENERATOR "DEB;RPM;TGZ")
  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "keifufu <webnowplaying@keifufu.dev>")
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_linux_amd64")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  unset(CPACK_PACKAGE_DESCRIPTION_FILE)
  unset(CPACK_RESOURCE_FILE_LICENSE)
  unset(CPACK_RESOURCE_FILE_README)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
  set(CPACK_GENERATOR "productbuild")
  set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/bin")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_macos_aarch64")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
  set(CPACK_NSIS_INSTALLER_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_win64")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}_win64-installer")
endif()

include(CPack)
cpack_add_component(CPACK_COMPONENTS_ALL REQUIRED) # packagebuild fix
