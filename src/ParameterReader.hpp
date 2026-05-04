#ifndef PARAMETER_READER_H
#define PARAMETER_READER_H

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <vector>

class ParameterReader {
private:
    std::map<std::string, std::string> parameters;

    // Internal helper to normalize keys
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    }

    // Internal helper to clean whitespace
    std::string trim(const std::string& s) {
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, (last - first + 1));
    }

public:
    ParameterReader() = default;

    /**
     * Loads parameters from a text file. 
     * Expected format: key = value
     */
    bool load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        parameters.clear(); // Clear existing data if loading a new file
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#' || line[0] == ';') continue;

            size_t sep = line.find('=');
            if (sep != std::string::npos) {
                std::string key = toLower(trim(line.substr(0, sep)));
                std::string value = trim(line.substr(sep + 1));
                parameters[key] = value;
            }
        }
        return true;
    }

    /**
     * Retrieves a value cast to type T.
     * Usage: int val = reader.get<int>("speed", 100);
     */
    template <typename T>
    T get(const std::string& key, T defaultVal = T()) {
        std::string lowerKey = toLower(key);
        auto it = parameters.find(lowerKey);
        
        if (it == parameters.end()) {
            return defaultVal;
        }

        std::stringstream ss(it->second);
        T result;
        if (!(ss >> result)) {
            return defaultVal;
        }
        return result;
    }

    /**
     * Specialized logic for boolean strings (true/false, yes/no, 1/0)
     */
    bool getBool(const std::string& key, bool defaultVal = false) {
        std::string val = toLower(get<std::string>(key));
        if (val == "true" || val == "1" || val == "yes" || val == "on") return true;
        if (val == "false" || val == "0" || val == "no" || val == "off") return false;
        return defaultVal;
    }

    /**
     * Retrieves a list of values separated by commas.
     * Usage: std::vector<int> scores = reader.getList<int>("high_scores");
     */
    template <typename T>
    std::vector<T> getList(const std::string& key) {
        std::vector<T> results;
        std::string lowerKey = toLower(key);
        
        auto it = parameters.find(lowerKey);
        if (it == parameters.end()) {
            return results; // Return empty vector if key not found
        }

        std::stringstream ss(it->second);
        std::string item;
        
        // Split by comma
        while (std::getline(ss, item, ',')) {
            std::string cleanedItem = trim(item);
            if (cleanedItem.empty()) continue;

            // Convert individual item to type T
            std::stringstream converter(cleanedItem);
            T value;
            if (converter >> value) {
                results.push_back(value);
            }
        }
        
        return results;
    }

    /**
     * Specialized getList for strings to handle multi-word items properly
     */
    std::vector<std::string> getListString(const std::string& key) {
        std::vector<std::string> results;
        std::string lowerKey = toLower(key);
        
        auto it = parameters.find(lowerKey);
        if (it == parameters.end()) return results;

        std::stringstream ss(it->second);
        std::string item;
        while (std::getline(ss, item, ',')) {
            results.push_back(trim(item));
        }
        return results;
    }
    
    // Checks if a key exists
    bool exists(const std::string& key) {
        return parameters.find(toLower(key)) != parameters.end();
    }
};

#endif // PARAMETER_READER_H
