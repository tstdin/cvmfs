#!/bin/sh

make -C build clean
make -C build

cp -rv librabbitmq/amqp.h \
       librabbitmq/amqp_framing.h \
       librabbitmq/amqp_tcp_socket.h \
       librabbitmq/amqp_ssl_socket.h \
       $EXTERNALS_INSTALL_LOCATION/include/
cp -rv build/librabbitmq/librabbitmq.a $EXTERNALS_INSTALL_LOCATION/lib/
