FSINK Framework

## 0. BRIEF INTRODUCTION
    FSink provides an unified interface for bitstream output.
    The framework contains three major parts:
        1. Sink: An abstract interface that contains the SinkOps operating functions and information.
        2. SinkOps: A set of function pointers that points to a set of functions defined by programmer, and passed to "ops" pointer in Sink.
        3. Info: Any struct defined by programmer and passed to "info" pointer in Sink.

## 1. USING UDP SINK OPS
    UDP socket is encapsulated as a bistream sink in "udp_socket.c".
    The header file "udp_socket.h" defines a type, UdpSockAttr, that stores info required by UDP Sink.
    Besides, a pair of function is also provided to create/release UDP Sink.

    1. Programmer should create an UDP before using the callback functions:
           Sink * udp_sink = createUdpSink("udp_socket", DSTN_IP, DSTN_PORT);

       where the DSTN_IP and DSTN_PORT are destination IP address and port that "receives" the output bistream.

    2. Once the UDP Sink is created, the open/write/close functions implemented in UDP SinkOps can be used to send the bitstream to the UDP socket.

        1. To open an UDP socket:
            udp_sink->ops->open(udp_sink->info, 0);
        2. To write data to the UDP socket:
            udp->ops->write(udp_sink->info, SRC_ADDR, SRC_SIZE, 0);
        3. To close the UDP socket:
            udp->ops->close(udp_sink->info);

    3. Once the transmission is finished, the UDP Sink can be released, freeing the allocated memories:
           releaseUdpSink(udp_socket);

    Please refer to udp_stream_server for sample code (main.c and mpi_ops.c) on using the UDP Sink.

