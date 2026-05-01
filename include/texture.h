#pragma once

#include <string>
#include <vector>

class Texture {
public:
  ~Texture();
  void init(std::vector<std::string> paths);
  void use();

private:
  unsigned int id, obj;
};
