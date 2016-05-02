#include "Noise.h"
#include <ctime>

using namespace std;

Noise::Noise(const Settings& s) {
	generator.seed(time(0));

	// Load standard deviations
	const float pitchSD = s.GetDouble("noise_pitch");
	const float yawSD = s.GetDouble("noise_yaw");
	const float rollSD = s.GetDouble("noise_roll");
	const float pixelSD = s.GetDouble("noise_pixel");
	const float sensorSD = s.GetDouble("noise_sensor");

	// If each random variable has a non-zero standard deviation, initialize its
	// corresponding distribution
	enable[pitch] = (pitchSD != 0.f);
	enable[yaw] = (yawSD != 0.f);
	enable[roll] = (rollSD != 0.f);
	enable[pixel] = false;//(pixelSD != 0.f);
	enable[sensor] = (sensorSD != 0.f);

	if (enable[pitch])	distr[pitch] = distr_t(0.f, pitchSD);
	if (enable[yaw])	distr[yaw] = distr_t(0.f, yawSD);
	if (enable[roll])	distr[roll] = distr_t(0.f, rollSD);
	if (enable[pixel])	distr[pixel] = distr_t(0.f, pixelSD);
	if (enable[sensor])	distr[sensor] = distr_t(0.f, sensorSD);
	
	// Initialize current values
	defaultVal[pitch] = 0.f;
	defaultVal[yaw] = 0.f;
	defaultVal[roll] = 0.f;
	defaultVal[sensor] = 0.f;
}

float Noise::GetVal(noiseEnum type) {
	if (!enable[type])
		return defaultVal.at(type);

	return distr[type](generator);
}
