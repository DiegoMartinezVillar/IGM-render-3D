// Copyright (C) 2020 Emilio J. Padrón
// Released as Free Software under the X11 License
// https://spdx.org/licenses/X11.html

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLM library to deal with matrix operations
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include "textfile_ALT.h"


#define NUM_LIGHTS 2

int gl_width = 640;
int gl_height = 480;

void glfw_window_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void render(double);
unsigned int load_texture(const char* filename);

GLuint shader_program = 0; // shader program to set render pipeline
GLuint vao = 0; // Vertext Array Object to set input data
GLint model_location, view_location, proj_location, normal_location, view_pos_location; // Uniforms for transformation matrices
GLint material_shininess_location; // Uniforms for material data
GLint light_position_location[NUM_LIGHTS], light_ambient_location[NUM_LIGHTS], light_diffuse_location[NUM_LIGHTS], light_specular_location[NUM_LIGHTS]; // Uniforms for light data
GLint texture_diffuse_location, texture_specular_location; // Uniform for texture data
GLint texture_diffuse_map, texture_specular_map; // Texture maps

// Shader names
const char *vertexFileName = "spinningcube_withlight_vs_SKEL.glsl";
const char *fragmentFileName = "spinningcube_withlight_fs_SKEL.glsl";

// Camera
glm::vec3 camera_pos(0.0f, 0.0f, 1.0f);

// Lighting
glm::vec3 light_positions[] = { // 2 lights with the same ambient, diffuse and specular
  glm::vec3(0.0f, 2.0f, -3.0f),
  glm::vec3(3.0f, 0.0f, -3.0f)
};
glm::vec3 light_ambient(0.2f, 0.2f, 0.2f);
glm::vec3 light_diffuse(0.5f, 0.5f, 0.5f);
glm::vec3 light_specular(1.0f, 1.0f, 1.0f);

// Material
const GLfloat material_shininess = 32.0f;

