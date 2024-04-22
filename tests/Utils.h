#ifndef UTILS_H_
#define UTILS_H_

#include <string>

inline std::string getSourceDir(const std::string &fileName)
{
  std::size_t pos = fileName.find_last_of("\\/");

  return fileName.substr(0, pos + 1);
}

#endif
