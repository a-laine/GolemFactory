#pragma once

#include <functional>

#include "Singleton.h"
#include "ThreadSafeRingBuffer.h"
#include "Mutex.h"
#include "BankArray.h"

class Job2
{
	friend class JobSystem;
	public:
		using Task = std::function<void(int,void*)>;
		enum JobPriority
		{
			VERY_HIGH = 0,
			HIGH,
			MEDIUM,
			LOW,

			LONG,
			COUNT
		};
		class ReaderCounter
		{
			public:
				ReaderCounter(std::atomic_uint32_t& c) : m_counter(&c) { m_counter->fetch_add(1); };
				~ReaderCounter() { m_counter->fetch_sub(1); };

			protected:
				std::atomic_uint32_t* m_counter;
		};

		Job2(JobPriority priority, Task task, uint32_t dependancyCount = 0);

		void waitCompletion(bool allowGrabInstances);
		bool pollCompletion(int* optionalRemainingInstances = nullptr);
		bool grabInstanceGroup(int& instanceStart, int& instanceCount);

		std::atomic_uint32_t* getDependancyCounter();

	private:
		void* m_data = nullptr;
		Task m_task;

		Mutex m_lock;
		JobPriority m_jobPriority = JobPriority::LOW;	// job priority and pool index
		int m_instanceCount = 1;						// number of time we need to execute m_task
		int m_instanceGroup = 1;						// number of instance grabed per worker
		volatile int m_instanceGrabedCount = 0;			// number of grabed instance
		volatile int m_instanceFinishedCount = 0;		// number of finished instances (atomic because poll should not lock whole Job)
		std::atomic_uint32_t m_dependancyCounter;		// how many job before me need to finished befaore start
		std::atomic_uint32_t* m_dependancy = nullptr;	// dependancyCounter pointer of an other job depending on me
		volatile int m_jobIndex = -1;					// job index in the pool

		std::atomic_uint32_t m_readingThreadCount;

		std::mutex m_finishedLock;
		std::condition_variable m_finishedSignal;
};

class JobSystem : public Singleton<JobSystem>
{
	friend class Singleton<JobSystem>;
	friend class Job2;

	public:
		JobSystem();
		~JobSystem();
		void init(bool printInfos);

		void pushJob(Job2& job, void* data = nullptr, std::atomic_uint32_t* dependancy = nullptr);
		void dispatchJob(Job2& job, int count, int groupSize, void* data = nullptr, std::atomic_uint32_t* dependancy = nullptr);

		/*Job2* pushJob(Job2::JobPriority priority, Job2::Task task, void* data,
			uint32_t dependancyCount = 0, std::atomic_uint32_t* dependancy = nullptr);
		Job2* dispatchJob(Job2::JobPriority priority, Job2::Task task, void* data, int count, int groupSize, 
			uint32_t dependancyCount = 0, std::atomic_uint32_t* dependancy = nullptr);*/

		void notifyWorkers(Job2::JobPriority type = Job2::JobPriority::COUNT, bool all = true);

	private:
		bool grabJobInstance(bool onlyLongJob, Job2** job, int& instanceStart, int& instanceCount);
		void removeJob(Job2* job);

		std::mutex m_lock; // has to be STD one due to conditions
		//ThreadSafeRingBuffer<Job2*, 256> m_jobPools[Job2::JobPriority::COUNT];

		BankArray<Job2*> m_jobPools[Job2::JobPriority::COUNT];
		std::condition_variable m_wakeupWorker;
		std::condition_variable m_wakeupLongWorker;
		int m_workerCount = 0;
		int m_longWorkerCount = 0;
		std::atomic_bool m_keepWorkersRunning;

		//std::vector<std::thread*> m_workers;
};