int main() {
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  //  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "My spinning cube and tetrahedron", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwMakeContextCurrent(window);

  // start GLEW extension handler
  // glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* vendor = glGetString(GL_VENDOR); // get vendor string
  const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte* glversion = glGetString(GL_VERSION); // version as a string
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION); // version as a string
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", glversion);
  printf("GLSL version supported %s\n", glslversion);
  printf("Starting viewport: (width: %d, height: %d)\n", gl_width, gl_height);

  // Enable Depth test: only draw onto a pixel if fragment closer to viewer
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); // set a smaller value as "closer"

  // Vertex Shader
  char* vertex_shader = textFileRead(vertexFileName);

  // Fragment Shader
  char* fragment_shader = textFileRead(fragmentFileName);

  // Shaders compilation
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  free(vertex_shader);
  glCompileShader(vs);

  int  success;
  char infoLog[512];
  glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vs, 512, NULL, infoLog);
    printf("ERROR: Vertex Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  free(fragment_shader);
  glCompileShader(fs);

  glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fs, 512, NULL, infoLog);
    printf("ERROR: Fragment Shader compilation failed!\n%s\n", infoLog);

    return(1);
  }

  // Create program, attach shaders to it and link it
  shader_program = glCreateProgram();
  glAttachShader(shader_program, fs);
  glAttachShader(shader_program, vs);
  glLinkProgram(shader_program);

  glValidateProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
    printf("ERROR: Shader Program linking failed!\n%s\n", infoLog);

    return(1);
  }

  // Release shader objects
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Vertex Array Object
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Cube to be rendered
  //
  //          0        3
  //       7        4 <-- top-right-near
  // bottom
  // left
  // far ---> 1        2
  //       6        5
  //
  // Tetrahedron to be rendered
  //
  //         0 <-- top
  //
  //
  //  1            3 <-- bottom-right
  //
  //       2 <-- bottom-center-near
  const GLfloat vertex_positions[] = {
    // Cube
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f, -0.25f, // 0
     0.25f, -0.25f, -0.25f, // 2

     0.25f,  0.25f, -0.25f, // 3
     0.25f, -0.25f, -0.25f, // 2
    -0.25f,  0.25f, -0.25f, // 0

     0.25f, -0.25f, -0.25f, // 2
     0.25f,  0.25f, -0.25f, // 3
     0.25f, -0.25f,  0.25f, // 5

     0.25f,  0.25f,  0.25f, // 4
     0.25f, -0.25f,  0.25f, // 5
     0.25f,  0.25f, -0.25f, // 3

     0.25f, -0.25f,  0.25f, // 5
     0.25f,  0.25f,  0.25f, // 4
    -0.25f, -0.25f,  0.25f, // 6

    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f,  0.25f, // 6
     0.25f,  0.25f,  0.25f, // 4

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f,  0.25f,  0.25f, // 7
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f, -0.25f, -0.25f, // 1
    -0.25f,  0.25f,  0.25f, // 7

     0.25f, -0.25f, -0.25f, // 2
     0.25f, -0.25f,  0.25f, // 5
    -0.25f, -0.25f, -0.25f, // 1

    -0.25f, -0.25f,  0.25f, // 6
    -0.25f, -0.25f, -0.25f, // 1
     0.25f, -0.25f,  0.25f, // 5

     0.25f,  0.25f,  0.25f, // 4
     0.25f,  0.25f, -0.25f, // 3
    -0.25f,  0.25f,  0.25f, // 7

    -0.25f,  0.25f, -0.25f, // 0
    -0.25f,  0.25f,  0.25f, // 7
     0.25f,  0.25f, -0.25f, // 3

    // Tetrahedron
     0.0f,   0.25f,  0.0f,  // 0
    -0.25f, -0.25f, -0.25f, // 1
     0.0f,  -0.25f,  0.25f, // 2

     0.0f,   0.25f,  0.0f,  // 0
     0.0f,  -0.25f,  0.25f, // 2
     0.25f, -0.25f, -0.25f, // 3

     0.0f,   0.25f,  0.0f,  // 0
     0.25f, -0.25f, -0.25f, // 3
    -0.25f, -0.25f, -0.25f, // 1

     0.25f, -0.25f, -0.25f, // 3
     0.0f,  -0.25f,  0.25f, // 2
    -0.25f, -0.25f, -0.25f, // 1
  };

  const GLfloat texture_positions[] {
    // Cube
    1.0f, 0.0f, // 1
    1.0f, 1.0f, // 0
    0.0f, 0.0f, // 2

    0.0f, 1.0f, // 3
    0.0f, 0.0f, // 2
    1.0f, 1.0f, // 0

    1.0f, 0.0f, // 2
    1.0f, 1.0f, // 3
    0.0f, 0.0f, // 5

    0.0f, 1.0f, // 4
    0.0f, 0.0f, // 5
    1.0f, 1.0f, // 3

    1.0f, 0.0f, // 5
    1.0f, 1.0f, // 4
    0.0f, 0.0f, // 6

    0.0f, 1.0f, // 7
    0.0f, 0.0f, // 6
    1.0f, 1.0f, // 4

    1.0f, 0.0f, // 6
    1.0f, 1.0f, // 7
    0.0f, 0.0f, // 1

    0.0f, 1.0f, // 0
    0.0f, 0.0f, // 1
    1.0f, 1.0f, // 7

    1.0f, 0.0f, // 2
    1.0f, 1.0f, // 5
    0.0f, 0.0f, // 1

    0.0f, 1.0f, // 6
    0.0f, 0.0f, // 1
    1.0f, 1.0f, // 5

    1.0f, 0.0f, // 4
    1.0f, 1.0f, // 3
    0.0f, 0.0f, // 7

    0.0f, 1.0f, // 0
    0.0f, 0.0f, // 7
    1.0f, 1.0f, // 3

    // Tetrahedron
    0.5f, 1.0f, // 0
    0.0f, 0.0f, // 1
    1.0f, 0.0f, // 2

    0.5f, 1.0f, // 0
    0.0f, 0.0f, // 2
    1.0f, 0.0f, // 3

    0.5f, 1.0f, // 0
    0.0f, 0.0f, // 3
    1.0f, 0.0f, // 1

    1.0f, 0.0f, // 3
    0.5f, 1.0f, // 2
    0.0f, 0.0f, // 1

  };

  // Vertex Buffer Object (for vertex coordinates)
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_positions), vertex_positions, GL_STATIC_DRAW);

  // Vertex attributes
  // 0: vertex position (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(0);

  // 1: vertex normals (x, y, z)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(1);

  // Vertex Buffer Object (for texture coordinates)
  GLuint tvbo = 0;
  glGenBuffers(1, &tvbo);
  glBindBuffer(GL_ARRAY_BUFFER, tvbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texture_positions), texture_positions, GL_STATIC_DRAW);

  // 2: texture coordinates (u, v)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
  glEnableVertexAttribArray(2);
  
  // Unbind vbo (it was conveniently registered by VertexAttribPointer)
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Unbind vao
  glBindVertexArray(0);

  texture_diffuse_map = load_texture("texture.png");
  texture_specular_map = load_texture("texture_specular.png");

  // Uniforms
  // - Model matrix
  // - View matrix
  // - Projection matrix
  // - Normal matrix: normal vectors from local to world coordinates
  // - Camera position
  // - Light data
  // - Material data
  // - Texture data
  model_location = glGetUniformLocation(shader_program, "model");
  view_location = glGetUniformLocation(shader_program, "view");
  proj_location = glGetUniformLocation(shader_program, "projection");
  normal_location = glGetUniformLocation(shader_program, "normal_matrix");
  view_pos_location = glGetUniformLocation(shader_program, "view_pos");

  for (int i = 0; i < 2; i++) {
    char light_position_name[64];
    char light_ambient_name[64];
    char light_diffuse_name[64];
    char light_specular_name[64];

    sprintf(light_position_name, "light[%d].position", i);
    sprintf(light_ambient_name, "light[%d].ambient", i);
    sprintf(light_diffuse_name, "light[%d].diffuse", i);
    sprintf(light_specular_name, "light[%d].specular", i);

    light_position_location[i] = glGetUniformLocation(shader_program, light_position_name);
    light_ambient_location[i] = glGetUniformLocation(shader_program, light_ambient_name);
    light_diffuse_location[i] = glGetUniformLocation(shader_program, light_diffuse_name);
    light_specular_location[i] = glGetUniformLocation(shader_program, light_specular_name);}

  material_shininess_location = glGetUniformLocation(shader_program, "material.shininess");
  
  texture_diffuse_location = glGetUniformLocation(shader_program, "material.diffuse");
  texture_specular_location = glGetUniformLocation(shader_program, "material.specular");

  glUniform1i(texture_diffuse_location, 0);
  glUniform1i(texture_specular_location, 1);

