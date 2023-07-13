#pragma once
#include "PandorAPI.h"

#include <thread>
#include <vector>
#include "mutex"
#include <any>
#include <functional>
#include <queue>

namespace Core
{
    class PANDOR_API ThreadManager
    {
    private:
        std::vector<std::thread> m_threadList;
        std::mutex m_taskListMutex;

        std::queue<std::function<void()>> m_taskList;   // Add any function of type void NameOfTheFunction() at the taskList

        int m_maxThreads;
        std::atomic_bool m_shouldStopThreads = false;
        
    private:
        void Life();
        
    public:
        //Delete all the copy constructors.
        ThreadManager(const ThreadManager&)				= delete;
        ThreadManager(ThreadManager&&)					= delete;
        ThreadManager& operator =(const ThreadManager&) = delete;
        ThreadManager& operator =(ThreadManager&&)		= delete;

        ThreadManager(const int& maxThread);
        ~ThreadManager();
        template <typename T>
        void Addtask(T* classObj , void (T::*task)())
        {
            struct Task
            {
                T* obj;
                void (T::* func)();

                void operator()() const
                {
                    (obj->*func)();
                }
            };
            Lock();
            m_taskList.emplace(Task{ classObj, task });
            Unlock();
        }

        void Addtask(void (*task)())
		{
			Lock();
			m_taskList.emplace(task);
			Unlock();
		}

		void Lock();
		void Unlock();
    };
}
