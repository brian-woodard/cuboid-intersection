
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

#define WIDTH  800
#define HEIGHT 800

// NOTE: Uncomment the following line for GL error handling
//#define GL_DEBUG

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
   window = glfwCreateWindow(WIDTH, HEIGHT, "Cuboid Intersection", NULL, NULL);

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
   GLCALL(glClearColor(0.5, 0.5, 0.5, 1.0));

   // enable blending
   GLCALL(glEnable(GL_BLEND));
   GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

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

      GLCALL(glClear(GL_COLOR_BUFFER_BIT));

      // Start the Dear ImGui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::Begin("Cuboid Intersection");

      ImGui::BeginChild("Sphere (Bullet)", ImVec2(0, 110), ImGuiChildFlags_Borders);
      ImGui::SeparatorText("Sphere (Bullet) Location (North, East, -Up)");

      float pos[3];

      for (int i = 0; i < 3; i++)
         pos[i] = sphere.data[i];

      ImGui::InputFloat3("Position##Sphere", pos);
      ImGui::SliderFloat3("Position##SphereSlider", pos, -100.0f, 100.0f);

      for (int i = 0; i < 3; i++)
         sphere.data[i] = pos[i];
   
      if (ImGui::Button("Reset"))
      {
         sphere = C_vector(10.0, 0.0, 0.0);
      }

      ImGui::EndChild();

      ImGui::BeginChild("Cuboid (Entity)", ImVec2(0, 300), ImGuiChildFlags_Borders);
      ImGui::SeparatorText("Cuboid (Entity) Location (North, East, -Up)");

      for (int i = 0; i < 3; i++)
         pos[i]  = entity.m_vPosition.data[i];

      ImGui::InputFloat3("Position##Entity", pos);
      ImGui::SliderFloat3("Position##EntitySlider", pos, -100.0f, 100.0f);

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
      ImGui::SliderFloat3("Size##EntitySlider", size, -100.0f, 100.0f);

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

      ImGui::SeparatorText("Results (New)");
      int face = entity.SphereCollision(sphere, 0.0, miss_distance, poc);
      ImGui::Text("Face: %s (%d)", (face == -1) ? "None" : face_str[face-1], face);
      ImGui::Text("Miss Distance (m): %f", miss_distance);
      ImGui::Text("Point on Face: (%f, %f, %f)", poc.data[0], poc.data[1], poc.data[2]);

      ImGui::SeparatorText("Results (Old)");
      face = entity.SphereCollisionOld(sphere, 0.0, miss_distance, poc);
      ImGui::Text("Face: %s (%d)", (face == -1) ? "None" : face_str[face-1], face);
      ImGui::Text("Miss Distance (m): %f", miss_distance);
      ImGui::Text("Point on Face: (%f, %f, %f)", poc.data[0], poc.data[1], poc.data[2]);

      ImGui::EndChild();

      ImGui::End();

      // Render ImGui
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);

      // wait until next frame
      std::this_thread::sleep_until(frame_time);
      frame_time += framerate{1};
   }

   return 0;
}
