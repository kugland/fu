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


#ifndef U68FB47BA_560F_4F14_9DD5_523062854D8A
#define U68FB47BA_560F_4F14_9DD5_523062854D8A

#include "../semaphore.hpp"
#include "buffer.hpp"

namespace fu {

    namespace audio {

        /**
         * Swaps audio::buffer instances between different threads,
         * used to construct audio pipes.
         */
        class connection {

            buffer*    _send_buf;
            semaphore  _send_semaphore;
            semaphore  _recv_semaphore;

        public:
            connection();

            /**
             * Sends data to a receiver thread, and recycles storage already used
             * by that thread.
             *
             * @param buf  audio::buffer containing data to be send, and which
             *             will receive a buffer to be recycled.
             */
            void send(buffer& buf);

            /**
             * Receives data from a sender thread, and sends back used storage
             * to be recycled.
             *
             * @param buf  an used audio::buffer which will receive fresh data,
             *             and whose internal storage be recycled.
             *
             * @return     true if success, false if connection closed.
             */
            bool recv(buffer& buf);

            /**
             * Closes a connection.
             */
            void close();

        };

    } // namespace audio

} // namespace fu

#endif
