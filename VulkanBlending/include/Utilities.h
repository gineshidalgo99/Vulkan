#ifndef UTILITIES_H
#define UTILITIES_H

#include <chrono>

class Utilities
{
	public:
		static double sleepGraphicsThreadIfRequired(const double microsecondsPerFrame,
												    std::chrono::high_resolution_clock::time_point & clock);
};

#endif // UTILITIES_H
