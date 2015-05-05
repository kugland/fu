// Copyright (c) 2015 André von Kugland

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#include "buffer.hpp"
#include "connection.hpp"

using fu::audio::connection;
using fu::audio::buffer;

connection::connection()
    : _send_buf(nullptr)
{ }

void connection::close()
{
    _send_buf = nullptr;
    _recv_semaphore.post();
}

void connection::send(buffer& buf)
{
    _send_buf = &buf;
    _recv_semaphore.post();
    _send_semaphore.wait();
}

bool connection::recv(buffer& buf)
{
    _recv_semaphore.wait();
    if (__builtin_expect(_send_buf != nullptr, 1)) {
        std::swap(*_send_buf, buf);
        _send_semaphore.post();
        return true;
    } else {
        // connection::close was called by the sender.
        return false;
    }
}
