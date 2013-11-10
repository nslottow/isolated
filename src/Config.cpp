#include "Config.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

using namespace std;

Config gConfig;

// TODO: these could all be specializations of a get<T>()
const char* ConfigSection::getProperty(const char* key) {
	auto it = mProperties.find(key);
	return it == mProperties.end() ? nullptr : it->second.c_str();
}

int ConfigSection::getInt(const char* key, int defaultValue) {
	auto valueStr = getProperty(key);
	return valueStr ? atoi(valueStr) : defaultValue;
}

bool ConfigSection::getBool(const char* key, bool defaultValue) {
	return getInt(key, defaultValue ? 1 : 0) != 0;
}

float ConfigSection::getFloat(const char* key, float defaultValue) {
	auto valueStr = getProperty(key);
	return valueStr ? (float)atof(valueStr) : defaultValue;
}

double ConfigSection::getDouble(const char* key, double defaultValue) {
	auto valueStr = getProperty(key);
	return valueStr ? atof(valueStr) : defaultValue;
}

const char* ConfigSection::getString(const char* key, const char* defaultValue) {
	auto valueStr = getProperty(key);
	return valueStr ? valueStr : defaultValue;
}

void ConfigSection::setProperty(const char* key, const char* value) {
	mProperties[key] = value;
}

Color ConfigSection::getColor(const char* key, Color defaultValue) {
	auto valueStr = getProperty(key);
	if (valueStr) {
		stringstream reader(valueStr);
		Color c;

		// Read RGB color, allowing alpha value to be left out
		reader >> c.r >> c.g >> c.b;
		if (reader.fail()) {
			return defaultValue;
		}

		if (!reader.eof()) {
			reader >> c.a;
		}

		return reader.fail() ? defaultValue : c;
	}

	return defaultValue;
}

bool ConfigSection::keyExists(const char* key) {
	return getProperty(key) != nullptr;
}

void Config::addFile(const char* path) {
	addFile(path, cerr);
}

void Config::addFile(const char* path, ostream& errorStream) {
	static const char* whitespace = " \t";

	ifstream in(path);
	string inputLine;
	string sectionName;
	ConfigSection* section = nullptr;
	string key;
	string value;
	stringstream trimmer;

	if (!in.is_open()) {
		errorStream << "failed to open configuration file '" << path << '\'' << endl;
		return;
	}

	for (int lineNumber = 0; in.good(); ++lineNumber) {
		getline(in, inputLine);

		// Parse the first word on the line as the key
		auto keyStartPos = inputLine.find_first_not_of(whitespace);
		if (keyStartPos == string::npos) {
			// Skip empty lines
			continue;
		}

		auto keyEndPos = inputLine.find_first_of(whitespace, keyStartPos);
		key = inputLine.substr(keyStartPos, keyEndPos - keyStartPos);
		transform(key.begin(), key.end(), key.begin(), tolower);

		// Skip commented lines
		if (key[0] == ';') {
			continue;
		}

		// Parse section header
		if (key[0] == '[' && *key.rbegin() == ']') {
			sectionName = key.substr(1, key.length() - 2);
			section = &mSections[sectionName];
			continue;
		}

		// Parse the value
		auto valueStartPos = inputLine.find_first_not_of(whitespace, keyEndPos);
		if (valueStartPos == string::npos) {
			errorStream << path << ": line " << lineNumber
				<< ": missing value for key '" << key << '\'' << endl;
			continue;
		}

		value = inputLine.substr(valueStartPos);

		// If the section is missing, we can't store the value and can't continue parsing
		if (!section) {
			errorStream << path << ": line " << lineNumber
				<< ": missing section name before this point" << endl;
			return;
		}

		// Enter the key-value pair
		section->setProperty(key.c_str(), value.c_str());
	}
}