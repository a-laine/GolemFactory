#include "WorkerThread.h"

#include <thread>
#include <string>
#include <list>
#include <Utiles/ProfilerConfig.h>
#include <iostream>
#include <chrono>

class JobQueues
{
	public:
		std::mutex m_mutex;
		std::list<Job*> m_regularJobQueues[Job::JobPriority::LONG];
		std::list<Job*> m_longJobQueues;
		std::condition_variable m_wakeup;

		Job* grabRegularJob(uint32_t& instanceID);
		Job* grabLongJob(uint32_t& instanceID);
		void removeJob(Job* job);
};
JobQueues g_jobQueues;


Job::Job(uint32_t instanceCount, JobCallback callback, void* data) : m_data(data), m_callback(callback), m_instanceCount(instanceCount), 
	m_isInQueue(false), m_priority(JobPriority::COUNT)
{
	m_instanceGrabedCount = 0;
	m_instanceFinishedCount = 0;
}
Job::Job() : m_data(nullptr), m_callback(nullptr), m_instanceCount(0),
	m_isInQueue(false), m_priority(JobPriority::COUNT)
{
	m_instanceGrabedCount = 0;
	m_instanceFinishedCount = 0;
}
void Job::waitCompletion(bool grabInstance)
{
	/*bool shouldWait = true;
	while (0 && grabInstance)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		if (instanceGrabedCount >= m_instanceCount)
			break;

		uint32_t grabedId = instanceGrabedCount;
		instanceGrabedCount++;
		lock.unlock();

		(*m_callback)(m_data, grabedId, m_instanceCount);

		lock.lock();
		instanceFinishedCount++;
		if (instanceFinishedCount == m_instanceCount)
		{
			std::unique_lock<std::mutex> lock2(g_jobQueues.m_mutex);
			lock.unlock();
			auto& joblist = g_jobQueues.m_regularJobQueues[m_priority];
			for (auto it = joblist.begin(); it != joblist.end(); it++)
			{
				if (*it == this)
				{
					joblist.erase(it);
					//std::unique_lock<std::mutex> joblock(job->m_mutex);
					//job->m_isInQueue = false;
					break;
				}
			}

			lock.lock();
			m_isInQueue = false;
			shouldWait = false;
			lock.unlock();

			//m_isInQueue = false;
			//lock.unlock();
			//g_jobQueues.removeJob(this);
			break;
		}
	}*/

	// wait
	std::unique_lock<std::mutex> lock(m_mutex);
	//if (!shouldWait) return;

	SCOPED_CPU_MARKER("Job::wait");
	while (m_instanceFinishedCount != m_instanceCount)
	{
		auto now = std::chrono::system_clock::now();
		g_jobQueues.m_wakeup.wait_until(lock, now + std::chrono::microseconds(200));
	}
	//m_wakeup.wait(lock);
}
void Job::addToQueue(JobPriority prio, bool wakeupThreads)
{
	if (prio == JobPriority::COUNT || !m_callback || m_instanceCount == 0)
	{
		std::cout << "Invalid job !" << std::endl;
		return;
	}

	std::unique_lock<std::mutex> lock(g_jobQueues.m_mutex);
	if (prio == JobPriority::LONG)
	{
		g_jobQueues.m_longJobQueues.emplace_back(this);
		m_isInQueue = true;
		m_priority = prio;

		if (wakeupThreads)
		{
			if (m_instanceCount == 1)
				g_jobQueues.m_wakeup.notify_one();
			else
				g_jobQueues.m_wakeup.notify_all();
		}
	}
	else if (prio < JobPriority::LONG)
	{
		int index = (int)prio;
		g_jobQueues.m_regularJobQueues[index].emplace_back(this);
		m_isInQueue = true;
		m_priority = prio;

		if (wakeupThreads)
		{
			if (m_instanceCount == 1)
				g_jobQueues.m_wakeup.notify_one();
			else
				g_jobQueues.m_wakeup.notify_all();
		}
	}
}

