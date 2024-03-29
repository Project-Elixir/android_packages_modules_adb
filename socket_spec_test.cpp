/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "socket_spec.h"

#include <string>

#include <unistd.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <gtest/gtest.h>

// If the socket spec is incorrectly specified (i.e w/o a "tcp:" prefix),
// check for the contents of the returned error string.
TEST(socket_spec, parse_tcp_socket_spec_failure_error_check) {
    std::string hostname, error, serial;
    int port;

    // spec needs to be prefixed with "tcp:"
    const std::string spec("sneakernet:5037");
    EXPECT_FALSE(parse_tcp_socket_spec(spec, &hostname, &port, &serial, &error));
    EXPECT_TRUE(error.find("sneakernet") != std::string::npos);
    EXPECT_EQ(error, "specification is not tcp: " + spec);
}

TEST(socket_spec, parse_tcp_socket_spec_just_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:5037", &hostname, &port, &serial, &error));
    EXPECT_EQ("", hostname);
    EXPECT_EQ(5037, port);
    EXPECT_EQ("", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_bad_ports_failure) {
    std::string hostname, error, serial;
    int port;
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:-1", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:65536", &hostname, &port, &serial, &error));
}

TEST(socket_spec, parse_tcp_socket_spec_host_and_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:localhost:1234", &hostname, &port, &serial, &error));
    EXPECT_EQ("localhost", hostname);
    EXPECT_EQ(1234, port);
    EXPECT_EQ("localhost:1234", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_host_no_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:localhost", &hostname, &port, &serial, &error));
    EXPECT_EQ("localhost", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("localhost:5555", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_host_ipv4_no_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:127.0.0.1", &hostname, &port, &serial, &error));
    EXPECT_EQ("127.0.0.1", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("127.0.0.1:5555", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_host_bad_ports_failure) {
    std::string hostname, error, serial;
    int port;
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:localhost:", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:localhost:-1", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:localhost:65536", &hostname, &port, &serial, &error));
}

TEST(socket_spec, parse_tcp_socket_spec_host_ipv4_bad_ports_failure) {
    std::string hostname, error, serial;
    int port;
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:127.0.0.1:", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:127.0.0.1:-1", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:127.0.0.1:65536", &hostname, &port, &serial, &error));
}

TEST(socket_spec, parse_tcp_socket_spec_host_ipv6_bad_ports_failure) {
    std::string hostname, error, serial;
    int port;
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:c63:50c9:8a91:8efa:", &hostname,
                                       &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:c63:50c9:8a91:8efa:-1", &hostname,
                                       &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:c63:50c9:8a91:8efa:65536", &hostname,
                                       &port, &serial, &error));
}

TEST(socket_spec, parse_tcp_socket_spec_ipv6_and_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:[::1]:1234", &hostname, &port, &serial, &error));
    EXPECT_EQ("::1", hostname);
    EXPECT_EQ(1234, port);
    EXPECT_EQ("[::1]:1234", serial);

    // Repeat with different format of ipv6
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:[2601:644:8e80:620::fbbc]:2345", &hostname, &port,
                                      &serial, &error));
    EXPECT_EQ("2601:644:8e80:620::fbbc", hostname);
    EXPECT_EQ(2345, port);
    EXPECT_EQ("[2601:644:8e80:620::fbbc]:2345", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_ipv6_no_port_success) {
    std::string hostname, error, serial;
    int port;
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:::1", &hostname, &port, &serial, &error));
    EXPECT_EQ("::1", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("[::1]:5555", serial);

    // Repeat with other supported formats of ipv6.
    EXPECT_TRUE(parse_tcp_socket_spec("tcp:2601:644:8e80:620::fbbc", &hostname, &port, &serial,
                                      &error));
    EXPECT_EQ("2601:644:8e80:620::fbbc", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("[2601:644:8e80:620::fbbc]:5555", serial);

    EXPECT_TRUE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:c63:50c9:8a91:8efa", &hostname, &port,
                                      &serial, &error));
    EXPECT_EQ("2601:644:8e80:620:c63:50c9:8a91:8efa", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("[2601:644:8e80:620:c63:50c9:8a91:8efa]:5555", serial);

    EXPECT_TRUE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:2d0e:b944:5288:97df", &hostname, &port,
                                      &serial, &error));
    EXPECT_EQ("2601:644:8e80:620:2d0e:b944:5288:97df", hostname);
    EXPECT_EQ(5555, port);
    EXPECT_EQ("[2601:644:8e80:620:2d0e:b944:5288:97df]:5555", serial);
}

TEST(socket_spec, parse_tcp_socket_spec_ipv6_bad_ports_failure) {
    std::string hostname, error, serial;
    int port;
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:[::1]", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:[::1]:", &hostname, &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:[::1]:-1", &hostname, &port, &serial, &error));

    EXPECT_TRUE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:2d0e:b944:5288:97df", &hostname, &port,
                                      &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:2d0e:b944:5288:97df:", &hostname,
                                       &port, &serial, &error));
    EXPECT_FALSE(parse_tcp_socket_spec("tcp:2601:644:8e80:620:2d0e:b944:5288:97df:-1", &hostname,
                                       &port, &serial, &error));
}

