#include "pch.h"

#include "Core/ThreadManager.h"
#include "../Resources/IResources.h"
using namespace Core;


ThreadManager::ThreadManager(const int& maxThread)
{
    m_maxThreads = maxThread;
    for(int i = 0; i < m_maxThreads; i++)
    {
        std::thread thread {&ThreadManager::Life, this};
        m_threadList.push_back(std::move(thread));
    }
}

ThreadManager::~ThreadManager()
{
    m_shouldStopThreads.store(true);
    for (std::thread& curThread : m_threadList)
        curThread.join();

    m_threadList.clear();
    std::queue<std::function<void()>> empty;
    std::swap(m_taskList, empty);
}

void ThreadManager::Life()
{
    static bool isAvailable = true;
    while (!m_shouldStopThreads)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (m_taskList.empty())
            continue;
        if(m_taskListMutex.try_lock())
        {
            if (!m_taskList.empty())
            {
                isAvailable = false;
                auto task = m_taskList.front();
                m_taskList.pop();
                m_taskListMutex.unlock();

                if (task != nullptr)
                    task();

                isAvailable = true;
            }
            else
            {
                m_taskListMutex.unlock();
            }
        }

    }
}

void ThreadManager::Lock()
{
#ifdef MULTITHREAD
    m_taskListMutex.lock();
#endif // MULTITHREAD
}

void ThreadManager::Unlock()
{
#ifdef MULTITHREAD
	m_taskListMutex.unlock();
#endif // MULTITHREAD
}
