#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_transform.hpp"
#include <assert.h>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// https://medium.com/@zpm.mehrdad/cube-projection-from-equirectangular-panorama-using-c-03813afa0cd2
// TODO:
// - Tie up some loose ends (resizing, error handling, scale normal by normal
//   matrix, basic phong lighting, etc) and refactor
// - Render instanced spheres
// - Render a basic skybox

unsigned int load_shader(const char *path, int type) {
  auto size = std::filesystem::file_size(path);
  std::string contents(size, '\0');
  std::ifstream file(path);
  assert(file.good() && file.is_open());
  file.read(&contents[0], size);

  const char *data = contents.c_str();
  unsigned int id = glCreateShader(type);
  glShaderSource(id, 1, &data, nullptr);
  glCompileShader(id);

  int success = 0;
  char info_log[1024];
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(id, 1024, nullptr, info_log);
    std::cout << path << " -> " << info_log << "\n";
    std::exit(-1);
  }

  return id;
}

class Shader {
public:
  explicit Shader(const char *vshader_path, const char *fshader_path) {
    unsigned int vertex = load_shader(vshader_path, GL_VERTEX_SHADER);
    unsigned int fragment = load_shader(fshader_path, GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    int success = 0;
    char info_log[1024];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(program, 1024, nullptr, info_log);
      std::cout << "Link error -> " << info_log << "\n";
      std::exit(-1);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  ~Shader() { glDeleteProgram(program); }

  void use() { glUseProgram(program); }

  void set_vec3(const char *name, glm::vec3 &value) {
    glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
  }

  void set_mat4(const char *name, glm::mat4 &value) {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE,
                       glm::value_ptr(value));
  }

private:
  unsigned int program;
};

struct Vertex {
  glm::vec3 position;
  glm::vec2 uv;
  glm::vec3 normal;
};

struct Mesh {
  Mesh() : translation(1.0), scale(1.0), current_rotation(1.0, 0.0, 0.0, 0.0) {}
  ~Mesh();

  glm::mat4 model_matrix();
  void render();
  void init_buffers();
  void rotate(float padx, float pady, float dx, float dy);

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  unsigned int vao, vbo, ebo;
  glm::mat4 translation, scale;
  glm::quat current_rotation;
};

void Mesh::init_buffers() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);
  glBindVertexArray(vao);

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

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

void Mesh::render() {
  glBindVertexArray(vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

glm::mat4 Mesh::model_matrix() {
  return translation * glm::mat4(current_rotation) * scale;
}

void Mesh::rotate(float padx, float pady, float dx, float dy) {
  glm::vec3 world_up = glm::vec3(0.0, 1.0, 0.0);
  glm::vec3 world_right = current_rotation * glm::vec3(1.0, 0.0, 0.0);

  glm::quat yaw = glm::angleAxis(dx * padx, world_up);
  glm::quat pitch = glm::angleAxis(dy * pady, world_right);

  current_rotation = glm::normalize(yaw * pitch * current_rotation);
}

Mesh generate_unit_sphere(int longitudes, int lattitudes) {
  Mesh mesh;

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

      mesh.vertices.push_back(v);
    }
  }

  // Add indices
  for (int i = 0; i < lattitudes; i++) {
    int top = i * (longitudes + 1);
    int bottom = top + longitudes + 1;

    for (int j = 0; j < longitudes; j++, top++, bottom++) {
      if (i != 0) {
        mesh.indices.push_back(top);
        mesh.indices.push_back(bottom);
        mesh.indices.push_back(top + 1);
      }

      if (i != (lattitudes - 1)) {
        mesh.indices.push_back(top + 1);
        mesh.indices.push_back(bottom);
        mesh.indices.push_back(bottom + 1);
      }
    }
  }

  mesh.init_buffers();
  return mesh;
}

class Camera {
public:
  explicit Camera(float aspect_ratio) {
    projection =
        glm::perspective((float)M_PI / 4.0f, aspect_ratio, 0.1f, 100.0f);

    up = glm::vec3(0.0, 1.0, 0.0);
    pos = glm::vec3(0.0, 0.0, 3.0);
    front = glm::vec3(0.0, 0.0, -1.0); // -Z goes into the screen
  }

  void move(float direction) { pos += front * direction; }

  glm::mat4 projection_matrix() { return projection; }
  glm::mat4 view_matrix() { return glm::lookAt(pos, pos + front, up); }

private:
  glm::vec3 up, pos, front;
  glm::mat4 projection;
};

int main() {
  // auto satellites = read_satellite_data("../data/starlink.csv");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int window_width = 900, window_height = 700;
  GLFWwindow *window = glfwCreateWindow(window_width, window_height,
                                        "LEO Visualization", nullptr, nullptr);
  assert(window != nullptr);

  glfwMakeContextCurrent(window);
  assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != 0);

  glViewport(0, 0, window_width, window_height);
  glEnable(GL_DEPTH_TEST);

  float aspect_ratio = (float)window_width / (float)window_height;
  Camera camera(aspect_ratio);

  {
    Shader shader("../src/vertex.glsl", "../src/fragment.glsl");
    Mesh sphere = generate_unit_sphere(32, 32);
    sphere.scale = glm::scale(sphere.scale, glm::vec3(2.0, 2.0, 2.0));

    const char *path = "../data/8081_earthmap4k.jpg";
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *pixels = stbi_load(path, &width, &height, &channels, 3);
    assert(pixels != nullptr && channels == 3);

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(pixels);

    double prev_x = 0, prev_y = 0;
    glfwGetCursorPos(window, &prev_x, &prev_y);

    while (!glfwWindowShouldClose(window)) {
      if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        break;

      if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.move(-1);

      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.move(1);

      double x, y;
      glfwGetCursorPos(window, &x, &y);
      bool mouse_down =
          glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
      if (mouse_down)
        sphere.rotate(0.01, 0.005, x - prev_x, y - prev_y);
      prev_x = x;
      prev_y = y;

      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 p = camera.projection_matrix();
      glm::mat4 v = camera.view_matrix();
      glm::mat4 model = sphere.model_matrix();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.use();
      shader.set_mat4("projection", p);
      shader.set_mat4("view", v);
      shader.set_mat4("model", model);
      sphere.render();

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
  }

  glfwTerminate();
  return 0;
}
