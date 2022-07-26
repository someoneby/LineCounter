#include <fstream>
#include <iostream>
#include "LineCounter.h"

bool LineCounter::getLinesNumber(const std::string& path, uint64_t& result) {
	if (checkIfDirectoryCashed(path, result))
		return true;

	std::vector<std::future<uint64_t>> resultsFromThreads;
	m_fillFilesVectorFinished = false;

	// Creates (MAX - 1) threads for listening files to counting
	// If there is no cores information use 2
	uint16_t optimalThreads{ std::thread::hardware_concurrency() != 0 ? std::thread::hardware_concurrency() : 2 };
	for (uint16_t i{ 0 }; i < optimalThreads - 1; ++i) {
		std::promise<uint64_t> promise;
		resultsFromThreads.push_back(promise.get_future());
		std::thread(&LineCounter::countLines, this, std::move(promise)).detach();
	}

	// Create directory iterator and check if it's correct
	boost::system::error_code err;
	boost::filesystem::recursive_directory_iterator directoryIt(path, err);
	if (!err)
		fillFilesVector(std::move(directoryIt));
	else
		return false;

	// Help other threads to count saved files.
	std::promise<uint64_t> promise;
	resultsFromThreads.push_back(promise.get_future());
	countLines(std::move(promise));

	uint64_t temp{ 0 };
	for(auto& i : resultsFromThreads)
		temp += i.get();

	m_countedPath.emplace(path, temp);

	result = temp;
	return true;
}

void LineCounter::fillFilesVector(boost::filesystem::recursive_directory_iterator&& begin) {
	boost::filesystem::recursive_directory_iterator end;
	for (; begin != end; ++begin)
		pushPathToCount(begin->path().string());

	m_fillFilesVectorFinished = true;
}

void LineCounter::countLines(std::promise<uint64_t>&& promise) {
	std::string path;
	std::string buff;
	uint64_t tempLines{ 0 };
	std::unique_lock<std::mutex> ul(m_filesToCountMutex, std::defer_lock);

	while (true)
	{
		ul.lock();
		if (!m_filesToCount.empty())
		{
			// Critical section
			// take new path from cashe
			path = m_filesToCount.back();
			m_filesToCount.pop_back();
			ul.unlock();

			// Count lines
			std::ifstream file(path);
			while (getline(file, buff))
				tempLines++;

			file.close();
		}
		else {
			ul.unlock();

			// Break if all files was saved and counted
			if (m_fillFilesVectorFinished)
				break;
		}
	}

	promise.set_value(tempLines);
}

void LineCounter::pushPathToCount(const std::string& path) {
	std::lock_guard<std::mutex> lg(m_filesToCountMutex);
	m_filesToCount.push_back(path);
}

bool LineCounter::checkIfDirectoryCashed(const std::string& path, uint64_t& result) {
	if (!LineCounter::m_countedPath.empty())
	{
		auto checkIfPathWasCounted = LineCounter::m_countedPath.find(path);
		if (checkIfPathWasCounted != LineCounter::m_countedPath.end())
		{
			result = checkIfPathWasCounted->second;
			return true;
		}
	}

	return false;
}

std::map<std::string, uint64_t> LineCounter::m_countedPath{};