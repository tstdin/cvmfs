# - Try to find RabbitMQC
#
# Once done this will define
#
#  RabbitMQC_FOUND - system has RabbitMQC
#  RabbitMQC_INCLUDE_DIRS - the RabbitMQC include directory
#  RabbitMQC_LIBRARIES - Link these to use RabbitMQC
#

find_path(
    RabbitMQC_INCLUDE_DIRS
    NAMES amqp.h
    HINTS ${RabbitMQC_INCLUDE_DIRS}
)

find_library(
    RabbitMQC_LIBRARY
    NAMES rabbitmq
    HINTS ${RabbitMQC_LIBRARY_DIRS}
)

set(RabbitMQC_LIBRARIES ${RabbitMQC_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    RabbitMQC
    DEFAULT_MSG
    RabbitMQC_LIBRARIES
    RabbitMQC_INCLUDE_DIRS
)

if(RabbitMQC_FOUND)
    mark_as_advanced(RabbitMQC_LIBRARIES RabbitMQC_INCLUDE_DIRS)
endif()
