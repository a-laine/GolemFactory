#include "JobSystem.h"
#include "ProfilerConfig.h"
#include <glm/detail/func_common.hpp>
#include <iostream>
#include <chrono>




Job2::Job2(JobPriority priority, Task task, uint32_t dependancyCount) : m_jobPriority(priority), m_task(task),
	m_dependancyCounter(dependancyCount), m_readingThreadCount(0)
{

}
void Job2::waitCompletion(bool allowGrabInstances)
{
	// wait for dependancy to finish
	JobSystem* system = JobSystem::getInstance();
	system->notifyWorkers(m_jobPriority);
	if (m_dependancyCounter.load() != 0)
	{
		SCOPED_CPU_MARKER("wait dependancy");
		while (m_dependancyCounter.load() != 0)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
	}

	if (allowGrabInstances)
	{
		// grab and update job counters
		bool grabing = true;
		while (grabing)
		{
			int start, count;
			grabing = grabInstanceGroup(start, count);
			if (grabing)
			{
				Job2::ReaderCounter readerguard(m_readingThreadCount);
				for (int i = 0; i < count; i++)
					m_task(start + i, m_data);

				bool finished = false;

				// do job conters update
				{
					MutexGuard guard(m_lock);
					m_instanceFinishedCount += count;
					finished = (m_instanceFinishedCount == m_instanceCount);
					if (finished && m_dependancy)
						(*m_dependancy)--;
				}

				// job finished
				if (finished)
				{
					// lock has to be free when entering in remove job
					system->removeJob(this);
					grabing = false;
				}
			}
		}
	}

	if (!pollCompletion())
	{
		SCOPED_CPU_MARKER("wait finished");

		constexpr int wakeupFrameCount = 20;
		int wakeuptimer = wakeupFrameCount;
		system->notifyWorkers(m_jobPriority);
		while (!pollCompletion())
		{
			wakeuptimer--;
			if (wakeuptimer == 0)
			{
				wakeuptimer = wakeupFrameCount;
				system->notifyWorkers(m_jobPriority);
			}

			std::unique_lock<std::mutex> lock(m_finishedLock);
			auto now = std::chrono::system_clock::now();
			m_finishedSignal.wait_until(lock, now + std::chrono::microseconds(10));
		}
	}
}
bool Job2::pollCompletion(int* optionalRemainingInstances)
{
	MutexGuard guard(m_lock);
	int remaining = m_instanceCount - m_instanceFinishedCount;
	bool finished = (remaining == 0 && m_jobIndex >= -1 && m_readingThreadCount.load() == 0);
	if (optionalRemainingInstances)
		*optionalRemainingInstances = remaining;
	return finished;
}
bool Job2::grabInstanceGroup(int& instanceStart, int& instanceCount)
{
	ReaderCounter readerguard(m_readingThreadCount);
	MutexGuard guard(m_lock);
	if (m_dependancyCounter.load() != 0)
		return false;

	int remainCount = m_instanceCount - m_instanceGrabedCount;
	if (remainCount != 0)
	{
		instanceStart = m_instanceGrabedCount;
		instanceCount = std::min(m_instanceGroup, remainCount);
		m_instanceGrabedCount += instanceCount;
		return true;
	}
	return false;
}
std::atomic_uint32_t* Job2::getDependancyCounter()
{
	return &m_dependancyCounter;
}






