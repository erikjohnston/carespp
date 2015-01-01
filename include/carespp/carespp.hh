#pragma once

#include <system_error>

#include <ares.h>
#include <functional>
#include <vector>

#include <arpa/nameser.h>

#include "carespp/buffer_view.hh"


namespace carespp {
    using ::ares_socket_t;
    using ::fd_set;

    using DnsClass = ns_class;
    using DnsType = ns_type;

    using Error = std::error_code;

    Error make_error(int s);

    std::error_category const& arespp_error_category();

    template<typename T>
    class WrappedObject {
    public:
        T& Get() { return t; }
        T const& Get() const { return t; }

        operator bool() const { return t; }

    protected:
        template<typename... Args>
        WrappedObject(Args&&... args) : t(std::forward<Args>(args)...) {}

        void set(T& o) { t = o; }

    private:
        T t;
    };

    template<typename T>
    class WrappedResource : public WrappedObject<T*> {
    public:
        T const* operator->() const { return this->Get();  }
        T* operator->() { return this->Get(); }

    protected:
        template<typename... Args>
        WrappedResource(Args&&... args) : WrappedObject<T*>(std::forward<Args>(args)...) {}
    };

    class Channel;

    using SockStateCb = std::function<void(Channel&, int socket, bool read, bool write)>;
    // typedef void (*ares_callback)(void *arg, int status, int timeouts, unsigned char *abuf, int alen)
    using QueryCb = std::function<void(Error, int, BufferView const&)>;
    using CreateSocketCb = std::function<Error(ares_socket_t socket_fd, int type)>;

    struct Options : public WrappedObject < ::ares_options > {
        SockStateCb sock_state_cb;
    };

    class Channel : public WrappedObject< ::ares_channel > {
    public:
        Channel(SockStateCb const&);
        Channel(Options& options, int optmask);
        Channel(Options&& options, int optmask);

        ~Channel();

        SockStateCb sock_state_cb;
        CreateSocketCb create_socket_cb;
    };

    struct SrvReply {
        std::uint16_t priority;
        std::uint16_t weight;
        std::uint16_t port;
        std::string host;
    };

    Error library_init(int flags);
    void library_cleanup();
    void destroy(Channel&);
    void cancel(Channel&);
    void send(Channel&, std::vector<unsigned char> const&, QueryCb const&);
    void query(Channel&, std::string const& name, int dnsclass, int type, QueryCb const&);
    void search(Channel&, std::string const& name, int dnsclass, int type, QueryCb const&);

    void process(Channel&, fd_set *read_fds, fd_set *write_fds);
    void process_fd(Channel&, ares_socket_t read_fd, ares_socket_t write_fd);

    void set_socket_callback(Channel&, CreateSocketCb callback);

    Error parse_a_reply(BufferView const&, struct hostent **host, struct ares_addrttl *addrttls = nullptr,
        int *naddrttls = nullptr);

    Error parse_srv_reply(BufferView const&, ares_srv_reply**);
    Error parse_srv_reply(BufferView const&, std::vector<SrvReply>&);
}
