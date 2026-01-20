#include "process_container.h"

#include "process_parser.h"

void ProcessContainer::refreshList() {
  _list.clear();

  for (const auto& pid : ProcessParser::getPidList()) {
    try {
      _list.emplace_back(pid);
    } catch (...) {
    }
  }
}

std::vector<std::string> ProcessContainer::getList() {
  std::vector<std::string> out;

  size_t start = (_list.size() > 10) ? _list.size() - 10 : 0;

  for (size_t i = start; i < _list.size(); i++)
    out.push_back(_list[i].getProcess());

  return out;
}
