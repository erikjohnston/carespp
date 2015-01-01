#pragma once


#include <cstdlib>
#include <ostream>
#include <string>
#include <vector>

namespace carespp {
    class BufferView {
    public:
        using Type = unsigned char;

        using iterator = Type *;
        using const_iterator = Type const *;

        BufferView(Type const *, std::size_t len);

        BufferView(std::vector<Type> const &c) : data_(c.data()), length_(c.size()) {
        }
        //BufferView(std::string const& c) : data_(c.data()), length_(c.size()) {}

        BufferView(BufferView const &) = delete;

        BufferView(BufferView &&) = delete;

        Type const *data() const;

        std::size_t length() const;

        std::size_t size() const;

//    iterator begin();
//    iterator end();

        const_iterator begin() const;

        const_iterator end() const;

        const_iterator cbegin() const;

        const_iterator cend() const;

    private:
        Type const *const data_;
        std::size_t length_;
    };
}


std::ostream& operator<<(std::ostream& os, carespp::BufferView const& buf);