TEST(socket_spec, get_host_socket_spec_port_success) {
    std::string error;
    EXPECT_EQ(5555, get_host_socket_spec_port("tcp:5555", &error));
    EXPECT_EQ(5555, get_host_socket_spec_port("tcp:localhost:5555", &error));
    EXPECT_EQ(5555, get_host_socket_spec_port("tcp:[::1]:5555", &error));
}

TEST(socket_spec, get_host_socket_spec_port_vsock_success) {
    std::string error;
#ifdef __linux__  // vsock is only supported on linux
    EXPECT_EQ(5555, get_host_socket_spec_port("vsock:5555", &error));
#else
    GTEST_SKIP() << "vsock is only supported on linux";
#endif
}

TEST(socket_spec, get_host_socket_spec_port_no_port) {
    std::string error;
    EXPECT_EQ(5555, get_host_socket_spec_port("tcp:localhost", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("vsock:localhost", &error));
}

TEST(socket_spec, get_host_socket_spec_port_bad_ports) {
    std::string error;
    EXPECT_EQ(-1, get_host_socket_spec_port("tcp:65536", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("tcp:-5", &error));

    // The following two expectations happen to fail on non-linux anyway(for
    // different reasons than "vsock is only supported on linux").
    EXPECT_EQ(-1, get_host_socket_spec_port("vsock:-5", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("vsock:5:5555", &error));
}

TEST(socket_spec, get_host_socket_spec_port_bad_string) {
    std::string error;
    EXPECT_EQ(-1, get_host_socket_spec_port("tcpz:5555", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("vsockz:5555", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("abcd:5555", &error));
    EXPECT_EQ(-1, get_host_socket_spec_port("abcd", &error));
}

TEST(socket_spec, socket_spec_listen_connect_tcp) {
    std::string error, serial;
    int port;
    unique_fd server_fd, client_fd;
    EXPECT_FALSE(socket_spec_connect(&client_fd, "tcp:localhost:7777", &port, &serial, &error));
    server_fd.reset(socket_spec_listen("tcp:7777", &error, &port));
    EXPECT_NE(server_fd.get(), -1);
    EXPECT_TRUE(socket_spec_connect(&client_fd, "tcp:localhost:7777", &port, &serial, &error));
    EXPECT_NE(client_fd.get(), -1);
}

TEST(socket_spec, socket_spec_connect_failure) {
    std::string error, serial;
    int port;
    unique_fd client_fd;
    EXPECT_FALSE(socket_spec_connect(&client_fd, "tcp:", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "acceptfd:", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "vsock:", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "vsock:x", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "vsock:5", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "vsock:5:x", &port, &serial, &error));
    EXPECT_FALSE(socket_spec_connect(&client_fd, "sneakernet:", &port, &serial, &error));
}

TEST(socket_spec, socket_spec_listen_connect_localfilesystem) {
    std::string error, serial;
    int port;
    unique_fd server_fd, client_fd;
    TemporaryDir sock_dir;

    // Only run this test if the created directory is writable.
    int result = access(sock_dir.path, W_OK);
    if (result == 0) {
        std::string sock_addr =
                android::base::StringPrintf("localfilesystem:%s/af_unix_socket", sock_dir.path);
        EXPECT_FALSE(socket_spec_connect(&client_fd, sock_addr, &port, &serial, &error));
        server_fd.reset(socket_spec_listen(sock_addr, &error, &port));

        EXPECT_NE(server_fd.get(), -1);
        EXPECT_TRUE(socket_spec_connect(&client_fd, sock_addr, &port, &serial, &error));
        EXPECT_NE(client_fd.get(), -1);
    }
}

TEST(socket_spec, is_socket_spec) {
    EXPECT_TRUE(is_socket_spec("tcp:blah"));
    EXPECT_TRUE(is_socket_spec("acceptfd:blah"));
    EXPECT_TRUE(is_socket_spec("local:blah"));
    EXPECT_TRUE(is_socket_spec("localreserved:blah"));
    EXPECT_TRUE(is_socket_spec("vsock:123:456"));
}

TEST(socket_spec, is_local_socket_spec) {
    EXPECT_TRUE(is_local_socket_spec("local:blah"));
    EXPECT_TRUE(is_local_socket_spec("tcp:localhost"));
    EXPECT_FALSE(is_local_socket_spec("tcp:www.google.com"));
}