Job* JobQueues::grabRegularJob(uint32_t& instanceID)
{
	std::unique_lock<std::mutex> lock(g_jobQueues.m_mutex);
	for (int i = 0; i < (int)Job::JobPriority::LONG; i++)
	{
		auto& joblist = m_regularJobQueues[i];
		if (!joblist.empty())
		{
			for (auto it : joblist)
			{
				std::unique_lock<std::mutex> joblock(it->m_mutex);
				if (it->m_instanceGrabedCount >= it->m_instanceCount)
					continue;

				instanceID = it->m_instanceGrabedCount;
				it->m_instanceGrabedCount++;
				return it;
			}
		}
	}

	auto now = std::chrono::system_clock::now();
	g_jobQueues.m_wakeup.wait_until(lock, now + std::chrono::microseconds(200));
	return nullptr;
}
Job* JobQueues::grabLongJob(uint32_t& instanceID)
{
	std::unique_lock<std::mutex> lock(g_jobQueues.m_mutex);
	if (!m_longJobQueues.empty())
	{
		for (auto it : m_longJobQueues)
		{
			std::unique_lock<std::mutex> joblock(it->m_mutex);
			if (it->m_instanceGrabedCount >= it->m_instanceCount)
				continue;

			instanceID = it->m_instanceGrabedCount;
			it->m_instanceGrabedCount++;
			return it;
		}
	}
	return nullptr;
}
void JobQueues::removeJob(Job* job)
{
	std::unique_lock<std::mutex> lock(g_jobQueues.m_mutex);
	auto& joblist = g_jobQueues.m_regularJobQueues[job->m_priority];
	for (auto it = joblist.begin(); it != joblist.end(); it++)
	{
		if (*it == job)
		{
			joblist.erase(it);
			std::unique_lock<std::mutex> joblock(job->m_mutex);
			job->m_isInQueue = false;
			break;
		}
	}
}

std::atomic_bool WorkerThread::g_run;
std::vector<WorkerThread*> WorkerThread::g_allWorkers = std::vector<WorkerThread*>();
WorkerThread::WorkerThread(int id, bool isLongWorker) : m_threadId(id), m_longWorker(isLongWorker)
{
	m_thread = isLongWorker ? std::thread(WorkerThread::updateLong, this) : std::thread(WorkerThread::update, this);
}
void WorkerThread::initialize(int workerCount, int longWorkerCount)
{
	g_run = true;
	for (int i = 0; i < workerCount; i++)
		g_allWorkers.push_back(new WorkerThread(i, false));
	for (int i = 0; i < longWorkerCount; i++)
		g_allWorkers.push_back(new WorkerThread(i, true));
}
void WorkerThread::update(WorkerThread* worker)
{
	std::string name = "WorkerThread " + std::to_string(worker->m_threadId);
	THREAD_MARKER(name.c_str());

	uint32_t instanceID;
	while (g_run)
	{
		Job* job = g_jobQueues.grabRegularJob(instanceID);
		if (job)
		{
			(*job->m_callback)(job->m_data, instanceID, job->m_instanceCount);

			std::unique_lock<std::mutex> lock(job->m_mutex);
			job->m_instanceFinishedCount++;
			if (job->m_instanceFinishedCount == job->m_instanceCount)
			{
				lock.unlock();
				g_jobQueues.removeJob(job);
				job->m_wakeup.notify_all();
			}
		}
	}
}
void WorkerThread::updateLong(WorkerThread* worker)
{
	std::string name = "LongWorkerThread " + std::to_string(worker->m_threadId);
	THREAD_MARKER(name.c_str());

	while (g_run)
	{
		uint32_t instanceID;
		Job* job = g_jobQueues.grabLongJob(instanceID);
		if (job)
		{
			(*job->m_callback)(job->m_data, instanceID, job->m_instanceCount);

			std::unique_lock<std::mutex> lock(job->m_mutex);
			job->m_instanceFinishedCount++;
			if (job->m_instanceFinishedCount == job->m_instanceCount)
			{
				lock.unlock();
				g_jobQueues.removeJob(job);
				job->m_wakeup.notify_all();
			}
		}
		else
		{
			std::unique_lock<std::mutex> lock(g_jobQueues.m_mutex);
			g_jobQueues.m_wakeup.wait(lock);
		}
	}
}