#pragma once

#include "satellite.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <set>

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class GLFWContext {
public:
  GLFWContext();
  ~GLFWContext();
};

struct InputState {
  float yscroll;
  glm::vec2 cursor_delta;
  glm::vec2 prev_cursor;
  std::set<int> keys;
  glm::mat4 projection;
  bool mouse_pressed;

  InputState()
      : yscroll(0), cursor_delta(0.0), prev_cursor(0.0), projection(1.0),
        mouse_pressed(false) {}
};

class Visualizer {
public:
  Visualizer(int width, int height);
  void run();

private:
  void create_window(int width, int height);
  void set_callbacks();
  void init_scene_objects();
  void render_scene();

  GLFWwindow *window;
  InputState state;

  std::vector<InstanceData> globe_instances;
  std::vector<InstanceData> circle_instances;
  double earth_scale, constellation_time_step;

  Camera camera;
  Shader main_shader;
  Shader cubemap_shader;
  Skybox skybox;
  Texture cubemap_texture;
  Texture earth_texture;
  InstancedMesh globe;
  InstancedMesh circles;
  Constellation constellation;
};
