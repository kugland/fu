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


#include <stdexcept>
#include <utility>

#include "buffer.hpp"

using fu::audio::buffer;


/* --------------------------------------------------------------------------------------------- */
/*                                  Constructors and destructor                                  */
/* --------------------------------------------------------------------------------------------- */

buffer::buffer(const buffer& other)
    : _capacity(other._frames * other._channels),
      _frames(other._frames),
      _channels(other._channels),
      _sample_rate(other._sample_rate)
{
    _data = new float[_capacity];
    __builtin_memcpy(_data, other._data, _capacity * sizeof(float));
}

buffer::buffer(buffer&& other) noexcept
    : _data(other._data),
      _capacity(other._capacity),
      _frames(other._frames),
      _channels(other._channels),
      _sample_rate(other._sample_rate)
{
    other._data        = nullptr;
    other._frames      = 0;
    other._channels    = 0;
    other._sample_rate = 0;
}

buffer::~buffer()
{
    delete _data;
}


/* --------------------------------------------------------------------------------------------- */
/*                                     Misc. member functions                                    */
/* --------------------------------------------------------------------------------------------- */

void buffer::reset(unsigned frames, unsigned channels, unsigned sample_rate)
{
    if (_data == nullptr || (frames * channels) > _capacity) {
        delete _data;
        _capacity = frames * channels;
        _data = new float[_capacity];
    }
    _frames      = frames;
    _channels    = channels;
    _sample_rate = sample_rate;
}

void buffer::trunc(unsigned frames)
{
    if (frames <= _frames)
        _frames = frames;
    else
        throw std::length_error("audio::buffer::trunc");
}

void buffer::swap(buffer& other)
{
    using std::swap;

    swap(_data, other._data);
    swap(_capacity, other._capacity);
    swap(_frames, other._frames);
    swap(_channels, other._channels);
    swap(_sample_rate, other._sample_rate);
}

float* buffer::release()
{
    float* data = _data;

    _data = nullptr;
    _capacity = 0;
    _frames = 0;
    _channels = 0;
    _sample_rate = 0;

    return data;
}
