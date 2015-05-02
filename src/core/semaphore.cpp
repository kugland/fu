#include "semaphore.hpp"

using fu::semaphore;
using std::unique_lock;
using std::mutex;
using std::lock_guard;

semaphore::semaphore(int initial_count)
    : _count(initial_count)
{ }

void semaphore::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [=]{ return _count > 0; });
    _count--;
}

void semaphore::post()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _count++;
    _cv.notify_one();
}
