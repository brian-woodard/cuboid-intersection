// main.cpp
#include <stdio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "CommonTypes.h"
#include "FileUtils.cpp"
#include "PrintData.cpp"
#include "Vector.cpp"
#include "Cuboid.cpp"

#define WIDTH  1600
#define HEIGHT 800

#ifdef GL_DEBUG
#define GLCALL(function) \
   { \
      GLenum error = GL_INVALID_ENUM; \
      while (error != GL_NO_ERROR) \
      { \
         error = glGetError(); \
      } \
      function; \
      error = glGetError(); \
      if (error != GL_NO_ERROR) \
      { \
         fprintf(stderr, "OpenGL Error: GL_ENUM(%d) at %s:%d\n", error, __FILE__, __LINE__); \
      } \
   }
#else
#define GLCALL(function) function;
#endif

const char* face_str[] =
{
   "Front",
   "Right",
   "Top",
   "Left",
   "Bottom",
   "Back"
};

// Simple vertex/fragment shader for MVP
static const char* vs_src = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* fs_src = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;
void main()
{
    FragColor = uColor;
}
)";

static GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char buf[1024];
        glGetShaderInfoLog(s, sizeof(buf), NULL, buf);
        fprintf(stderr, "Shader compile error: %s\n", buf);
    }
    return s;
}

static GLuint create_program(const char* vs, const char* fs)
{
    GLuint vs_s = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint fs_s = compile_shader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs_s);
    glAttachShader(prog, fs_s);
    glLinkProgram(prog);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char buf[1024];
        glGetProgramInfoLog(prog, sizeof(buf), NULL, buf);
        fprintf(stderr, "Program link error: %s\n", buf);
    }
    glDeleteShader(vs_s);
    glDeleteShader(fs_s);
    return prog;
}

// Build unit-cube (centered at origin) line vertices for wireframe
// We'll scale this in model matrix to get cuboid size.
void buildUnitCubeLines(std::vector<glm::vec3>& outVerts, std::vector<unsigned int>& outIdx)
{
    // 8 cube corners
    outVerts = {
        {-0.5f, -0.5f, -0.5f}, // 0
        { 0.5f, -0.5f, -0.5f}, // 1
        { 0.5f,  0.5f, -0.5f}, // 2
        {-0.5f,  0.5f, -0.5f}, // 3
        {-0.5f, -0.5f,  0.5f}, // 4
        { 0.5f, -0.5f,  0.5f}, // 5
        { 0.5f,  0.5f,  0.5f}, // 6
        {-0.5f,  0.5f,  0.5f}  // 7
    };

    // lines as pairs of indices (12 edges -> 24 indices)
    unsigned int idxs[] = {
        0,1, 1,2, 2,3, 3,0, // bottom ring (z=-0.5)
        4,5, 5,6, 6,7, 7,4, // top ring (z=+0.5)
        0,4, 1,5, 2,6, 3,7  // vertical edges
    };
    outIdx.assign(idxs, idxs + sizeof(idxs)/sizeof(unsigned int));
}

