#pragma once

class Shader {
public:
  ~Shader();

  void use();
  void init(const char *vshader_path, const char *fshader_path);
  template <typename T> void set(const char *name, T value);

private:
  unsigned int program;
};
