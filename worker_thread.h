#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include "common_headers.h"

using namespace boost::asio;

namespace {
class workerThread
{
public:
    static void run(std::shared_ptr<io_context> context)
    {
        {
            std::lock_guard < std::mutex > lock(workerMutex);
            std::cout << "[" << std::this_thread::get_id() << "] Thread starts" << std::endl;
        }

        context->run();

        {
            std::lock_guard < std::mutex > lock(workerMutex);
            std::cout << "[" << std::this_thread::get_id() << "] Thread ends" << std::endl;
        }

    }
private:
    static std::mutex workerMutex;
};
std::mutex workerThread::workerMutex;
}

#endif