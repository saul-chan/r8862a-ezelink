cmake_minimum_required(VERSION 2.6)

PROJECT(cls-qos C)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

IF(APPLE)
  INCLUDE_DIRECTORIES(/opt/local/include)
  LINK_DIRECTORIES(/opt/local/lib)
ENDIF()

IF(DEBUG)
  ADD_DEFINITIONS(-DDEBUG -g3)
ENDIF()

ADD_LIBRARY(rpcd-clsqos MODULE rpcd-clsqos.c)
TARGET_LINK_LIBRARIES(rpcd-clsqos uci ubox ubus)
SET_TARGET_PROPERTIES(rpcd-clsqos PROPERTIES OUTPUT_NAME clsqos PREFIX "")
INSTALL(TARGETS rpcd-clsqos LIBRARY DESTINATION lib/rpcd)

ADD_EXECUTABLE(cls-qos classic_qos.c)
TARGET_LINK_LIBRARIES(cls-qos uci ubox)
INSTALL(TARGETS cls-qos RUNTIME DESTINATION sbin)
