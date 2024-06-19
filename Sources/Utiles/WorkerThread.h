#pragma once

#include <queue>
#include <atomic>
#include <condition_variable>

#include <Utiles/Mutex.h>


class Job
{
	friend class WorkerThread;
	friend class JobQueues;
	public:
		enum JobPriority
		{
			VERY_HIGH = 0,
			HIGH,
			MEDIUM,
			LOW,

			LONG,
			COUNT
		};
		typedef void(*JobCallback)(void* /*data*/, int /*instanceID*/, int /*instanceCount*/);

		Job();
		Job(uint32_t instanceCount, JobCallback callback, void* data);
		void waitCompletion(bool grabInstance);
		void addToQueue(JobPriority prio, bool wakeupThreads = true);

	protected:
		std::mutex m_mutex;
		void* m_data;
		Job::JobCallback m_callback;
		uint32_t m_instanceCount;
		uint32_t m_instanceGrabedCount;
		uint32_t m_instanceFinishedCount;
		std::condition_variable m_wakeup;
		bool m_isInQueue;
		JobPriority m_priority;
};

class WorkerThread
{
	public:
		WorkerThread(int id, bool isLongWorker);
		static void initialize(int workerCount, int longWorkerCount);
		static void update(WorkerThread* worker);
		static void updateLong(WorkerThread* worker);
		static std::atomic_bool g_run;

	protected:
		std::thread m_thread;
		int m_threadId;
		bool m_longWorker;
		static std::vector<WorkerThread*> g_allWorkers;
};

