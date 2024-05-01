#pragma once

#include "vulkan_model.hpp"

#include <memory>

namespace lve {
   //better way is to use entity component system as opposed to OOP. https://www.kodeco.com/2806-introduction-to-component-based-architecture-in-games

   struct Transform2dComponent {
      //the position offset of the object
      glm::vec2 translation{};
      //scale transformation of the object
      glm::vec2 scale{1.f, 1.f};
      //rotation of the object
      float rotation;

      glm::mat2 mat2() { 
         //rotation matrix
         const float s = glm::sin(rotation);
         const float c = glm::cos(rotation);
         glm::mat2 rotMatrix{{c, -s}, {s, c}};
         //each entry is a column vector, NOT ROW VECTOR
         glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
         //we can combine several transformations into one matrix
         //this first scales the object, then rotates it. Remember the order
         return rotMatrix * scaleMat;
      }
   };

   class LveGameObject {
      public:
      using id_t = unsigned int;

      static LveGameObject createGameObject() {
         static id_t currentId = 0;
         return LveGameObject{currentId++};
      }

      LveGameObject(const LveGameObject&) = delete;
      LveGameObject& operator=(const LveGameObject&) = delete;
      LveGameObject(LveGameObject&&) = default;
      LveGameObject& operator=(LveGameObject&&) = default;

      id_t getId() const { return id; }

      std::shared_ptr<LveModel> model{};
      glm::vec3 color{};
      Transform2dComponent transform2d{};

      private:
      LveGameObject(id_t objId) : id(objId) {}
      id_t id;
   };
}