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


#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <boost/algorithm/string.hpp>

#include <syscall.h>
#include <cxxabi.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "logger.hpp"


using std::clog;
using std::endl;
using std::exception;
using std::ios_base;
using std::lock_guard;
using std::mutex;
using std::ostream;
using std::ostringstream;
using std::string;
using std::size_t;
using std::list;
using fu::logger;
using fu::logger_level;
using fu::logger_line;


/* --------------------------------------------------------------------------------------------- */
/*                                           fu::logger                                          */
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*                            Defaults for fu::logger static variables                           */
/* --------------------------------------------------------------------------------------------- */

logger_level             fu::logger::_level       = fu::ERROR;
thread_local const char* fu::logger::_thread_name = nullptr;


/* --------------------------------------------------------------------------------------------- */
/*                                        fu::logger_color                                       */
/* --------------------------------------------------------------------------------------------- */

enum class attr {
    reset = 0, bright = 1
};

enum class fg {
    black = 30, red = 31, green = 32, yellow = 33, blue = 34,
    magenta = 35, cyan = 36, white = 37, reset = 39
};

enum class bg {
    black = 40, red = 41, green = 42, yellow = 43, blue = 44,
    magenta = 45, cyan = 46, white = 47, reset = 49
};

static bool __use_color = false;

__attribute__((hot))
static ostream& __print_code(ostream& os, int val)
{
    if (__builtin_expect(__use_color, 0)) {
        os << "\033[" << val << "m";
    }

    return os;
}

__attribute__((cold))
void fu::logger::color(force_color force)
{
    __use_color = (force == FORCE_COLOR) || isatty(STDERR_FILENO);
}

__attribute__((always_inline, hot))
inline static ostream& operator<< (ostream& os, fg color)
{
    return __print_code(os, static_cast<int>(color));
}

__attribute__((always_inline, hot))
inline static ostream& operator<< (ostream& os, bg color)
{
    return __print_code(os, static_cast<int>(color));
}

__attribute__((always_inline, hot))
inline static ostream& operator<< (ostream& os, attr attr)
{
    return __print_code(os, static_cast<int>(attr));
}


/* --------------------------------------------------------------------------------------------- */
/*                                        fu::logger_line                                        */
/* --------------------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------------------- */
/*                                     Configurable constants                                    */
/* --------------------------------------------------------------------------------------------- */

#define TIME_WIDTH    13
#define THREAD_WIDTH  12
#define TAG_WIDTH     12


/* --------------------------------------------------------------------------------------------- */
/*                                  Module-local static objects                                  */
/* --------------------------------------------------------------------------------------------- */

static mutex __logger_mutex;


/* --------------------------------------------------------------------------------------------- */
/*                                 Module-local static functions                                 */
/* --------------------------------------------------------------------------------------------- */

__attribute__((cold))
static string cxx_demangle(const string& name)
{
    using std::free;
    using std::unique_ptr;
    using abi::__cxa_demangle;

    struct cfree {
        void operator()(void* p) { free(p); }
    };

    thread_local static unique_ptr<char, cfree> demangle_ptr  = nullptr;
    thread_local static size_t                  demangle_size = 0;

    int   status;
    char* temp_ptr;

    temp_ptr = __cxa_demangle(name.c_str(), demangle_ptr.get(), &demangle_size, &status);

    if (temp_ptr != nullptr) {
        demangle_ptr.release();
        demangle_ptr.reset(temp_ptr);
    }

    return string(status == 0 ? demangle_ptr.get() : name);
}

__attribute__((always_inline, cold))
inline static void generate_stacktrace(list<string>& list, void* first_return_address)
{
    using std::sprintf;

    char   name[256];
    string func;

    unw_cursor_t   cursor;
    unw_context_t  uc;
    unw_word_t     ip, offp;

    list.emplace_back("---");

    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);

    bool add_line = false;
    while (unw_step(&cursor) > 0)
    {
        name[0] = '\0';

        unw_get_proc_name(&cursor, name, 256, &offp);

        func = cxx_demangle(name);

        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        //unw_get_reg(&cursor, UNW_REG_SP, &sp);

        if (ip == reinterpret_cast<unw_word_t>(first_return_address))
            add_line = true;

        if (func.size() == 0 && ip == 0)
            break;

        if (!add_line)
            continue;

        char ip_str[32];

        static_assert(sizeof(ip) == 4 || sizeof(ip) == 8, "sizeof(ip) is neither 4 nor 8");
        if (sizeof(ip) == 4) {
            sprintf(ip_str, "[%08" PRIiPTR "]", ip);
        } else if (sizeof(ip) == 8) {
            sprintf(ip_str, "[%016" PRIiPTR "]", ip);
        }
        list.emplace_back(string(ip_str) + func);
    }

    list.emplace_back("---");
}