JobSystem::JobSystem()
{
	unsigned int coreCount = std::thread::hardware_concurrency();
	m_workerCount = glm::clamp(coreCount, 1u, 10u);
	m_longWorkerCount = glm::clamp(coreCount, 1u, 4u);
	m_keepWorkersRunning = true;
	for (int i = 0; i < m_workerCount + m_longWorkerCount; i++)
	{
		bool isLongWorker = i >= m_workerCount;
		std::thread worker([this, isLongWorker]
			{
				THREAD_MARKER(isLongWorker ? "LongWorker" : "Worker");
				Job2* job;
				int start, count;

				while (m_keepWorkersRunning.load())
				{
					if (grabJobInstance(isLongWorker, &job, start, count))
					{
						Job2::ReaderCounter readerguard(job->m_readingThreadCount);

						SCOPED_CPU_MARKER("Working");
						for (int i = 0; i < count; i++)
							job->m_task(start + i, job->m_data);

						bool finished = false;

						// do job conters update
						{
							MutexGuard guard(job->m_lock);
							job->m_instanceFinishedCount += count;
							finished = (job->m_instanceFinishedCount == job->m_instanceCount);
							if (finished && job->m_dependancy)
								(*job->m_dependancy)--;
						}

						// job finished
						if (finished)
						{
							// lock has to be free when entering in remove job
							removeJob(job);
						}
					}
					else
					{
						SCOPED_CPU_MARKER("sleeping");
						std::unique_lock<std::mutex> lock(m_lock);
						auto now = std::chrono::system_clock::now();
						if (isLongWorker)
							m_wakeupLongWorker.wait_until(lock, now + std::chrono::microseconds(200));
						else
							m_wakeupWorker.wait_until(lock, now + std::chrono::microseconds(400));
					}
				}
			});

		//m_workers.push_back(worker);
		worker.detach();
	}
}
JobSystem::~JobSystem()
{
	m_keepWorkersRunning = false;
}
void JobSystem::init(bool printInfos)
{
	if (printInfos)
	{
		unsigned int coreCount = std::thread::hardware_concurrency();
		std::cout << "JobSystem : " << std::endl;
		std::cout << "   core count : " << coreCount << std::endl;
		std::cout << "   worker count : " << m_workerCount << std::endl;
		std::cout << "   Long worker count : " << m_longWorkerCount << std::endl;
	}
}


void JobSystem::pushJob(Job2& job, void* data, std::atomic_uint32_t* dependancy)
{
	dispatchJob(job, 1, 1, data, dependancy);
}
void JobSystem::dispatchJob(Job2& job, int count, int groupSize, void* data, std::atomic_uint32_t* dependancy)
{
	job.m_data = data;
	job.m_instanceGroup = groupSize;
	job.m_instanceCount = count;
	job.m_dependancy = dependancy;

	std::unique_lock<std::mutex> lock(m_lock);
	job.m_jobIndex = m_jobPools[(int)job.m_jobPriority].add(&job);
	notifyWorkers(job.m_jobPriority, count > groupSize);
}

bool JobSystem::grabJobInstance(bool onlyLongJob, Job2** job, int& instanceStart, int& instanceCount)
{
	int poolStart = onlyLongJob ? (int)Job2::JobPriority::LONG : (int)Job2::JobPriority::VERY_HIGH;
	int poolStop = onlyLongJob ? (int)Job2::JobPriority::COUNT : (int)Job2::JobPriority::LONG;

	std::unique_lock<std::mutex> lock(m_lock);
	for (int i = poolStart; i < poolStop; i++)
	{
		for (uint32_t j = 0; j < m_jobPools[i].range(); j++)
		{
			if (!m_jobPools[i].isValid(j))
				continue;
			Job2* jobCandidate = m_jobPools[i][j];
			if (!jobCandidate)
				continue;

			if (jobCandidate->grabInstanceGroup(instanceStart, instanceCount))
			{
				*job = jobCandidate;
				return true;
			}
		}
	}
	return false;
}
void JobSystem::removeJob(Job2* job)
{
	Job2::ReaderCounter readerguard(job->m_readingThreadCount);
	std::unique_lock<std::mutex> lock(m_lock);
	MutexGuard guard(job->m_lock);
	m_jobPools[(int)job->m_jobPriority].remove(job->m_jobIndex);
	job->m_jobIndex = -1;
	job->m_finishedSignal.notify_all();
}

void JobSystem::notifyWorkers(Job2::JobPriority type, bool all)
{
	bool notifyWorkers = type != Job2::JobPriority::LONG;
	bool notifyLongWorkers = type >= Job2::JobPriority::LONG;
	if (all)
	{
		if (notifyWorkers)
			m_wakeupWorker.notify_all();
		if (notifyLongWorkers)
			m_wakeupLongWorker.notify_all();
	}
	else
	{
		if (notifyWorkers)
			m_wakeupWorker.notify_one();
		if (notifyLongWorkers)
			m_wakeupLongWorker.notify_one();
	}
}