#include "process_parser.h"

#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "constants.h"
#include "util.h"

using namespace std;

vector<string> ProcessParser::splitLine(const string& line) {
  istringstream iss(line);

  return {istream_iterator<string>{iss}, istream_iterator<string>{}};
}

vector<string> ProcessParser::splitStatLine(const string& line) {
  vector<string> result;

  size_t pos1 = line.find('(');
  size_t pos2 = line.rfind(')');

  if (pos1 == string::npos || pos2 == string::npos || pos2 <= pos1) return {};

  result.push_back(line.substr(0, pos1 - 1));
  result.push_back(line.substr(pos1 + 1, pos2 - pos1 - 1));

  string rest = line.substr(pos2 + 2);

  istringstream iss(rest);

  vector<string> rest_values{istream_iterator<string>{iss},
                             istream_iterator<string>{}};

  result.insert(result.end(), rest_values.begin(), rest_values.end());

  return result;
}

vector<string> ProcessParser::readFirstLineValues(const string& path) {
  ifstream file = Util::getStream(path);

  string line;

  if (!getline(file, line)) return {};

  return splitLine(line);
}

string ProcessParser::getValueByName(const string& path, const string& name) {
  ifstream file = Util::getStream(path);

  string line;

  while (getline(file, line)) {
    if (line.find(name) == 0) {
      auto values = splitLine(line);

      return values.size() > 1 ? values[1] : "";
    }
  }

  return "";
}

string ProcessParser::getProcessNameFromStat(const string& pid) {
  string path = Path::basePath() + pid + Path::statPath();

  ifstream file(path);
  if (!file) return "N/A";

  string line;
  if (!getline(file, line)) return "N/A";

  size_t start = line.find('(');
  size_t end = line.rfind(')');

  if (start == string::npos || end == string::npos || end <= start)
    return "N/A";

  return line.substr(start + 1, end - start - 1);
}