__attribute__((always_inline, cold))
inline static void __print_line(logger_level level, const ostringstream& oss,
                                const logger& parent_logger, bool stacktrace,
                                void* first_return_address)
{
    using std::fixed;
    using std::list;
    using std::left;
    using std::setprecision;
    using std::setw;
    using std::to_string;
    using std::logic_error;
    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    using boost::is_any_of;
    using boost::split;
    using boost::trim;

    // Get thread name or thread id if none, and limit it to THREAD_WIDTH chars.
    string thread_name(logger::thread_name() != nullptr ? logger::thread_name() : to_string(syscall(SYS_gettid)));

    if (thread_name.size() > THREAD_WIDTH) {
        thread_name.resize(THREAD_WIDTH);
        thread_name[THREAD_WIDTH - 1] = '.';
        thread_name[THREAD_WIDTH - 2] = '.';
        thread_name[THREAD_WIDTH - 3] = '.';
    }

    // Get tag name, and limit it to TAG_WIDTH chars.
    string tag(parent_logger.tag());

    if (tag.size() > TAG_WIDTH) {
        tag.resize(TAG_WIDTH);
        tag[THREAD_WIDTH - 1] = '.';
        tag[THREAD_WIDTH - 2] = '.';
        tag[THREAD_WIDTH - 3] = '.';
    }

    // Selects the apropriate character and color for a given log level;
    fg level_fg;
    bg level_bg;
    const char* level_ch;

    switch (level) {
        case fu::TRACE: level_fg = fg::black;  level_bg = bg::white;  level_ch = " T "; break;
        case fu::DEBUG: level_fg = fg::green;  level_bg = bg::green;  level_ch = " D "; break;
        case fu::INFO:  level_fg = fg::cyan;   level_bg = bg::cyan;   level_ch = " I "; break;
        case fu::WARN:  level_fg = fg::yellow; level_bg = bg::yellow; level_ch = " W "; break;
        case fu::ERROR: level_fg = fg::red;    level_bg = bg::red;    level_ch = " E "; break;
        case fu::FATAL: level_fg = fg::red;    level_bg = bg::white;  level_ch = " ! "; break;
        case fu::ALL:   throw logic_error("logger_line cannot have level fu::ALL");
        case fu::NONE:  throw logic_error("logger_line cannot have level fu::NONE");
    }

    // Split message lines.
    string message = oss.str();
    trim(message);
    if (message.size() == 0 && !stacktrace)
        message = "<empty>";
    list<string> lines;
    split(lines, message, is_any_of("\n"));

    // Create filler to align subsequent lines.
    const string align_filler(1+TIME_WIDTH+1 + 2 + THREAD_WIDTH + 2 + TAG_WIDTH + 2 + 3 + 2, ' ');

    // Generates stack trace;
    list<string> trace;
    if (stacktrace) {
        generate_stacktrace(trace, first_return_address);
    }

    {
        lock_guard<mutex> lock(__logger_mutex);

        // Calculate µs since epoch.
        // This is done inside the lock to ensure that time always increases steadly.
        const auto time_since_epoch = steady_clock::now().time_since_epoch();
        const double us_since_epoch = (double) (duration_cast<microseconds>(time_since_epoch).count()) / 1000000.0;

        // Show µs since epoch.
        clog
            << attr::reset << '['
            << attr::bright << fg::green
            << fixed << setw(TIME_WIDTH) << setprecision(6) << us_since_epoch
            << attr::reset << ']';

        clog << "  ";

        // Show thread name.
        clog
            << attr::bright << fg::cyan
            << left << setw(THREAD_WIDTH) << thread_name
            << attr::reset;

        clog << "  ";

        // Show tag name.
        clog
            << attr::bright << fg::white
            << left << setw(TAG_WIDTH) << tag
            << attr::reset;

        clog << "  ";

        // Show level tag.
        clog << attr::bright << level_fg << level_bg
               << level_ch
               << attr::reset;

        clog << "  ";

        bool first_line = true;

        // Show message lines.
        for (auto line: lines) {
            if (!first_line) {
                clog << align_filler;
            }
            clog << line << endl;
            first_line = false;
        }

        // Show stack trace.
        for (auto line: trace) {
            if (!first_line) {
                clog << align_filler;
            }
            clog << line << endl;
            first_line = false;
        }
    }
}


/* --------------------------------------------------------------------------------------------- */
/*                                    Private member functions                                   */
/* --------------------------------------------------------------------------------------------- */

/**
 * Initializes _buffer to a new std::ostringstream
 * if line is visible, to nullptr otherwise.
 */
__attribute__((hot))
void logger_line::initialize_buffer()
{
    _buffer = __builtin_expect(_parent_logger.level() <= _level, 0)
             ? new ostringstream
             : nullptr;
}


/* --------------------------------------------------------------------------------------------- */
/*                                           Destructor                                          */
/* --------------------------------------------------------------------------------------------- */

__attribute__((hot))
logger_line::~logger_line()
{
    if (__builtin_expect(_buffer != nullptr, 0)) {
        __print_line(_level, *_buffer, _parent_logger, _stacktrace, __builtin_return_address(0));
        delete _buffer;
    }
}


/* --------------------------------------------------------------------------------------------- */
/*                          operator<< specializations for manipulators                          */
/* --------------------------------------------------------------------------------------------- */

__attribute__((hot))
logger_line& logger_line::operator<<(ostream& (*function)(ostream&))
{
    if (__builtin_expect(_buffer != nullptr, 0)) {
        function(*_buffer);
    }

    return *this;
}

__attribute__((hot))
logger_line& logger_line::operator<<(logger_line::__ios_type& (*function)(logger_line::__ios_type&))
{
    if (__builtin_expect(_buffer != nullptr, 0)) {
        function(*_buffer);
    }

    return *this;
}

__attribute__((hot))
logger_line& logger_line::operator<<(ios_base& (*function)(ios_base&))
{
    if (__builtin_expect(_buffer != nullptr, 0)) {
        function(*_buffer);
    }

    return *this;
}


/* --------------------------------------------------------------------------------------------- */
/*                            operator<< specialization for exceptions                           */
/* --------------------------------------------------------------------------------------------- */

__attribute__((hot))
logger_line& logger_line::operator<< (const exception& e)
{
    if (__builtin_expect(_buffer != nullptr, 0)) {
        string name = cxx_demangle(typeid(e).name()),
               what = cxx_demangle(e.what());
        *_buffer << "Exception " << name;
        if (name != what) {
            *_buffer << ": " << what;
        }
        *_buffer << endl;
    }
    return *this;
}
