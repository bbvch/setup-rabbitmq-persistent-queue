cmake_minimum_required (VERSION 2.6)

project (setup-rabbitmq-persistent-queue)

set(common
	utility.cpp
	utility.h
)


add_executable(setup-rabbitmq-persistent-queue
        setup-rabbitmq-persistent-queue.cpp
        ${common}
)
target_link_libraries(setup-rabbitmq-persistent-queue
        rabbitmq
)
set_target_properties(setup-rabbitmq-persistent-queue PROPERTIES
    CXX_STANDARD 11
)
target_compile_options(setup-rabbitmq-persistent-queue PRIVATE -Wall -Wextra -pedantic)
install(TARGETS setup-rabbitmq-persistent-queue DESTINATION bin)


add_executable(amqp-send
        amqp-send.cpp
        ${common}
)
target_link_libraries(amqp-send
        rabbitmq
)
set_target_properties(amqp-send PROPERTIES
    CXX_STANDARD 11
)
target_compile_options(amqp-send PRIVATE -Wall -Wextra -pedantic)
install(TARGETS amqp-send DESTINATION bin)
