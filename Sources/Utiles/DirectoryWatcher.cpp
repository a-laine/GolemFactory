#include "DirectoryWatcher.h"
#include "System.h"
#include "ConsoleColor.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef GF_OS_WINDOWS
    #define stat _stat
#elif
    #include <unistd.h>
#endif



//  Default
DirectoryWatcher::DirectoryWatcher(const std::string& path) : directory(path)
{
    /*std::string error = "";
    lastDirectoryChange = getLastModificationDate(directory, &error);

    if (!error.empty())
    {
        std::cout << "DirectoryWatcher error : " << error << " (" << directory << ")" << std::endl;
    }*/
}

DirectoryWatcher::~DirectoryWatcher()
{
    for (auto it = fileList.begin(); it != fileList.end(); ++it)
        delete it->second;
    fileList.clear();
}
//

// Public functions
bool DirectoryWatcher::hasChanges()
{
    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
        std::string fullpath = directory + '/' + it->second->getFilemane();
        size_t hash = getLastModificationDate(fullpath);

        if (hash != it->second->getLastChangeHash())
        {
            changes.push_back(it->second->getFilemane());
            it->second->setLastChangeHash(hash);
        }
    }

    return !changes.empty();
}

std::vector<std::string> DirectoryWatcher::getAllChanges()
{
    std::vector<std::string> result;
    result.swap(changes);
    return result;
}
//

// Set / get
std::string DirectoryWatcher::getDirectory() const { return directory; }
bool DirectoryWatcher::isEmpty() const { return fileList.empty(); }
void DirectoryWatcher::clear() { fileList.clear(); }


void DirectoryWatcher::createNewFileWatcher(const std::string& filename)
{
    if (fileList.find(filename) != fileList.end())
        return;

    FileWatcher* watcher = new FileWatcher(filename);
    addFileWatcher(watcher);
}
void DirectoryWatcher::addFileWatcher(FileWatcher* watcher)
{
    if (!watcher)
        return;

    std::string error = "";
    std::string fullpath = directory + '/' + watcher->getFilemane();
    watcher->setLastChangeHash(getLastModificationDate(fullpath, &error));

    if (!error.empty())
    {
        std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::YELLOW) << "DirectoryWatcher warning : " << std::flush;
        std::cerr << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::flush;
        std::cerr << error << " (" << fullpath << ")" << std::endl;
    }

    fileList[watcher->getFilemane()] = watcher;
}
void DirectoryWatcher::removeFileWatcher(FileWatcher* watcher)
{
    if (!watcher)
        return;
    removeFileWatcher(watcher->getFilemane());
}
void DirectoryWatcher::removeFileWatcher(const std::string& filename)
{
    fileList.erase(filename);
}
//

// Protected functions
void DirectoryWatcher::errorHandler() const
{
#ifdef GF_OS_WINDOWS
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::RED) << "DirectoryWatcher : Error in HANDLER creation :" << std::flush;
    std::cout << ConsoleColor::getColorString(ConsoleColor::Color::CLASSIC) << std::endl;

    DWORD errorCode = ::GetLastError();
    if (errorCode != 0)
    {
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
        std::cout << messageBuffer << std::endl;
        LocalFree(messageBuffer);
    }

#elif GF_OS_MACOS

#elif GF_OS_LINUX

#endif 
}

size_t DirectoryWatcher::getLastModificationDate(const std::string& path, std::string* error) const
{
    struct stat data;
    if (stat(path.c_str(), &data) != 0)
    {
        if (error)
        {
            switch (errno)
            {
                case ENOENT:
                    *error = path + " not found" ;
                    break;
                case EINVAL:
                    *error = "Invalid parameter to _stat";
                    break;
                default:
                    *error = "Unexpected error in _stat";
            }
        }
    }
    else
    {
        char timebuf[26];
        if (ctime_s(timebuf, 26, &data.st_mtime))
        {
            if (error)
                *error = "Invalid arguments to ctime_s";
        }
        else
            return std::hash<std::string>{}(std::string(timebuf));
    }
    return 0;
}
//



