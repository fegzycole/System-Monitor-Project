#include "util.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "constants.h"

std::string Util::convertToTime(long int input_seconds) {
  long seconds = input_seconds % 60;
  long minutes = (input_seconds / 60) % 60;
  long hours = input_seconds / 3600;

  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2)
      << minutes << ":" << std::setw(2) << seconds;

  return oss.str();
}

std::string Util::getProgressBar(const std::string& percent) {
  std::string result = "0% ";

  int size = 50;
  float perc = 0.0f;

  try {
    perc = std::stof(percent);
  } catch (...) {
    perc = 0.0f;
  }

  int boundaries = static_cast<int>((perc / 100.0f) * size);

  for (int i = 0; i < size; i++) result += (i <= boundaries) ? "|" : " ";

  result += " " + percent.substr(0, 5) + " /100%";

  return result;
}

std::ifstream Util::getStream(const std::string& path) {
  std::ifstream stream(path);
  return stream;
}