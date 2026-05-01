#include <cmath>
#include <glad/glad.h>

#include "mesh.h"

Skybox::~Skybox() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}

void Skybox::init() {
  glm::vec3 cube_positions[] = {
      // +X (right)
      glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), glm::vec3(1, 1, 1),
      glm::vec3(1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(1, 1, -1),

      // -X (left)
      glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1),
      glm::vec3(-1, -1, 1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, 1),

      // +Y (top)
      glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1), glm::vec3(1, 1, 1),
      glm::vec3(-1, 1, -1), glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1),

      // -Y (bottom)
      glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1),
      glm::vec3(-1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1),

      // +Z (front)
      glm::vec3(-1, -1, 1), glm::vec3(-1, 1, 1), glm::vec3(1, 1, 1),
      glm::vec3(-1, -1, 1), glm::vec3(1, 1, 1), glm::vec3(1, -1, 1),

      // -Z (back)
      glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1),
      glm::vec3(1, -1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1)};

  glGenVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_positions), cube_positions,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glEnableVertexAttribArray(0);
}

void Skybox::render() {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

InstancedMesh::InstancedMesh() : initialized(false) {}

InstancedMesh::InstancedMesh(InstancedMesh &&other) noexcept {
  *this = std::move(other);
}

InstancedMesh &InstancedMesh::operator=(InstancedMesh &&other) noexcept {
  if (this != &other) {
    if (initialized) {
      glDeleteVertexArrays(1, &vao);
      glDeleteBuffers(1, &vbo);
      glDeleteBuffers(1, &ebo);
      glDeleteBuffers(1, &ssbo);
    }

    vao = other.vao;
    vbo = other.vbo;
    ebo = other.ebo;
    ssbo = other.ssbo;
    num_indices = other.num_indices;
    ssbo_size = other.ssbo_size;
    initialized = other.initialized;

    other.initialized = false;
    other.vao = other.vbo = other.ebo = other.ssbo = 0;
  }
  return *this;
}

InstancedMesh::InstancedMesh(std::vector<Vertex> vertices,
                             std::vector<unsigned int> indices) {
  num_indices = indices.size();
  ssbo_size = sizeof(InstanceData);
  initialized = true;

  glGenVertexArrays(1, &vao);
  glCreateBuffers(1, &vbo);
  glCreateBuffers(1, &ebo);
  glCreateBuffers(1, &ssbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size, nullptr, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, uv));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));
  glEnableVertexAttribArray(2);
}

InstancedMesh::~InstancedMesh() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &ssbo);
}

void InstancedMesh::render(std::vector<InstanceData> &data) {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

  // Resize the SSBO if needed
  int new_size = data.size() * sizeof(InstanceData);
  if (new_size > ssbo_size) {
    glBufferData(GL_SHADER_STORAGE_BUFFER, new_size, data.data(),
                 GL_DYNAMIC_DRAW);
    ssbo_size = new_size;
  } else {
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, ssbo_size, data.data());
  }

  glBindVertexArray(vao);
  glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0,
                          data.size());
}

InstancedMesh create_unit_sphere(int longitudes, int lattitudes) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // Add vertices
  for (int i = 0; i <= lattitudes; i++) {
    float lattitude_angle =
        (M_PI / 2.0) - i * (M_PI / (float)lattitudes); // -pi/2 to pi/2
    float xy = cos(lattitude_angle);
    float z = sin(lattitude_angle);

    for (int j = 0; j <= longitudes; j++) {
      float longitude_angle = j * (2.0 * M_PI / (float)longitudes);

      Vertex v;
      v.position =
          glm::vec3(xy * cos(longitude_angle), z, xy * sin(longitude_angle));
      v.uv = glm::vec2((float)j / (float)longitudes,
                       1.0 - (float)i / (float)lattitudes);
      v.normal = v.position;

      vertices.push_back(v);
    }
  }

  // Add indices
  for (int i = 0; i < lattitudes; i++) {
    int top = i * (longitudes + 1);
    int bottom = top + longitudes + 1;

    for (int j = 0; j < longitudes; j++, top++, bottom++) {
      if (i != 0) {
        indices.push_back(top);
        indices.push_back(bottom);
        indices.push_back(top + 1);
      }

      if (i != (lattitudes - 1)) {
        indices.push_back(top + 1);
        indices.push_back(bottom);
        indices.push_back(bottom + 1);
      }
    }
  }

  return InstancedMesh(vertices, indices);
}

InstancedMesh create_circle_mesh(int num_fans) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  Vertex v;
  v.uv = v.normal = glm::vec3(0.0, 0.0, 0.0);
  v.position = glm::vec3(0.0, 0.0, 0.0);
  vertices.push_back(v);

  for (int i = 0; i < num_fans; i++) {
    Vertex v;
    float angle = i * ((2.0 * M_PI) / (float)num_fans);
    v.position = glm::vec3(std::cos(angle), std::sin(angle), 0);
    v.uv = v.normal = glm::vec3(0.0, 0.0, 0.0);
    vertices.push_back(v);
  }

  for (int i = 1; i <= num_fans; i++) {
    indices.push_back(0);
    indices.push_back(i);
    indices.push_back((i % num_fans) + 1);
  }

  return InstancedMesh(vertices, indices);
}
