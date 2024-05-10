#include "vulkan_model.hpp"
#include "vulkan_utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
   template<>
   struct hash<lve::LveModel::Vertex> {
      size_t operator()(lve::LveModel::Vertex const &vertex) const {
         size_t seed = 0;
         lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
         return seed;
      }
   };
}

namespace lve {
   LveModel::LveModel(LveDevice &device, const LveModel::Builder &builder) : lveDevice{device} {
      createVertexBuffers(builder.vertices);
      createIndexBuffers(builder.indices);
   }

   LveModel::~LveModel() {
      vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
      vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

      if (hasIndexBuffer) {
         vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
         vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
      }
   }

   std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice &device, const std::string &filepath) {
      Builder builder{};
      builder.loadModel(filepath);

      std::cout << "Vertex count: " << builder.vertices.size() << "\n";
      return std::make_unique<LveModel>(device, builder);
   }

   //first stage buffer, then copy to local device memory

   void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
      vertexCount = static_cast<uint32_t>(vertices.size());
      assert(vertexCount >= 3 && "Vertex count must be at least 3");
      //total number of bytes required for vertex buffer to store all vertices
      VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      lveDevice.createBuffer(
         bufferSize, 
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         //host: CPU, device: GPU 
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
         stagingBuffer, 
         stagingBufferMemory);

      void* data;
      //creates region of host memory, maps to device memory, and sets data to point to beginning of mapped memory range
      vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
      memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
      vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

      lveDevice.createBuffer(
         bufferSize, 
         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         vertexBuffer, 
         vertexBufferMemory);

         //move contents of staging buffer to vertex buffer
      lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

      vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
      vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);

   }

   void LveModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
      indexCount = static_cast<uint32_t>(indices.size());
      //if a non empty vector of indices is provided, use index buffer for rendering model
      hasIndexBuffer = indexCount > 0;

      if (!hasIndexBuffer) return;
      VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

      VkBuffer stagingBuffer;
      VkDeviceMemory stagingBufferMemory;

      lveDevice.createBuffer(
         bufferSize, 
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         //host: CPU, device: GPU 
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
         stagingBuffer, 
         stagingBufferMemory);

      void* data;
      //creates region of host memory, maps to device memory, and sets data to point to beginning of mapped memory range
      vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
      memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
      vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

      lveDevice.createBuffer(
         bufferSize, 
         VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         indexBuffer, 
         indexBufferMemory);

         //move contents of staging buffer to vertex buffer
      lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

      vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
      vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
   }

   void LveModel::draw(VkCommandBuffer commandBuffer) {
      if (hasIndexBuffer) {
         //0 0 0 -> first index, vertex offset, first instance
         vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
      } else {
         vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
      }
   }

   void LveModel::bind(VkCommandBuffer commandBuffer) {
      VkBuffer buffers[] = {vertexBuffer};
      VkDeviceSize offsets[] = {0};
      //record to command buffer to bind 1 vertex buffer starting at binding 0
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

      if (hasIndexBuffer) {
         //for smaller models, can use 16 bit indices
          vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      }
   }

   std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
      std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
      bindingDescriptions[0].binding = 0;
      bindingDescriptions[0].stride = sizeof(Vertex);
      bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return bindingDescriptions;
   }

   std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

      attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
      attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
      attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
      attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

      return attributeDescriptions;
   }

   void LveModel::Builder::loadModel(const std::string &filepath) {
      //attrib stores pos, color, normal, texture coordinate data
      tinyobj::attrib_t attrib;
      //index values
      std::vector<tinyobj::shape_t> shapes;
      std::vector<tinyobj::material_t> materials;
      std::string warn, err;

      if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
         throw std::runtime_error(warn + err);
      }

      vertices.clear();
      indices.clear();

      //map that keeps track of the vertices that have already been added to builder.vertices vector, and store position at which the vertex was originally added
      std::unordered_map<Vertex, uint32_t> uniqueVertices{};
      for (const auto &shape : shapes) {
         for (const auto &index : shape.mesh.indices) {
            Vertex vertex{};

            //vertex index is first value of face element, says what position value to use. Optional, and negative if not present
            if (index.vertex_index >= 0) {
               vertex.position = {
                  //each vertex has 3 values. To read position, multiply by 3 and add 0 for initial component
                  attrib.vertices[3 * index.vertex_index + 0],
                  attrib.vertices[3 * index.vertex_index + 1],
                  attrib.vertices[3 * index.vertex_index + 2]
               };

               vertex.color = {
                  attrib.colors[3 * index.vertex_index + 0],
                  attrib.colors[3 * index.vertex_index + 1],
                  attrib.colors[3 * index.vertex_index + 2]
               };
            }

            if (index.normal_index >= 0) {
               vertex.normal = {
                  attrib.normals[3 * index.normal_index + 0],
                  attrib.normals[3 * index.normal_index + 1],
                  attrib.normals[3 * index.normal_index + 2]
               };
            }

            if (index.texcoord_index >= 0) {
               vertex.uv = {
                  attrib.texcoords[2 * index.texcoord_index + 0],
                  attrib.texcoords[2 * index.texcoord_index + 1],
               };
            }

            if (uniqueVertices.count(vertex) == 0) {
               uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
               vertices.push_back(vertex);
            }
            
            //add position of vertex to indices vector
            indices.push_back(uniqueVertices[vertex]);
         }
      }
   }
}