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


#ifndef UE0284644_73FE_42B6_BFC6_E1B6C079A0EF
#define UE0284644_73FE_42B6_BFC6_E1B6C079A0EF

#include <sstream>

namespace fu {

    /**
     * Available logger levels.
     */
    enum logger_level {
        ALL   = -1,
        TRACE = 0,
        DEBUG = 1,
        INFO  = 2,
        WARN  = 3,
        ERROR = 4,
        FATAL = 5,
        NONE  = 6
    };

    /**
     * Force color even when stderr is not a console?
     */
    enum force_color {
        DONT_FORCE_COLOR = 0,
        FORCE_COLOR = 1
    };


    class logger; // forward declaration


    /* ----------------------------------------------------------------------------------------- */
    /*                                      fu::logger_line                                      */
    /* ----------------------------------------------------------------------------------------- */

    /**
     * logger_line is created every time a message should be sent to log,
     * it then accumulates text inside an internal std::ostringstream,
     * and flushes the text (neatly formatted) to std::clog on destruciton.
     */
    class logger_line
    {
        logger_level        _level;
        const logger&       _parent_logger;
        std::ostringstream* _buffer;
        bool                _stacktrace;

        void initialize_buffer();

        typedef std::basic_ios<std::ostream::char_type, std::ostream::traits_type> __ios_type;

    public:
        /**
         * Initializes logger_line instance.
         *
         * @param   level           level of the message
         * @param   parent_logger   parent of this instance
         */
        __attribute__((always_inline, hot))
        inline logger_line(logger_level level, const logger& parent_logger)
            : _level(level),
                _parent_logger(parent_logger),
                _stacktrace(false)
        {
            initialize_buffer();
        }

        /**
         * Sends text accumulated in _buffer to std::clog.
         */
        ~logger_line();

        /**
         * Implements operator<< so that logger_line can be used as an ostream.
         */
        template <class T>
        __attribute__((hot))
        logger_line& operator<<(const T& object)
        {
            if (_buffer != nullptr) {
                *_buffer << object;
            }

            return *this;
        }

        // The three next functions implement specializations of operator<<
        // for use with manipulators (e.g. std::endl).
        logger_line& operator<<(std::ostream& (*function)(std::ostream&));
        logger_line& operator<<(__ios_type& (*function)(__ios_type&));
        logger_line& operator<<(std::ios_base& (*function)(std::ios_base&));

        /**
         * Implements operator<< for exceptions.
         */
        logger_line& operator<< (const std::exception& e);

        /**
        * IO manipulator for logger_line intended to print stacktraces.
        */
        struct stacktrace {
            bool enabled;
            __attribute__((always_inline)) inline stacktrace(bool val) : enabled(val) { }
        };

        /**
         * Implements operator<< for stacktrace manipulators.
         */
        __attribute__((always_inline))
        inline logger_line& operator<<(stacktrace flag) {
            _stacktrace = flag.enabled;
            return *this;
        }

    };


    /* ----------------------------------------------------------------------------------------- */
    /*                                         fu::logger                                        */
    /* ----------------------------------------------------------------------------------------- */

    /**
     * logger is meant to be used module-locally to create logger_line instances.
     */
    class logger {
        const char* _tag;

        static logger_level _level;
        thread_local static const char* _thread_name;

    public:
        /**
         * Creates a logger instance.
         *
         * @param   tag     tag of the instance
         */
        __attribute__((always_inline))
        inline logger(const char* tag)
            : _tag(tag)
        { }

        /**
         * Returns logger tag.
         *
         * @return  tag
         */
        __attribute__((always_inline, pure))
        inline const char* tag() const
        {
            return _tag;
        }

        /**
         * Creates a logger line associated with this instance.
         *
         * @param   level   level of the message logged
         *
         * @return  newly created logger line
         */
        __attribute__((always_inline))
        inline logger_line operator()(logger_level level) const
        {
            return logger_line(level, *this);
        }

        /**
         * Gets minimum message level to display.
         *
         * @return  minimum level
         */
        __attribute__((always_inline, pure))
        inline static logger_level level()
        {
            return _level;
        }

        /**
         * Sets minimum message level to display.
         *
         * @param   level   minimum level
         */
        __attribute__((always_inline))
        inline static void level(logger_level level)
        {
            _level = level;
        }

        /**
         * Gets the name of the current thread.
         *
         * @return  name of the thread
         */
        __attribute__((always_inline, pure))
        inline static const char* thread_name()
        {
            return _thread_name;
        }

        /**
         * Sets the name of the current thread.
         *
         * @param   name    name of the thread
         */
        __attribute__((always_inline))
        inline static void thread_name(const char* name)
        {
            _thread_name = name;
        };

        /**
         * Copy of logger_line::stacktrace for convenience.
         */
        typedef logger_line::stacktrace stacktrace;


        /**
         * Sets up logging color.
         */
        static void color(force_color force = DONT_FORCE_COLOR);
    };

};

#endif
