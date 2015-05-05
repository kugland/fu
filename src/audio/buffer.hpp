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


#ifndef U24B04923_30DA_417A_8C0C_5AE69CD57755
#define U24B04923_30DA_417A_8C0C_5AE69CD57755

#include <utility>
#include <stdexcept>

namespace fu {

    namespace audio {

        class buffer
        {

            /* --------------------------------------------------------------------------------- */
            /*                                Internal properties                                */
            /* --------------------------------------------------------------------------------- */

            float*   _data;
            unsigned _capacity;
            unsigned _frames;
            unsigned _channels;
            unsigned _sample_rate;
            bool     _finished;

        public:

            /* --------------------------------------------------------------------------------- */
            /*                            Constructors and destructor                            */
            /* --------------------------------------------------------------------------------- */

            /**
             * Constructs a zero-length buffer object.
             */
            __attribute__((always_inline))
            inline buffer() noexcept
                : _data(nullptr),
                  _capacity(0),
                  _frames(0),
                  _channels(0),
                  _sample_rate(0),
                  _finished(false)
            { }

            /**
             * Constructs an buffer object, sets its properties and initializes its buffer.
             */
            __attribute__((always_inline))
            inline buffer(unsigned frames, unsigned channels, unsigned sample_rate)
                : _data(nullptr), _finished(false)
            {
                reset(frames, channels, sample_rate);
            }

            /**
             * Copy constructor.
             */
            buffer(const buffer& other);

            /**
             * Move constructor.
             */
            buffer(buffer&& other) noexcept;

            /**
             * Destructor.
             */
            virtual ~buffer();


            /* --------------------------------------------------------------------------------- */
            /*                                Getters and setters                                */
            /* --------------------------------------------------------------------------------- */

            /**
             * Access to the internal buffer.
             */
            __attribute__((always_inline))
            inline float* data()
            {
                return _data;
            }

            /**
             * Const access to the internal buffer.
             */
            __attribute__((always_inline))
            inline const float* cdata() const
            {
                return _data;
            }

            /**
             * Returns the number of frames in the buffer.
             */
            __attribute__((always_inline))
            inline unsigned frames() const
            {
                return _frames;
            }

            /**
             * Returns the number of channels per frame.
             */
            __attribute__((always_inline))
            inline unsigned channels() const {
                return _channels;
            }

            /**
             * Returns the sample rate (Hz) of the buffer.
             */
            __attribute__((always_inline))
            inline unsigned sample_rate() const
            {
                return _sample_rate;
            }

            /**
             * Sets the sample rate (Hz) of the buffer.
             */
            __attribute__((always_inline))
            inline void sample_rate(unsigned value)
            {
                _sample_rate = value;
            }

            /**
             * Returns true if this is the last buffer in a stream, false otherwise.
             */
            __attribute__((always_inline))
            inline bool finished() const
            {
                return _finished;
            }

            /* --------------------------------------------------------------------------------- */
            /*                               Misc member functions                               */
            /* --------------------------------------------------------------------------------- */

            /**
             * Resets buffer properties, reallocating only when necessary.
             *
             * @param frames        number of frames
             * @param channels      number of channels
             * @param sample_rate   sample rate (Hz)
             */
            void reset(unsigned frames, unsigned channels, unsigned sample_rate);

            /**
             * Truncates the buffer.
             *
             * @param frames   new number of frames
             */
            void trunc(unsigned frames);

            /**
             * Swaps two buffers, used to specialize std::swap.
             */
            void swap(buffer& other);

            /**
             * Releases its internal buffer.
             */
            float* release();

            /**
             * Tells the receiver this is the last buffer in a stream.
             */
            __attribute__((always_inline))
            inline void finish()
            {
                _finished = true;
            }
        };

    } // namespace audio

} // namespace fu

namespace std {

    /**
     * Specialization of std::swap for fu::buffer.
     */
    template<>
    __attribute__((always_inline))
    inline void swap(fu::audio::buffer& a, fu::audio::buffer& b)
    {
        a.swap(b);
    }

} // namespace std

#endif
