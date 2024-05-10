#pragma once

#include "vulkan_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace lve {
   //better way is to use entity component system as opposed to OOP. https://www.kodeco.com/2806-introduction-to-component-based-architecture-in-games

   struct TransformComponent {
      //the position offset of the object
      glm::vec3 translation{};
      //scale transformation of the object
      glm::vec3 scale{1.f, 1.f, 1.f};
      //rotation of the object
      glm::vec3 rotation{};

      glm::mat4 mat4();
      glm::mat3 normalMatrix();
      //extrinsic rotation is when axeses of x, y, and z are fixed in the world space.
      //intrinsic rotation is when the axeses are fixed to the object. (yaw, pitch, roll)
      //example: as an extrinsic rotation, this rotation transformation Y(1), X(2), Z(3) can be read from right to left, while intrinsic rotation can be read from left to right.
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
      TransformComponent transform{};

      private:
      LveGameObject(id_t objId) : id(objId) {}
      id_t id;
   };
}