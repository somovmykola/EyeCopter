#ifndef QCS_NOISE_GENERATOR_H
#define QCS_NOISE_GENERATOR_H

#include "Settings.h"
#include <random>
#include <map>

// This class is an abstract interface to any variable that exhibits random noise
// Stores multiple distributions/parameters. Uses enums to get parameters from settings
// object

// This may be stupid as hell and its probably just best to have a class/object
// representing the noise generator for one (type of) variable. Though in that case
// its just a pleasant interface for C++11's random distributions

class Noise {
public:
	Noise(const Settings& s);

	// This may be better organized with a generic LastValue(...) NewValue(...)
	// that takes either a string or an enum as an argument

	enum noiseEnum {
		pitch,
		yaw,
		roll,
		pixel,
		sensor,
	};

	float GetVal(noiseEnum type);
private:
	typedef std::normal_distribution<float> distr_t;

	std::map<noiseEnum, bool> enable;
	std::map<noiseEnum, distr_t> distr;
	std::map<noiseEnum, float> defaultVal;
	
	std::default_random_engine generator;
};

#endif