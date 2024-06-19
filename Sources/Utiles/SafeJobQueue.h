#pragma once

#include <atomic>

class SafeJobQueue
{
	public:
		typedef void(*JobCallback)(void*);
		struct Job
		{
			void* m_data;
			SafeJobQueue::JobCallback m_function;
			std::atomic<Job*> m_next;
		};

		SafeJobQueue();
		~SafeJobQueue();

		void AddJob(void* _data, JobCallback _callback);
		Job GetJob(void* _data, JobCallback _callback);

	protected:
		std::atomic<Job*> m_head;
		std::atomic<Job*> m_tail;
};