#pragma once

#define GLM_FORCE_RADIANS
//depth values between 0 and 1 (not -1 to 1 like opengl)
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {

   class LveCamera {
      public:
      void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);
      //vertical field of view, aspect ratio, near and far clipping planes
      void setPerspectiveProjection(float fovy, float aspect, float near, float far);

      //do i set it to -1.f or 1.f?
      void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
      //when you want to have camera locked onto target
      void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
      //use euler angles to specify orientation of camera
      void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

      const glm::mat4& getProjection() const {
         return projectionMatrix;
      }

      const glm::mat4& getView() const {
         return viewMatrix;
      }

      private:
      //{1.f} is identity matrix
      glm::mat4 projectionMatrix{1.f};
      glm::mat4 viewMatrix{1.f};
   };
} 