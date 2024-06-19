#include "SafeJobQueue.h"


SafeJobQueue::SafeJobQueue()
{
	m_head = nullptr;
	m_tail = nullptr;
}
SafeJobQueue::~SafeJobQueue()
{

}

void SafeJobQueue::AddJob(void* _data, JobCallback _callback)
{
    std::atomic<Job*> const newJob = new Job();
    Job* jobptr = newJob.load();
    jobptr->m_data = _data;
    jobptr->m_function = _callback;

    Job* oldTail = m_tail.load();
    //while (!oldTail->m_next.compare_exchange_weak(nullptr, jobptr))
    {
        oldTail = m_tail.load();
    }
    m_tail.compare_exchange_weak(oldTail, jobptr);
}
SafeJobQueue::Job SafeJobQueue::GetJob(void* _data, JobCallback _callback)
{

}
