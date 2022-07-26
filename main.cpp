#include "LineCounter.h"
#include <iostream>

int main(int argc, char* argv[])
{
	const std::string path(argc > 1 ? argv[1] : "./");

    LineCounter lineCounter;
    uint64_t linesNumber{ 0 };
    if (lineCounter.getLinesNumber(path, linesNumber))
        std::cout << "In " << path << ": " << linesNumber << " lines\n";
    else
        std::cout << "Wrong path\n";
}