// Render loop
  while(!glfwWindowShouldClose(window)) {

    processInput(window);

    render(glfwGetTime());

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void render(double currentTime) {
  float f = (float)currentTime * 0.3f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, gl_width, gl_height);

  glUseProgram(shader_program);
  glBindVertexArray(vao);

  glm::mat4 model_matrix, view_matrix, proj_matrix;
  glm::mat3 normal_matrix;

  // Cube
  model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -3.0f));
  model_matrix = glm::translate(model_matrix,
                             glm::vec3(sinf(2.1f * f) * 0.5f,
                                       cosf(1.7f * f) * 0.5f,
                                       sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 45.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));
  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 81.0f),
                          glm::vec3(1.0f, 0.0f, 0.0f));

  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
  
  // Normal matrix: normal vectors to world coordinates
  normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
  glUniformMatrix3fv(normal_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  // Draw the cube
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // Tetrahedron
  model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -3.0f)); // Reset model_matrix for the tetrahedron
  model_matrix = glm::translate(model_matrix,
                             glm::vec3(sinf(2.1f * f) * 0.5f,
                                       cosf(1.7f * f) * 0.5f,
                                       sinf(1.3f * f) * cosf(1.5f * f) * 2.0f));
  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 45.0f),
                          glm::vec3(0.0f, 1.0f, 0.0f));
  model_matrix = glm::rotate(model_matrix,
                          glm::radians((float)currentTime * 81.0f),
                          glm::vec3(1.0f, 0.0f, 0.0f));

  glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_matrix));

  // Normal matrix: normal vectors to world coordinates
  normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
  glUniformMatrix3fv(normal_location, 1, GL_FALSE, glm::value_ptr(normal_matrix));

  // Draw tetrahedron
  glDrawArrays(GL_TRIANGLES, 36, 12);

  proj_matrix = glm::perspective(glm::radians(50.0f),
                                 (float) gl_width / (float) gl_height,
                                 0.1f, 1000.0f);
  glUniformMatrix4fv(proj_location, 1, GL_FALSE, glm::value_ptr(proj_matrix));
  
  view_matrix = glm::lookAt(                 camera_pos,  // pos
                            glm::vec3(0.0f, 0.0f, 0.0f),  // target
                            glm::vec3(0.0f, 1.0f, 0.0f)); // up
  glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
  
  // Camera position
  glUniform3fv(view_pos_location, 1, glm::value_ptr(camera_pos));

  // Set light data
  for (int i = 0; i < NUM_LIGHTS; i++) {
    glUniform3fv(light_position_location[i], 1, glm::value_ptr(light_positions[i]));
    glUniform3fv(light_ambient_location[i], 1, glm::value_ptr(light_ambient));
    glUniform3fv(light_diffuse_location[i], 1, glm::value_ptr(light_diffuse));
    glUniform3fv(light_specular_location[i], 1, glm::value_ptr(light_specular));
  }

  // Set material data
  glUniform1f(material_shininess_location, material_shininess);

  // Bind textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_diffuse_map);
  
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture_specular_map);
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

// Callback function to track window size and update viewport
void glfw_window_size_callback(GLFWwindow* window, int width, int height) {
  gl_width = width;
  gl_height = height;
  printf("New viewport: (width: %d, height: %d)\n", width, height);
}

unsigned int load_texture(const char* filename) {
  unsigned int texture;
  glGenTextures(1, &texture);

  // Load image, create texture and generate mipmaps
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1)
      format = GL_RED;
    else if (nrChannels == 3)
      format = GL_RGB;
    else if (nrChannels == 4)
      format = GL_RGBA;
    
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture wrapping and filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,  GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,  GL_LINEAR);
  } else {
    printf("ERROR: Failed to load texture\n");
  }

  // Free image after texture generation
  stbi_image_free(data);
  return texture;
}