#pragma once

#include "swoole.h"
#include "connection.h"
#include "socks5.h"
#include <string>

namespace swoole
{
enum socket_lock_operation
{
    SOCKET_LOCK_READ = 1u << 1, SOCKET_LOCK_WRITE = 1U << 2,
};

class Socket
{
public:
    Socket(enum swSocket_type type);
    Socket(int _fd, Socket *sock);
    ~Socket();
    bool connect(std::string host, int port, int flags = 0);
    bool connect(const struct sockaddr *addr, socklen_t addrlen);
    bool shutdown(int how);
    bool close();
    ssize_t send(const void *__buf, size_t __n);
    ssize_t peek(void *__buf, size_t __n);
    ssize_t recv(void *__buf, size_t __n);
    ssize_t recv_all(void *__buf, size_t __n);
    ssize_t send_all(const void *__buf, size_t __n);
    ssize_t recv_packet();
    Socket* accept();
    void resume();
    void yield(int operation);
    bool bind(std::string address, int port = 0);
    std::string resolve(std::string host);
    bool listen(int backlog = 0);
    bool sendfile(char *filename, off_t offset, size_t length);
    ssize_t sendto(char *address, int port, char *data, int len);
    ssize_t recvfrom(void *__buf, size_t __n, char *address, int *port = nullptr);
    swString* get_buffer();

    void setTimeout(double timeout)
    {
        _timeout = timeout;
    }

#ifdef SW_USE_OPENSSL
    bool ssl_handshake();
    int ssl_verify(bool allow_self_signed);
#endif

protected:
    inline void init()
    {
        _cid = 0;
        read_locked = false;
        write_locked = false;
        _timeout = 0;
        _port = 0;
        errCode = 0;
        errMsg = nullptr;
        timer = nullptr;
        bind_port = 0;
        _backlog = 0;

        http2 = 0;
        shutdow_rw = 0;
        shutdown_read = 0;
        shutdown_write = 0;
        open_length_check = 0;
        open_eof_check = 0;

        socks5_proxy = nullptr;
        http_proxy = nullptr;

        buffer = nullptr;
        protocol = {0};

        protocol.package_length_type = 'N';
        protocol.package_length_size = 4;
        protocol.package_body_offset = 0;
        protocol.package_max_length = SW_BUFFER_INPUT_SIZE;

#ifdef SW_USE_OPENSSL
        open_ssl = 0;
        ssl_wait_handshake = 0;
        ssl_context = NULL;
        ssl_option = {0};
#endif
    }

    inline bool wait_events(int events)
    {
        if (socket->events == 0)
        {
            if (reactor->add(reactor, socket->fd, SW_FD_CORO_SOCKET | events) < 0)
            {
                errCode = errno;
                return false;
            }
        }
        else
        {
            if (reactor->set(reactor, socket->fd, SW_FD_CORO_SOCKET | socket->events | events) < 0)
            {
                errCode = errno;
                return false;
            }
        }
        return true;
    }

    bool socks5_handshake();
    bool http_proxy_handshake();

public:
    swTimer_node *timer;
    swReactor *reactor;
    std::string _host;
    std::string bind_address;
    int bind_port;
    int _port;
    int _cid;
    bool read_locked;
    bool write_locked;
    swConnection *socket;
    enum swSocket_type type;
    int _sock_type;
    int _sock_domain;
    double _timeout;
    int _backlog;
    int errCode;
    const char *errMsg;
    uint32_t http2 :1;
    uint32_t shutdow_rw :1;
    uint32_t shutdown_read :1;
    uint32_t shutdown_write :1;
    /**
     * one package: length check
     */
    uint32_t open_length_check :1;
    uint32_t open_eof_check :1;

    swProtocol protocol;
    swString *buffer;

    struct _swSocks5 *socks5_proxy;
    struct _http_proxy* http_proxy;

#ifdef SW_USE_OPENSSL
    uint8_t open_ssl :1;
    uint8_t ssl_wait_handshake :1;
    SSL_CTX *ssl_context;
    swSSL_option ssl_option;
#endif
};

};
