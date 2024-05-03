#pragma once

#include "vulkan_device.hpp"

//angles in radians, NOT degrees
#define GLM_FORCE_RADIANS
//depth values between 0 and 1 (not -1 to 1 like opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace lve {
   class LveModel {
      public:

      struct Vertex {
         //we are going to interleave the color attribute with the position attribute
         glm::vec3 position{};
         glm::vec3 color{};

         static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
         static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
      };

         LveModel(LveDevice &device, const std::vector<Vertex> &vertices);
         ~LveModel();

         LveModel(const LveModel&) = delete;
         LveModel& operator=(const LveModel &) = delete;
         LveModel() = default;

         void bind(VkCommandBuffer commandBuffer);
         void draw(VkCommandBuffer commandBuffer);

      private:

      void createVertexBuffers(const std::vector<Vertex>& vertices);
      //device reference
         LveDevice& lveDevice;
         //note these are 2 separate objects: in control of memory management
         VkBuffer vertexBuffer;
         VkDeviceMemory vertexBufferMemory;
         uint32_t vertexCount;
   };
}