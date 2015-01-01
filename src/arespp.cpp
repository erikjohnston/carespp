#include "carespp/carespp.hh"

using namespace carespp;


Error carespp::make_error(int s) {
    return std::error_code(s, arespp_error_category());
}

template<typename O>
static void init_channel(Channel& channel, O&& options, int optmask=0) {
    options.Get().sock_state_cb_data = &channel;
    options.Get().sock_state_cb = [] (void* data, int s, int read, int write) {
        Channel* c = reinterpret_cast<Channel*>(data);
        c->sock_state_cb(*c, s, read, write);
    };


    ::ares_init_options(&channel.Get(), &options.Get(), optmask | ARES_OPT_SOCK_STATE_CB);
}

Channel::Channel(SockStateCb const& cb) : sock_state_cb(cb) {
    init_channel(*this, Options());
}

Channel::Channel(Options& options, int optmask) : sock_state_cb(options.sock_state_cb) {
    init_channel(*this, options, optmask);
}


Channel::Channel(Options&& options, int optmask) : sock_state_cb(std::move(options.sock_state_cb)) {
    init_channel(*this, options, optmask);
}

Channel::~Channel() {
    destroy(*this);
}


class : public std::error_category {
public:
    virtual std::string message( int condition ) const {
        return ares_strerror(condition);
    }

    virtual const char* name() const noexcept {
        return "ares";
    }
} aress_error_category;

std::error_category const& carespp::arespp_error_category() {
    return aress_error_category;
}

Error carespp::library_init(int flags) {
    int s = ::ares_library_init(flags);
    return make_error(s);
}

void carespp::library_cleanup() {
    return ::ares_library_cleanup();
}

void carespp::destroy(Channel& channel) {
    return ::ares_destroy(channel.Get());
}

void carespp::cancel(Channel& channel) {
    return ::ares_cancel(channel.Get());
}

void carespp::send(Channel& channel, std::vector<unsigned char> const& data, QueryCb const& cb) {
    QueryCb* cb_ptr = new QueryCb(cb);
    return ::ares_send(
        channel.Get(),
        data.data(),
        static_cast<int>(data.size()),
        [] (void *arg, int status, int timeouts, unsigned char *abuf, int alen) {
            QueryCb& queryCb = *reinterpret_cast<QueryCb*>(arg);
            queryCb(
                make_error(status),
                timeouts,
                BufferView(abuf, static_cast<std::size_t>(alen))
            );

            delete &queryCb;
        },
        cb_ptr
    );
}

void carespp::query(Channel& channel, std::string const& name, int dnsclass, int type,
    QueryCb const& cb)
{
    QueryCb* cb_ptr = new QueryCb(cb);
    return ::ares_query(
        channel.Get(),
        name.c_str(),
        dnsclass,
        type,
        [] (void *arg, int status, int timeouts, unsigned char *abuf, int alen) {
            QueryCb& queryCb = *reinterpret_cast<QueryCb*>(arg);
            queryCb(
                make_error(status),
                timeouts,
                BufferView(abuf, static_cast<std::size_t>(alen))
            );

            delete &queryCb;
        },
        cb_ptr
    );
}
void carespp::search(Channel&, std::string const& name, int dnsclass, int type, QueryCb const&);

void carespp::process(Channel&, fd_set *read_fds, fd_set *write_fds);

void carespp::process_fd(Channel& channel, ares_socket_t read_fd, ares_socket_t write_fd) {
    return ::ares_process_fd(channel.Get(), read_fd, write_fd);
}

void carespp::set_socket_callback(Channel& channel, CreateSocketCb cb) {
    channel.create_socket_cb = cb;
    return ::ares_set_socket_callback(
        channel.Get(),
        [] (ares_socket_t socket_fd, int type, void* arg) -> int {
            Channel* c = reinterpret_cast<Channel*>(arg);
            Error error = c->create_socket_cb(socket_fd, type);

            int s = 0;
            if (error.category() == arespp_error_category()) {
                s = error.value();
            } else if (error) {
                s = -1;
            }

            return s;
        },
        &channel
    );
}

Error carespp::parse_a_reply(BufferView const& buffer, struct hostent **host,
    struct ares_addrttl *addrttls, int *naddrttls)
{
    int s = ::ares_parse_a_reply(
        buffer.data(), buffer.size(),
        host,
        addrttls,
        naddrttls
    );

    return make_error(s);
}

Error carespp::parse_srv_reply(BufferView const& buffer, ares_srv_reply** out) {
    int s = ::ares_parse_srv_reply(buffer.data(), buffer.size(), out);
    return make_error(s);
}

Error carespp::parse_srv_reply(BufferView const& buffer, std::vector<SrvReply>& out) {
    ares_srv_reply* reply;
    int s = ::ares_parse_srv_reply(buffer.data(), buffer.size(), &reply);

    if (!s) {
        ares_srv_reply* n = reply;
        for (; n; n = n->next) {
            out.push_back(SrvReply{
                n->priority,
                n->weight,
                n->port,
                std::string(n->host)
            });
        }

        ares_free_data(reply);
    }

    return make_error(s);
}

