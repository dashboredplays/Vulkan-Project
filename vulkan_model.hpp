#pragma once

#include "vulkan_device.hpp"

//angles in radians, NOT degrees
#define GLM_FORCE_RADIANS
//depth values between 0 and 1 (not -1 to 1 like opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

namespace lve {
   class LveModel {
      public:

      struct Vertex {
         //we are going to interleave the color attribute with the position attribute
         glm::vec3 position{};
         glm::vec3 color{};
         glm::vec3 normal{};
         //2 dimensional texture coordinates
         glm::vec2 uv{};

         static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
         static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

         bool operator==(const Vertex &other) const {
            return position == other.position && color == other.color && uv == other.uv;
         }
      };

      struct Builder {
         //temporary helper object, storing vertex and index info until it can be copied over into the object's vertex and index buffer memory
         std::vector<Vertex> vertices{};
         std::vector<uint32_t> indices{};

         void loadModel(const std::string &filepath);
      };

      LveModel(LveDevice &device, const LveModel::Builder &builder);
      ~LveModel();

      LveModel(const LveModel&) = delete;
      LveModel& operator=(const LveModel &) = delete;

      static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath);

      void bind(VkCommandBuffer commandBuffer);
      void draw(VkCommandBuffer commandBuffer);

      private:

      void createVertexBuffers(const std::vector<Vertex>& vertices);
      void createIndexBuffers(const std::vector<uint32_t>& indices);
      //device reference
      LveDevice& lveDevice;
      //note these are 2 separate objects: in control of memory management
      VkBuffer vertexBuffer;
      VkDeviceMemory vertexBufferMemory;
      uint32_t vertexCount;

      bool hasIndexBuffer = false;
      VkBuffer indexBuffer;
      VkDeviceMemory indexBufferMemory;
      uint32_t indexCount;
   };
}