#include <algorithm>
#include <assert.h>
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

#include "mesh.h"
#include "satellite.h"

// https://medium.com/@zpm.mehrdad/cube-projection-from-equirectangular-panorama-using-c-03813afa0cd2
// https://mycoordinates.org/tracking-satellite-footprints-on-earth%E2%80%99s-surface/
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

glm::quat rotate(glm::quat rotation, float padx, float pady, float dx,
                 float dy) {
  glm::vec3 world_up = glm::vec3(0.0, 1.0, 0.0);
  glm::vec3 world_right = rotation * glm::vec3(1.0, 0.0, 0.0);

  glm::quat yaw = glm::angleAxis(dx * padx, world_up);
  glm::quat pitch = glm::angleAxis(dy * pady, world_right);

  return glm::normalize(yaw * pitch * rotation);
}

glm::mat4 satellite_to_model(Satellite s) {
  s.propagate(0);
  glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.01, 0.01, 0.01));
  // glm::mat4 translate = glm::translate(glm::mat4(0.0), s.position * 0.0001f);
  glm::mat4 translate =
      glm::translate(glm::mat4(0.0), glm::vec3(0.0, 0.0, 0.0));
  return translate * scale;
}
int main() {
  auto satellites = read_satellite_data("../data/starlink.csv");

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

    /*
    glm::quat globe_rotation = glm::quat(1.0, 0.0, 0.0, 0.0);
    glm::mat4 globe_scale =
        glm::scale(glm::mat4(1.0), glm::vec3(2.0, 2.0, 2.0));
    InstancedMesh globe = generate_unit_sphere(32, 32);
    globe.model_matrices.push_back(glm::mat4(1.0));
    */

    InstancedMesh circles = generate_circle_mesh(10);
    std::transform(satellites.begin(), satellites.end(),
                   std::back_inserter(circles.model_matrices),
                   satellite_to_model);
    std::cout << circles.model_matrices.size() << "\n";

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

      /*
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      bool mouse_down =
          glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
      if (mouse_down) {
        globe_rotation =
            rotate(globe_rotation, 0.01, 0.005, x - prev_x, y - prev_y);
        globe.model_matrices[0] = glm::mat4(globe_rotation) * globe_scale;
      }
      prev_x = x;
      prev_y = y;
      */

      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 p = camera.projection_matrix();
      glm::mat4 v = camera.view_matrix();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      shader.use();
      shader.set_mat4("projection", p);
      shader.set_mat4("view", v);

      // globe.render();
      circles.render();

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteTextures(1, &texture);
  }

  glfwTerminate();
  return 0;
}
