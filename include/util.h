#pragma once
#include <fstream>
#include <string>

class Util {
 public:
  static std::string convertToTime(long int input_seconds);
  static std::string getProgressBar(const std::string& percent);
  static std::ifstream getStream(const std::string& path);
};