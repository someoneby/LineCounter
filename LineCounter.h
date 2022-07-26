#pragma once

#include <boost/filesystem/operations.hpp>
#include <future>
#include <map>
#include <mutex>
#include <vector>

struct LineCounter {
	/*
		Counts the lines in all files in all directories in a given path
		If path doesn't provided, lines counts in the current directory
		Returns false if wrong path provided
	*/
	bool getLinesNumber(const std::string& path, uint64_t& result);
private:
	/*
		Recursively fills m_filesToCount
	*/
	void fillFilesVector(boost::filesystem::recursive_directory_iterator&& path);

	/*
		Critical section! Reads lines from file
	*/
	void countLines(std::promise<uint64_t>&& promise);

	/*
		Thread-safe push to m_filesToCount
	*/
	void pushPathToCount(const std::string& path);

	/*
		Check if lines for this path are cashed
	*/
	bool checkIfDirectoryCashed(const std::string& path, uint64_t& result);

	static std::map<std::string, uint64_t> m_countedPath;		// cashe for counted directories
	bool m_fillFilesVectorFinished;								// flag that there is no new files
	std::vector<std::string> m_filesToCount;					// cashe of files should be counted
	std::mutex m_filesToCountMutex;								// mutex to make thread-safe m_filesToCount
};