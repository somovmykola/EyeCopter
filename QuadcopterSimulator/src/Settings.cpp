#include "Settings.h"
#include <algorithm>
#include <fstream>
#include <cstdlib>

using namespace std;

Settings::Settings() {
	LoadDefault();
}
	
int Settings::LoadDefault() {
	// Units are kg-m-s
	settingsMap["camera_fov"]="100";				// degrees
	settingsMap["camera_capture_period"]="1";		// s

	settingsMap["copter_sensor_angle"]="45";		// degrees
	settingsMap["copter_size"]="0.1";				// m
	settingsMap["copter_mass"] = "1";				// kg
	settingsMap["copter_h_speed"]="1";				// m/s
	settingsMap["copter_v_speed"]="0.5";			// m/s
	settingsMap["copter_pitch_limit"]="90";			// degrees
	settingsMap["copter_roll_limit"]="90";			// degrees
	
	settingsMap["copter_init_pos_x"] = "0.0";		// m
	settingsMap["copter_init_pos_y"] = "0.0";		// m
	settingsMap["copter_init_pos_z"] = "0.0";		// m
	settingsMap["copter_init_dir_x"] = "0.0";		// m
	settingsMap["copter_init_dir_y"] = "0.0";		// m
	settingsMap["copter_init_dir_z"] = "1.0";		// m


	settingsMap["image_h_res"]="640";				// pixels
	settingsMap["image_v_res"]="480";				// pixels
	settingsMap["image_path"]="./";					// string
	settingsMap["image_prefix"]="IMG";				// string
	settingsMap["image_filetype"]="png";			// bmp | tga | png
	settingsMap["image_background"]="floor";		// floor | room

	settingsMap["floor_type"]="solid";				// solid | checker

	settingsMap["noise_pitch"] = "0.0";				// degrees (std. dev)
	settingsMap["noise_yaw"] = "0.0";				// degrees (std. dev)
	settingsMap["noise_roll"] = "0.0";				// degrees (std. dev)
	settingsMap["noise_pixel"] = "0.0";				// n/a (std. dev)
	settingsMap["noise_sensor"] = "0.0";			// m (std. dev)

	settingsMap["model_file"] = "";					// string
	settingsMap["model_scale"] = "1.0";				// unitless
	
	settingsMap["sensor_range_min"] = "0";			// m
	settingsMap["sensor_range_max"] = "99999";		// m

	settingsMap["simulator_visual"] = "on";			// on | off
	settingsMap["simulator_interactive"] = "off";	// on | off
	settingsMap["simulator_end_time"] = "0";		// seconds
	settingsMap["simulator_timestep"] = "10";		// milliseconds, non-visual mode only

	return 0;
}

// Separate each line of settings file into two words, keyword and value.
// keywords and values may be separated only by spaces or tabs.
int Settings::LoadFromFile(string filename) {
	ifstream fin (filename.c_str());

	if (!fin.good()) {
		fprintf(stderr, "Unable to open settings file \"%s\"\n", filename.c_str());
		return 1;
	}

	string delims(" \t");

	for (int lineNum = 0; !fin.eof(); lineNum++) {
		string line;
		getline(fin, line);

		line = line.substr(0, line.find('#'));

		size_t keywordEnd = line.find_first_of(delims);
		if (keywordEnd == string::npos)		// If there are no non-whitespace characters
			continue;						// line is empty, so ignore and check next

		size_t valueStart = line.find_first_not_of(delims, keywordEnd);
		if (keywordEnd == string::npos) {
			fprintf(stderr, "Unable to find value on line %d of \"%s\"\n", lineNum, filename.c_str());
			continue;
		}

		
		string keyword = line.substr(0, keywordEnd);
		string value = line.substr(valueStart, string::npos);
	
		// Erase any trailing whitespace
		size_t valueEnd = value.find_last_not_of(delims)+1;
		value.erase(value.begin()+valueEnd, value.end());

		settingsMap[keyword] = value;
	}

	fin.close();

	return 0;
}

string Settings::GetString(string name) const {
	auto it = settingsMap.find(name);

	if (it == settingsMap.end())
		return "";

	return it -> second;
}

int Settings::GetInt(string name) const {
	auto it = settingsMap.find(name);

	if (it == settingsMap.end())
		return 0;

	return atoi((it -> second).c_str());
}

double Settings::GetDouble(string name) const {
	auto it = settingsMap.find(name);

	if (it == settingsMap.end())
		return 0.0;

	return atof((it -> second).c_str());
}
