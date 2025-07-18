/* -*- c++ -*- */
/*
 * Copyright 2025 Skandalis Georgios
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "gnuradio/block.h"
#include "gnuradio/network/tcp_source.h"
#include "gnuradio/sptr_magic.h"
#include "gnuradio/types.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)

#define WINDOWS_SOCKETS

#pragma commend(lib, "Ws2_32.lib")

#include <Winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tcp_source_impl.h"

namespace gr {
namespace network {

typedef struct addrinfo addrinfo_t;

tcp_source::sptr tcp_source::make(size_t itemsize,
                                  size_t veclen,
                                  const std::string& host,
                                  const std::string& port,
                                  int source_mode)
{
    return gnuradio::make_block_sptr<tcp_source_impl>(
        itemsize, veclen, host, port, source_mode);
}

tcp_source_impl::tcp_source_impl(size_t item_size,
                                 size_t vec_len,
                                 const std::string& host,
                                 const std::string& port,
                                 int source_mode)
    : d_connection_mode(source_mode), d_block_size(item_size * vec_len), d_veclen(vec_len)
{
#ifdef WINDOWS_SOCKETS

    WSADATA wsaData;

    int wsa_res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (wsa_res != 0) {
        d_logger->error("WSAStartup failed with error code {}.", res);
        throw std::runtime_error("Error while initializing Windows sockets.");
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        d_logger->error("Winsock version 2.2 not available.");
        WSACleanup();
        throw std::runtime_error("Error while initializing Windows sockets.");
    }

#endif

    struct addrinfo hints;
    // very ugly but its either that or make an entire class for a single pointer that
    // gets used a single time
    std::unique_ptr<addrinfo_t, void (*)(addrinfo_t*)> host_info(new addrinfo_t,
                                                                 freeaddrinfo);

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Get IP version, port, e.t.c in a workable format
    int res;
    addrinfo_t* tmp;
    if (d_connection_mode == TCP_SOURCE_MODE_CLIENT) {
        res = getaddrinfo(host.c_str(), port.c_str(), &hints, &tmp);
    } else {
        res = getaddrinfo(NULL, port.c_str(), &hints, &tmp);
    }
    host_info.reset(tmp);

    if (res != 0) {
#ifdef WINDOWS_SOCKETS
        d_logger->erorr("Error while initializing: {}", WSAGetLastError());
#else
        d_logger->error("Error while initializing: {}", gai_strerror(res));
#endif
        throw std::runtime_error("TCP Source encountered an error while initializing");
    }

    bool failed = false;
    d_logger->info("Trying to find a usable address.");
    // loop over the results given by gai and attempt to bind/connect to the first
    // available one, if we can't throw an exception and die.
    for (struct addrinfo* i = host_info.get(); i != NULL; i = i->ai_next) {

        // Create the socket and also bind to a a port if we're the server
        if (d_connection_mode == TCP_SOURCE_MODE_SERVER) {
            d_socket = socket(host_info.get()->ai_family,
                              host_info->ai_socktype,
                              host_info->ai_protocol);
            if (d_socket == -1) {
#ifdef WINDOWS_SOCKETS
                d_logger->warn("Failed to initialize socket: {}", WSAGetLastError());
#else
                d_logger->warn("Failed to initialize socket: {}", strerror(errno));
#endif
                failed = true;
                continue;
            }

            // get rid of "Address already in use" error
            int yes = 1;
            setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            int r = bind(d_socket, host_info->ai_addr, host_info->ai_addrlen);
            if (r == -1) {
#ifdef WINDOWS_SOCKETS
                d_logger->warn("Failed to bind socket: {}", WSAGetLastError());
#else
                d_logger->warn("Failed to bind socket: {}", strerror(errno));
#endif
                failed = true;
                continue;
            }

            // set the listening queue to 1 because in a flowgraph we expect to be an
            // single sink-source pair per tcp connection
            r = listen(d_socket, 1);
            if (r == -1) {
#ifdef WINDOWS_SOCKETS
                d_logger->warn("Error while attempting to listen for connections: {}.",
                               WSAGetLastError());

#else
                d_logger->warn("Error while attempting to listen for connections: {}.",
                               strerror(errno));
#endif
                failed = true;
                continue;
            }

            // successully created socket and bound to address, copy the data over
            // and exit loop
            failed = false;
            d_host_addr_len = i->ai_addrlen;
            memcpy(&d_host_addr, i->ai_addr, d_host_addr_len);
            break;
        } else {
            d_socket = socket(
                host_info->ai_family, host_info->ai_socktype, host_info->ai_protocol);
            if (d_socket == -1) {
#ifdef WINDOWS_SOCKETS
                d_logger->warn("Failed to initialize socket: {}", WSAGetLastError());
#else
                d_logger->warn("Failed to initialize socket: {}", strerror(errno));
#endif
                failed = true;
                continue;
            }

            failed = false;
            d_host_addr_len = i->ai_addrlen;
            memcpy(&d_host_addr, i->ai_addr, d_host_addr_len);
        }
    }

    if (failed) {
        d_logger->error("Could not find a usable address, aborting.");
        throw std::runtime_error(
            "TCP Source could encountered an error while initializing socket");
    }

    d_logger->info("Successfully initialized socket.");
}

tcp_source_impl::~tcp_source_impl()
{

#ifdef WINDOWS_SOCKETS
    closesocket(d_socket);
    WSACleanup();
#else
    close(d_socket);
#endif
}


void tcp_source_impl::establish_connection()
{
    if (d_connection_mode == TCP_SOURCE_MODE_CLIENT) {
        int res = connect(d_socket, (struct sockaddr*)&d_host_addr, d_host_addr_len);

        if (res < 0) {
#ifdef WINDOWS_SOCKETS
            d_logger->error("Could not connect to specified server: {}",
                            WSAGetLastError());
#else
            d_logger->error("Could not connect to specified server: {}", strerror(errno));
#endif
        } else {
            d_logger->info("Successfully connected to server.");
            d_connected = true;
            d_exchange_socket = d_socket;
            return;
        }
    } else {
        d_exchange_socket = accept(d_socket, NULL, NULL);
        if (d_exchange_socket < 0) {
#ifdef WINDOWS_SOCKETS
            d_logger->error("Could not accept connection: {}", WSAGetLastError());
#else
            d_logger->error("Could not accept connection: {}", strerror(errno));
#endif
            return;
        }

        d_logger->info("Successfully connected to Client.");
        d_connected = true;
    }

    return;
}

int tcp_source_impl::work(int noutput_items,
                          gr_vector_const_void_star& input_items,
                          gr_vector_void_star& output_items)
{

    while (!d_connected) {
        // if we're not connected, then attempt to connect with something, also
        // block until we succeed
        this->establish_connection();
    }

    int* out = (int*)output_items[0];

    int to_be_received = d_block_size * noutput_items;

    // we have connection, good, then we receive.
    while (to_be_received > 0) {
        int received = recv(d_exchange_socket, out, to_be_received, 0);

        // error encountered
        if (received == -1) {

#ifdef WINDOWS_SOCKETS
            d_logger->error("Error encountered while attempting to receive data: {}",
                            WSAGetLastError());

            // notify the user in case of disconnects
            if (errno == WSAECONNRESET || errno == WSAETIMEDOUT) {
                d_logger->warn(
                    "Connection has been shut down, waiting for a new connection.");
                if (d_connection_mode == TCP_SOURCE_MODE_SERVER) {
                    closesocket(d_exchange_socket);
                }

                d_connected = false;
            }

#else
            d_logger->error("Error encountered while attempting to receive data: {}",
                            strerror(errno));

            // notify the user in case of disconnects
            if (errno == ECONNRESET || errno == ETIMEDOUT) {
                d_logger->warn(
                    "Connection has been shut down, waiting for a new connection.");
                if (d_connection_mode == TCP_SOURCE_MODE_SERVER) {
                    close(d_exchange_socket);
                }

                d_connected = false;
            }
#endif
            break;                  // exit because there is not point in receiving
        } else if (received == 0) { // no data left to receive and peer shut down
            if (d_connection_mode == TCP_SOURCE_MODE_CLIENT) {
                d_logger->warn(
                    "Server has closed the connection and no more data remains.");
                return WORK_DONE;
            } else {
                d_logger->info("Client has closed the connection and no more data "
                               "remains. Waiting for new connection.");
#ifdef WINDOWS_SOCKETS
                closesocket(d_exchange_socket);
#else

                close(d_exchange_socket);
#endif
                d_connected = false;
                break;
            }
        } else {
            to_be_received -= received;
            out += received;
        }
    }

    return noutput_items;
}
} // namespace network
} // namespace gr
