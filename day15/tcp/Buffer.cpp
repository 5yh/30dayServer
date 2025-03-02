#include "Buffer.h"

const std::string &Buffer::buf() const { return buf_; }
const char *Buffer::c_str() const { return buf_.c_str(); }
void Buffer::set_buf(const char *buf)
{
    std::string new_buf(buf);
    // 更快
    buf_.swap(new_buf);
}

size_t Buffer::Size() const { return buf_.size(); }

void Buffer::Append(const char *str, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (str[i] == '\0')
        {
            break;
        }
        buf_.push_back(str[i]);
    }
}

void Buffer::Clear() { buf_.clear(); }