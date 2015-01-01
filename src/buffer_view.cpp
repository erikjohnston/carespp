#include "carespp/buffer_view.hh"

using namespace carespp;

BufferView::BufferView(typename BufferView::Type const* data, std::size_t len) : data_(data), length_(len) {}

BufferView::Type const* BufferView::data() const { return data_; }
std::size_t BufferView::length() const { return length_; }
std::size_t BufferView::size() const { return length_; }

BufferView::const_iterator BufferView::begin() const { return data_; }
BufferView::const_iterator BufferView::end() const { return data_ + length_; }

BufferView::const_iterator BufferView::cbegin() const  { return data_; }
BufferView::const_iterator BufferView::cend() const { return data_ + length_; }


std::ostream& operator<<(std::ostream& os, BufferView const& buf) {
    for (auto& c : buf) os << c;
    return os;
}
