#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

struct Vertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
};

struct InstancedMesh {
  ~InstancedMesh();

  void init_buffers();
  void render();

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<glm::mat4> model_matrices;

  unsigned int vao, vbo, ebo;
  unsigned int model_matrices_buffer;
};

InstancedMesh generate_unit_sphere(int longitudes, int lattitudes);
InstancedMesh generate_circle_mesh(int num_fans);