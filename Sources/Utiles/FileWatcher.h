#pragma once

#include <string>

class FileWatcher
{
	public:
		//  Default
		FileWatcher(const std::string& name);
		//

		// Setter / getter
		std::string getFilemane() const;
		size_t getLastChangeHash() const;
		void setFilename(const std::string& name);
		void setLastChangeHash(const size_t& hash);
		//

	protected:
		// Attributes
		size_t lastChangeHash;
		std::string filename;
		//
};
