#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <strings.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Container struct for shaders & programs.
typedef struct ShaderData {
  GLuint vertex_shader;
  GLuint fragment_shader;
  GLuint shader_program;
  GLboolean success;
} ShaderData;

void cleanup_shader(ShaderData *shader_data) {
  glDeleteShader(shader_data->vertex_shader);
  glDeleteShader(shader_data->fragment_shader);
  glDeleteProgram(shader_data->shader_program);
}

// Container struct for data associated with setup stage.
typedef struct RenderSetupData {
  GLuint VAO;
  GLuint VBO;
} RenderSetupData;

void cleanup_render_setup(RenderSetupData *render_setup) {
  glDeleteVertexArrays(1, &render_setup->VAO);
  glDeleteBuffers(1, &render_setup->VBO);
}

const ShaderData ShaderData_default = {0};
const RenderSetupData RenderSetupData_default = {0};

GLuint create_shader(GLenum type, const char source[], const int len) {
  GLuint shader = glCreateShader(type);
  GLchar const *files[] = {source};
  GLint lengths[] = {len};
  glShaderSource(shader, 1, files, lengths);
  glCompileShader(shader);

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (success == GL_FALSE) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    printf(infoLog);
    printf("\n");
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    glDeleteProgram(program);
    return 0;
  }

  return program;
}

const char vertex_shader_source[] =
    "#version 300 es"

    "// an attribute is an input (in) to a vertex shader."
    "// It will receive data from a buffer"
    "in vec4 a_position;"

    "// all shaders have a main function"
    "void main() {"
    "  // gl_Position is a special variable a vertex shader"
    "  // is responsible for setting"
    "  gl_Position = a_position;"
    "}";

const char fragment_shader_source[] =
    "#version 300 es"

    "// fragment shaders don't have a default precision so we need"
    "// to pick one. highp is a good default. It means \"high precision\""
    "precision highp float;"

    "// we need to declare an output for the fragment shader"
    "out vec4 outColor;"

    "void main() {"
    "  // Just set the output to a constant redish-purple"
    "  outColor = vec4(1, 0, 0.5, 1);"
    "}";

const int vert_len = sizeof(vertex_shader_source);
const int frag_len = sizeof(fragment_shader_source);

ShaderData setup_shader_program() {
  ShaderData result = {0};
  result.shader_program = glCreateProgram();
  result.vertex_shader =
      create_shader(GL_VERTEX_SHADER, vertex_shader_source, vert_len);
  result.fragment_shader =
      create_shader(GL_FRAGMENT_SHADER, fragment_shader_source, frag_len);

  if (result.vertex_shader == 0 || result.fragment_shader == 0) {
    return ShaderData_default;
  }

  glAttachShader(result.shader_program, result.vertex_shader);
  glAttachShader(result.shader_program, result.fragment_shader);
  glLinkProgram(result.shader_program);

  int success;
  char infoLog[512];
  glGetProgramiv(result.shader_program, GL_LINK_STATUS, &success);

  if (!success) {
    glGetProgramInfoLog(result.shader_program, 512, NULL, infoLog);
    printf("ERROR::SHADER::Program::COMPILATION_FAILED\n");
    printf(infoLog);
    cleanup_shader(&result);
    return ShaderData_default;
  }

  result.success = GL_TRUE;
  return result;
}

RenderSetupData setup_rendering() {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      -0.5f, -0.5f, 0.0f, // left
      0.5f,  -0.5f, 0.0f, // right
      0.0f,  0.5f,  0.0f  // top
  };

  // ..:: Initialization code (done once (unless your object frequently
  // changes)) :: ..
  // 1. bind Vertex Array Object
  RenderSetupData result;
  glGenVertexArrays(1, &result.VAO);
  glGenBuffers(1, &result.VBO);

  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(result.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, result.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // note that this is allowed, the call to glVertexAttribPointer registered VBO
  // as the vertex attribute's bound vertex buffer object so afterwards we can
  // safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't accidentally
  // modify this VAO, but this rarely happens. Modifying other VAOs requires a
  // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
  // VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  return result;
}

void render(ShaderData shader_data, GLuint VAO) {
  // Set a clear color.
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // 2. use our shader program when we want to render an object
  glUseProgram(shader_data.shader_program);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

int main(void) {
  GLFWwindow *window;

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // ^ Needed for Mac OS.

  /* Create a windowed mode window and its OpenGL context */
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "snake in c min", NULL,
                            NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  GLuint vert_shader =
      create_shader(GL_VERTEX_SHADER, vertex_shader_source, vert_len);
  GLuint frag_shader =
      create_shader(GL_FRAGMENT_SHADER, vertex_shader_source, vert_len);

  GLuint program = create_program(vert_shader, frag_shader);
  GLint posAttrib = glGetAttribLocation(program, "a_position");

  GLuint *buffers;
  glGenBuffers(1, buffers);
  GLuint buf = buffers[0];

  glBindBuffer(GL_ARRAY_BUFFER, buf);
  float positions[] = {
      0.0f, 0.0f, 0.0f, 0.5f, 0.7f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

  GLuint *arrays;
  glGenVertexArrays(1, arrays);
  GLuint vao = arrays[0];

  glBindVertexArray(vao);
  glEnableVertexAttribArray(posAttrib);

  GLint size = 2;                 // 2 components per iteration
  GLboolean normalize = GL_FALSE; // don't normalize the data
  GLsizei stride = 0;             // 0 = move forward size * sizeof(type) each
  glVertexAttribPointer(posAttrib, size, GL_FLOAT, normalize, stride,
                        positions);

  glUseProgram(program);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

/** glfw: whenever the window size changed (by OS or user resize) this callback
 * function executes. */
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

/** Process all input: query GLFW whether relevant keys are pressed/released
 * this frame and react accordingly. */
void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}
