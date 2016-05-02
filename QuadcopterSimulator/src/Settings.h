#ifndef QCS_SETTINGS_H
#define QCS_SETTINGS_H

#include <string>
#include <map>

class Settings {
public:
	Settings();
	
	int LoadDefault();
	int LoadFromFile(std::string filename);

	std::string GetString(std::string name) const;
	int GetInt(std::string name) const;
	double GetDouble(std::string name) const;
private:
	std::map<std::string, std::string> settingsMap;
};

#endif