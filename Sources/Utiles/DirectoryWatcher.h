#pragma once

//#include "System.h"
#include "FileWatcher.h"
#include "Mutex.h"

#include <vector>
#include <map>
#include <thread>
#include <atomic>


#define MAX_FILE_NOTIFY_INFORMATION 1024
#define MAX_FILE_PATH_LENGTH 1024


class DirectoryWatcher
{
	public:
		//  Default
		DirectoryWatcher(const std::string& path);
		~DirectoryWatcher();
		//

		// Public functions
		bool hasChanges();
		std::vector<std::string> getAllChanges();
		//

		// Set / get / add / remove
		std::string getDirectory() const;
		bool isEmpty() const;
		void clear();

		void createNewFileWatcher(const std::string& filename);
		void addFileWatcher(FileWatcher* watcher);
		void removeFileWatcher(FileWatcher* watcher);
		void removeFileWatcher(const std::string& filename);
		//

	protected:
		// Protected functions
		void errorHandler() const;
		size_t getLastModificationDate(const std::string& path, std::string* error = nullptr) const;
		//

		// Attributes
		//size_t lastDirectoryChange;
		std::string directory;
		std::map<std::string, FileWatcher*> fileList;
		std::vector<std::string> changes;

		/*std::thread* thread;
		std::atomic_bool runThread;
		Mutex mutex;
		std::vector<std::string> touchedFiles;*/
		//
};