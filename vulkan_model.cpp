#include "vulkan_model.hpp"

#include <cassert>
#include <cstring>
namespace lve {
    LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : lveDevice{device} {
        createVertexBuffers(vertices);
    }

    LveModel::~LveModel() {
        vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        //total number of bytes required for vertex buffer to store all vertices
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        lveDevice.createBuffer(
            bufferSize, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            //host: CPU, device: GPU 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            vertexBuffer, 
            vertexBufferMemory);

        void* data;
        //creates region of host memory, maps to device memory, and sets data to point to beginning of mapped memory range
        vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(lveDevice.device(), vertexBufferMemory);
    }

    void LveModel::draw(VkCommandBuffer commandBuffer) {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void LveModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        //record to command buffer to bind 1 vertex buffer starting at binding 0
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

        attributeDescriptions[0].binding = 0;
        //location specified in vertex shader
        attributeDescriptions[0].location = 0;
        //data type: 2 components, each 32 bit floats
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1; //location used in vertex shader
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; //vec3
        //automatically calculates offset of color attribute
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        //you can also use this form, where the order is: location, binding, format, offset ->
        // std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        //     {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, position)},
        //     {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
        // };

        return attributeDescriptions;
    }
}