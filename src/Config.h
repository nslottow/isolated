#ifndef CONFIG_H
#define CONFIG_H

#include "Color.h"
#include <iosfwd>
#include <map>
#include <string>

/**
 * class ConfigSection
 */
class ConfigSection {
private:
	std::map<std::string, std::string> mProperties;

	const char* getProperty(const char* key);

public:
	int getInt(const char* key, int defaultValue = 0);
	bool getBool(const char* key, bool defaultValue = false);
	float getFloat(const char* key, float defaultValue = 0.f);
	double getDouble(const char* key, double defaultValue = 0.0);
	const char* getString(const char* key, const char* defaultValue = "");
	Color getColor(const char* key, Color defaultValue = { 1.f, 1.f, 1.f, 1.f });
	bool keyExists(const char* key);

	void setProperty(const char* key, const char* value);
};

/**
 * class Config
 * 
 * Global application configuration key-value storage.
 * Loads configuration files of the following format:
 *   [section-name]
 *   key value
 *   ...
 *   ; comments
 * Keys are assigned the last value they appeared with in the last file loaded.
 * This behavior can be used to load a default configuration and override
 * properties via a user configuration file.
 */
class Config {
private:
	std::map<std::string, ConfigSection> mSections;

public:
	ConfigSection& operator[](const char* sectionName) { return mSections[sectionName]; }

	void addFile(const char* path); // Loads values from a file using cerr to print errors
	void addFile(const char* path, std::ostream& errorStream);
};

extern Config gConfig;

// TODO: Maybe add support for registering config vars so that their defaults are automatically known if they're a static value

#endif
