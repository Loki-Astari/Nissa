#ifndef THORSANVIL_NISSA_JOB_QUEUE_H
#define THORSANVIL_NISSA_JOB_QUEUE_H

#include "NissaConfig.h"
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nissa
{

using Work    = std::function<void()>;
class JobQueue
{
    using WorkQueue     = std::queue<Work>;

    std::vector<std::thread>        workers;
    std::mutex                      workMutex;
    std::condition_variable         workCV;
    WorkQueue                       workQueue;

    public:
        JobQueue(int workerCount);
        ~JobQueue();

        void addJob(Work&& action);

    private:
        Work     getNextJob();
        void     processWork();
};

}

#endif