int main(int argc, char* argv[])
{
   GLFWwindow* window = nullptr;

   C_vector sphere(10.0, 0.0, 0.0);
   C_cuboid entity(C_vector(0.0), 2.0, 2.0, 2.0);
   float    entity_hdg = 0.0;

   // initialize glfw
   if (!glfwInit())
      return 0;

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   // Create window
   window = glfwCreateWindow(WIDTH, HEIGHT, "Cuboid Intersection - 3D View", NULL, NULL);

   if (!window)
   {
      glfwTerminate();
      return 0;
   }

   // make the window's context current
   glfwMakeContextCurrent(window);

   // use glad to load OpenGL function pointers
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      printf("Error: Failed to initialize GLAD.\n");
      glfwTerminate();
      return 0;
   }

   glfwSetWindowSize(window, WIDTH, HEIGHT);

   // Setup Dear ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui::StyleColorsDark();

   // Setup Platform/Render backends
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init("#version 330");

   // Make the window visible
   glfwShowWindow(window);

   // Initialize opengl
   GLCALL(glClearColor(0.15f, 0.15f, 0.15f, 1.0f));

   // enable blending and depth test
   GLCALL(glEnable(GL_BLEND));
   GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
   GLCALL(glEnable(GL_DEPTH_TEST));

   // Create shader program
   GLuint program = create_program(vs_src, fs_src);
   GLint loc_uMVP = glGetUniformLocation(program, "uMVP");
   GLint loc_uColor = glGetUniformLocation(program, "uColor");

   // Build unit cube wireframe geometry
   std::vector<glm::vec3> unitVerts;
   std::vector<unsigned int> unitIdx;
   buildUnitCubeLines(unitVerts, unitIdx);

   GLuint cubeVBO = 0, cubeVAO = 0, cubeEBO = 0;
   GLCALL(glGenVertexArrays(1, &cubeVAO));
   GLCALL(glGenBuffers(1, &cubeVBO));
   GLCALL(glGenBuffers(1, &cubeEBO));

   GLCALL(glBindVertexArray(cubeVAO));
   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, cubeVBO));
   GLCALL(glBufferData(GL_ARRAY_BUFFER, unitVerts.size() * sizeof(glm::vec3), unitVerts.data(), GL_STATIC_DRAW));

   GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO));
   GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, unitIdx.size() * sizeof(unsigned int), unitIdx.data(), GL_STATIC_DRAW));

   GLCALL(glEnableVertexAttribArray(0));
   GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0));

   GLCALL(glBindVertexArray(0));

   // Setup a simple VBO/VAO for rendering the sphere position as a GL_POINT
   GLuint pointVAO = 0, pointVBO = 0;
   GLCALL(glGenVertexArrays(1, &pointVAO));
   GLCALL(glGenBuffers(1, &pointVBO));
   GLCALL(glBindVertexArray(pointVAO));
   GLCALL(glBindBuffer(GL_ARRAY_BUFFER, pointVBO));
   // allocate once, will update per-frame with glBufferSubData
   GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW));
   GLCALL(glEnableVertexAttribArray(0));
   GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0));
   GLCALL(glBindVertexArray(0));

   GLuint lineVAO = 0, lineVBO = 0;
   glGenVertexArrays(1, &lineVAO);
   glGenBuffers(1, &lineVBO);
   glBindVertexArray(lineVAO);
   glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, NULL, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
   glBindVertexArray(0);

   GLuint faceVAO = 0, faceVBO = 0;
   glGenVertexArrays(1, &faceVAO);
   glGenBuffers(1, &faceVBO);
   glBindVertexArray(faceVAO);
   glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, NULL, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
   glBindVertexArray(0);

   // Camera setup: cameraPos is arbitrary but must be looking at origin
   // This satisfies "camera pointed at the origin".
   glm::vec3 cameraPos = glm::vec3(0.0f, -20.0f, 10.0f);
   glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

   // Projection matrix
   glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

   // set frame rate to 60 Hz
   using framerate = std::chrono::duration<double, std::ratio<1, 60>>;
   auto frame_time = std::chrono::high_resolution_clock::now() + framerate{1};

   while (window)
   {
      // Poll events
      glfwPollEvents();

      if (glfwWindowShouldClose(window))
      {
         glfwTerminate();
         window = nullptr;
         break;
      }

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // ---------- ImGui UI (unchanged, minus slight ordering to render 3D afterwards) ----------
      ImGui::Begin("Cuboid Intersection");

      ImGui::BeginChild("Sphere (Bullet)", ImVec2(0, 110), ImGuiChildFlags_Borders);
      ImGui::SeparatorText("Sphere (Bullet) Location (East, North, Up)");

      float pos[3];

      for (int i = 0; i < 3; i++)
         pos[i] = sphere.data[i];

      ImGui::InputFloat3("Position##Sphere", pos);
      ImGui::SliderFloat3("Position##SphereSlider", pos, -35.0f, 35.0f);

      for (int i = 0; i < 3; i++)
         sphere.data[i] = pos[i];

      if (ImGui::Button("Reset"))
      {
         sphere = C_vector(10.0, 0.0, 0.0);
      }

      ImGui::EndChild();

      ImGui::BeginChild("Cuboid (Entity)", ImVec2(0, 300), ImGuiChildFlags_Borders);
      ImGui::SeparatorText("Cuboid (Entity) Location (East, North, Up)");

      for (int i = 0; i < 3; i++)
         pos[i]  = entity.m_vPosition.data[i];

      ImGui::InputFloat3("Position##Entity", pos);
      ImGui::SliderFloat3("Position##EntitySlider", pos, -10.0f, 10.0f);

      for (int i = 0; i < 3; i++)
         entity.m_vPosition.data[i] = pos[i];

      if (ImGui::Button("Reset"))
      {
         entity.SetPosition(0.0, 0.0, 0.0);
      }

      ImGui::SeparatorText("Cuboid (Entity) Size (Depth, Width, Height)");

      float size[3];

      for (int i = 0; i < 3; i++)
         size[i] = entity.m_pSize[i];

      ImGui::InputFloat3("Size##Entity", size);
      ImGui::SliderFloat3("Size##EntitySlider", size, 0.0f, 100.0f);

      for (int i = 0; i < 3; i++)
         entity.m_pSize[i] = size[i];

      if (ImGui::Button("Reset##Size"))
      {
         entity.SetDepth(2.0);
         entity.SetWidth(2.0);
         entity.SetHeight(2.0);
      }

      ImGui::SeparatorText("Cuboid (Entity) Orientation (Heading, Pitch, Roll)");

      ImGui::InputFloat("Heading##Entity", &entity_hdg);
      ImGui::SliderFloat("Heading##EntitySlider", &entity_hdg, -180.0f, 180.0f);

      if (ImGui::Button("Reset##Heading"))
      {
         entity_hdg = 0.0;
      }

      ImGui::EndChild();

      // Run cuboid intersection stuff here
      entity.SetYaw_D(entity_hdg);

      C_vector poc;
      double   miss_distance;

      ImGui::BeginChild("Results", ImVec2(0, 0), ImGuiChildFlags_Borders);

      ImGui::SeparatorText("Results (Old)");
      int face = entity.SphereCollisionOld(sphere, 0.0, miss_distance, poc);
      ImGui::Text("Face: %s (%d)", (face == -1) ? "None" : face_str[face-1], face);
      ImGui::Text("Miss Distance (m): %f", miss_distance);
      ImGui::Text("Point on Face: (%f, %f, %f)", poc.data[0], poc.data[1], poc.data[2]);

      ImGui::SeparatorText("Results (New)");
      face = entity.SphereCollision(sphere, 0.0, miss_distance, poc);
      ImGui::Text("Face: %s (%d)", (face == -1) ? "None" : face_str[face-1], face);
      ImGui::Text("Miss Distance (m): %f", miss_distance);
      ImGui::Text("Point on Face: (%f, %f, %f)", poc.data[0], poc.data[1], poc.data[2]);

      ImGui::EndChild();

      ImGui::End();

      // ---------- 3D rendering ----------
      GLCALL(glViewport(0, 0, WIDTH, HEIGHT));
      GLCALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

      // Build view matrix (camera looking at origin)
      glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

      // ---------------------- Draw line from sphere → cuboid center ----------------------
      glm::vec3 spherePos(sphere.data[0], sphere.data[1], sphere.data[2]);
      glm::vec3 centerPos(entity.m_vPosition.data[0], 
                           entity.m_vPosition.data[1], 
                           entity.m_vPosition.data[2]);

      glm::mat4 mvpLine = projection * view * glm::mat4(1.0f);

      glUseProgram(program);
      glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvpLine[0][0]);
      glUniform4f(loc_uColor, 1.0f, 1.0f, 0.0f, 1.0f);  // yellow line

      glm::vec3 linePts[2] = { spherePos, centerPos };

      glBindVertexArray(lineVAO);
      glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(linePts), linePts);
      glDrawArrays(GL_LINES, 0, 2);


      // ---------------------- Draw intersection point ----------------------
      if (face != -1)
      {
         glm::vec3 pocPos(poc.data[0], poc.data[1], poc.data[2]);

         glm::mat4 mvpPoc = projection * view * glm::mat4(1.0f);

         glUseProgram(program);
         glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvpPoc[0][0]);
         glUniform4f(loc_uColor, 1.0f, 0.47f, 0.0f, 1.0f);  // orange point

         glPointSize(10.0f);

         glBindVertexArray(pointVAO);
         glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
         glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &pocPos);
         glDrawArrays(GL_POINTS, 0, 1);
      }

      // Draw cuboid wireframe
      GLCALL(glUseProgram(program));

      // Model for entity: translate to position, rotate yaw (heading), scale to cuboid size
      glm::mat4 model = glm::mat4(1.0f);
      // Note: C_cuboid stores m_vPosition and m_pSize; adjust order/axes as needed for your coordinate system
      model = glm::translate(model, glm::vec3(entity.m_vPosition.data[0], entity.m_vPosition.data[1], entity.m_vPosition.data[2]));
      // entity.SetYaw_D uses degrees; rotate about Z axis (assuming heading yaw around Z)
      model = glm::rotate(model, glm::radians(-entity_hdg), glm::vec3(0.0f, 0.0f, 1.0f));
      // sizes: Depth, Width, Height (your code uses m_pSize[0..2] - adapt if different)
      model = glm::scale(model, glm::vec3(entity.m_pSize[0], entity.m_pSize[1], entity.m_pSize[2]));

      glm::mat4 mvp = projection * view * model;
      GLCALL(glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvp[0][0]));
      // wireframe: dark blue
      GLCALL(glUniform4f(loc_uColor, 0.0f, 0.2f, 0.8f, 1.0f));

      // Draw lines
      GLCALL(glBindVertexArray(cubeVAO));
      GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)); // ensure wireframe
      // Because we put edges in EBO as pairs, draw with GL_LINES
      GLCALL(glDrawElements(GL_LINES, (GLsizei)unitIdx.size(), GL_UNSIGNED_INT, 0));
      GLCALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
      GLCALL(glBindVertexArray(0));

      // Draw sphere position as a point (in world space)
      GLCALL(glBindVertexArray(pointVAO));
      GLCALL(glBindBuffer(GL_ARRAY_BUFFER, pointVBO));
      GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &spherePos)); // update point position

      glm::mat4 modelPoint = glm::translate(glm::mat4(1.0f), spherePos);
      glm::mat4 mvpPoint = projection * view * glm::mat4(1.0f);
      GLCALL(glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvpPoint[0][0]));
      GLCALL(glUniform4f(loc_uColor, 0.8f, 0.1f, 0.1f, 1.0f));
      GLCALL(glPointSize(8.0f));
      GLCALL(glDrawArrays(GL_POINTS, 0, 1));
      GLCALL(glBindVertexArray(0));

      // Draw cuboid position as a point (in world space)
      GLCALL(glBindVertexArray(pointVAO));
      GLCALL(glBindBuffer(GL_ARRAY_BUFFER, pointVBO));
      GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), &centerPos)); // update point position

      modelPoint = glm::translate(glm::mat4(1.0f), centerPos);
      mvpPoint = projection * view * glm::mat4(1.0f);
      GLCALL(glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvpPoint[0][0]));
      GLCALL(glUniform4f(loc_uColor, 0.8f, 0.1f, 0.1f, 1.0f));
      GLCALL(glPointSize(8.0f));
      GLCALL(glDrawArrays(GL_POINTS, 0, 1));
      GLCALL(glBindVertexArray(0));

      // Unbind shader
      GLCALL(glUseProgram(0));

      if (face != -1)
      {
         // Get the face defining corners via C_cuboid helper
         C_vector c0, c1, c2, c3;
         entity.GetFaceCorners(face, c0, c1, c2, c3); // YOU implement or already have this

         glm::vec3 quadVerts[4] = {
            { c0.data[0], c0.data[1], c0.data[2] },
            { c1.data[0], c1.data[1], c1.data[2] },
            { c2.data[0], c2.data[1], c2.data[2] },
            { c3.data[0], c3.data[1], c3.data[2] }
         };

         glm::mat4 mvpFace = projection * view * glm::mat4(1.0f);

         glUseProgram(program);
         glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, &mvpFace[0][0]);
         glUniform4f(loc_uColor, 0.0f, 0.8f, 1.0f, 0.3f);  // cyan-ish, transparent

         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

         glBindVertexArray(faceVAO);
         glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
         glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVerts), quadVerts);

         // Draw 2 triangles
         glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
      }

      // Render ImGui on top
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);

      // wait until next frame
      std::this_thread::sleep_until(frame_time);
      frame_time += framerate{1};
   }

   // cleanup
   glDeleteBuffers(1, &cubeVBO);
   glDeleteBuffers(1, &cubeEBO);
   glDeleteVertexArrays(1, &cubeVAO);
   glDeleteBuffers(1, &pointVBO);
   glDeleteVertexArrays(1, &pointVAO);
   glDeleteProgram(program);

   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();

   return 0;
}

