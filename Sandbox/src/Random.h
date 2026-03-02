#pragma once

#include <random>



class Random
{
public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());//这里std::random_device类重载了()，所以才能这么用，并且这个作用是产生随机数
	}

	static float Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}


private:
	inline static std::mt19937 s_RandomEngine;
	inline static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