string ProcessParser::getCmd(const string& pid, size_t displayWidth) {
  string path = Path::basePath() + pid + Path::cmdPath();
  ifstream file(path, ios::binary);

  string cmd((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

  for (auto& c : cmd)
    if (c == '\0') c = ' ';

  size_t start = cmd.find_first_not_of(' ');
  size_t end = cmd.find_last_not_of(' ');

  cmd = (start == string::npos) ? "" : cmd.substr(start, end - start + 1);

  if (cmd.empty()) cmd = getProcessNameFromStat(pid);

  if (displayWidth > 0 && cmd.length() > displayWidth) cmd.resize(displayWidth);

  return cmd.empty() ? "N/A" : cmd;
}

vector<string> ProcessParser::getPidList() {
  vector<string> pids;

  for (const auto& entry : filesystem::directory_iterator(Path::basePath())) {
    string name = entry.path().filename();

    if (!name.empty() && all_of(name.begin(), name.end(), ::isdigit)) {
      if (filesystem::exists(Path::basePath() + name + "/" + Path::statPath()))
        pids.push_back(name);
    }
  }

  return pids;
}

string ProcessParser::getVmSize(const string& pid) {
  string value =
      getValueByName(Path::basePath() + pid + Path::statusPath(), "VmData:");

  if (value.empty()) return "0";

  return to_string(stof(value) / 1024.0f);
}

long int ProcessParser::getSysUpTime() {
  auto values = readFirstLineValues(Path::basePath() + Path::upTimePath());

  if (values.empty()) return 0;

  return stol(values[0]);
}

string ProcessParser::getProcUpTime(const string& pid) {
  ifstream file =
      Util::getStream(Path::basePath() + pid + "/" + Path::statPath());

  string line;
  if (!getline(file, line)) return "0";

  auto values = splitStatLine(line);

  if (values.size() < 22) return "0";

  long starttime = stol(values[21]);
  long uptime = getSysUpTime();
  long freq = sysconf(_SC_CLK_TCK);

  return to_string(static_cast<float>(uptime) -
                   (static_cast<float>(starttime) / freq));
}

string ProcessParser::getCpuPercent(const string& pid) {
  ifstream file =
      Util::getStream(Path::basePath() + pid + "/" + Path::statPath());

  string line;
  if (!getline(file, line)) return "0";

  auto values = splitStatLine(line);

  if (values.size() < 22) return "0";

  long utime = stol(values[13]);
  long stime = stol(values[14]);
  long cutime = stol(values[15]);
  long cstime = stol(values[16]);
  long starttime = stol(values[21]);
  long total_time = utime + stime + cutime + cstime;
  long uptime = getSysUpTime();
  long freq = sysconf(_SC_CLK_TCK);
  float seconds =
      static_cast<float>(uptime) - static_cast<float>(starttime) / freq;

  if (seconds <= 0) return "0";

  return to_string(100.0f *
                   ((static_cast<float>(total_time) / freq) / seconds));
}

string ProcessParser::getProcUser(const string& pid) {
  string uid =
      getValueByName(Path::basePath() + pid + Path::statusPath(), "Uid:");
  if (uid.empty()) return "";

  ifstream file = Util::getStream("/etc/passwd");
  string line;
  string token = "x:" + uid;

  while (getline(file, line)) {
    if (line.find(token) != string::npos) return line.substr(0, line.find(":"));
  }

  return "";
}

int ProcessParser::getNumberOfCores() {
  unsigned cores = thread::hardware_concurrency();

  if (cores > 0) return cores;

  ifstream file = Util::getStream(Path::basePath() + Path::cpuPath());
  string line;

  int count = 0;

  while (getline(file, line)) {
    if (line.compare(0, 9, "processor") == 0) count++;
  }

  return count > 0 ? count : 1;
}

vector<string> ProcessParser::getSysCpuPercent(const string& coreNumber) {
  ifstream file = Util::getStream(Path::basePath() + Path::statPath());
  string line, name = "cpu" + coreNumber;

  while (getline(file, line)) {
    if (line.compare(0, name.size(), name) == 0) {
      vector<string> vals = splitLine(line);
      if (!vals.empty() && vals[0] == name) return vals;
    }
  }

  return {};
}

float ProcessParser::getSysActiveCpuTime(const vector<string>& values) {
  if (values.size() < 11) return 0.0f;

  long active_time = stol(values[1]) + stol(values[2]) + stol(values[3]) +
                     stol(values[6]) + stol(values[7]) + stol(values[8]) +
                     stol(values[9]) + stol(values[10]);

  return static_cast<float>(active_time);
}

float ProcessParser::getSysIdleCpuTime(const vector<string>& values) {
  if (values.size() < 5) return 0.0f;

  long idle_time = stol(values[4]) + stol(values[5]);

  return static_cast<float>(idle_time);
}

string ProcessParser::printCpuStats(const vector<string>& oldVals,
                                    const vector<string>& newVals) {
  if (oldVals.empty() || newVals.empty()) return "0.0";

  float active_diff =
      getSysActiveCpuTime(newVals) - getSysActiveCpuTime(oldVals);
  float idle_diff = getSysIdleCpuTime(newVals) - getSysIdleCpuTime(oldVals);
  float total_diff = active_diff + idle_diff;

  if (total_diff <= 0.0f) return "0.0";

  return to_string(100.0f * (active_diff / total_diff));
}

float ProcessParser::getSysRamPercent() {
  ifstream file = Util::getStream(Path::basePath() + Path::memInfoPath());
  string line;

  float total = 0, available = 0, free = 0, buffers = 0;

  while (getline(file, line)) {
    if (line.find("MemTotal:") == 0)
      total = stof(splitLine(line)[1]);
    else if (line.find("MemAvailable:") == 0)
      available = stof(splitLine(line)[1]);
    else if (line.find("MemFree:") == 0)
      free = stof(splitLine(line)[1]);
    else if (line.find("Buffers:") == 0)
      buffers = stof(splitLine(line)[1]);
  }

  if (available > 0) return 100.0f * (1.0f - (available / total));

  return 100.0f * (1.0f - ((free + buffers) / total));
}

string ProcessParser::getSysKernelVersion() {
  ifstream file = Util::getStream(Path::basePath() + Path::versionPath());
  string line;

  while (getline(file, line)) {
    if (line.find("Linux version ") == 0) return splitLine(line)[2];
  }

  return "";
}

string ProcessParser::getOsName() {
  ifstream file = Util::getStream("/etc/os-release");
  string line;

  while (getline(file, line)) {
    if (line.find("PRETTY_NAME=") == 0) {
      string res = line.substr(line.find("=") + 1);

      res.erase(remove(res.begin(), res.end(), '"'), res.end());

      return res;
    }
  }

  return "";
}

int ProcessParser::getTotalThreads() {
  int total = 0;

  for (const auto& pid : getPidList()) {
    ifstream file =
        Util::getStream(Path::basePath() + pid + Path::statusPath());
    string line;
    while (getline(file, line)) {
      if (line.find("Threads:") == 0) {
        total += stoi(splitLine(line)[1]);
        break;
      }
    }
  }

  return total;
}

int ProcessParser::getTotalNumberOfProcesses() {
  ifstream file = Util::getStream(Path::basePath() + Path::statPath());
  string line;

  while (getline(file, line)) {
    if (line.find("processes") == 0) return stoi(splitLine(line)[1]);
  }

  return 0;
}

int ProcessParser::getNumberOfRunningProcesses() {
  ifstream file = Util::getStream(Path::basePath() + Path::statPath());
  string line;

  while (getline(file, line)) {
    if (line.find("procs_running") == 0) return stoi(splitLine(line)[1]);
  }

  return 0;
}
