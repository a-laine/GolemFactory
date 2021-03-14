#include "FileWatcher.h"

//  Default
FileWatcher::FileWatcher(const std::string& name) : filename(name), lastChangeHash(0)
{}
//

// Setter / getter
std::string FileWatcher::getFilemane() const
{
	return filename;
}
size_t FileWatcher::getLastChangeHash() const
{
	return lastChangeHash;
}
void FileWatcher::setFilename(const std::string& name)
{
	filename = name;
}
void FileWatcher::setLastChangeHash(const size_t& hash)
{
	lastChangeHash = hash;
}
//
