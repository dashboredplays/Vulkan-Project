#include "game_object.hpp"

namespace lve {

   // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
   // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
   // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
   glm::mat4 TransformComponent::mat4() {
      //in 3d, rotation becomes much more complex. You can use either euler angles or quaternions.
      //for proper euler angles, 1st and 3rd angles use the same axis.
      //for tait-bryan angles, all angles use distinct axes.
      //also exists axis angles, which is a single angle around a single axis.

      //the object is originally at the origin in object space. This matrix moves the object into the shared world space. 
      //Every object then has the camera transform applied to it (moving it to camera space).
      //a camera object doesn't actually exist in the scene. It is a transform that is applied to every object in the scene.
      //finally, apply the projection matrix, which captures everything that is in the viewing furstum, and transforms it into the canonical viewing volume.
      //the viewport transform then converts it to the actual pixel volume.
      const float c3 = glm::cos(rotation.z);
      const float s3 = glm::sin(rotation.z);
      const float c2 = glm::cos(rotation.x);
      const float s2 = glm::sin(rotation.x);
      const float c1 = glm::cos(rotation.y);
      const float s1 = glm::sin(rotation.y);
      return glm::mat4{
         {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
         },
         {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
         },
         {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
         },
         {translation.x, translation.y, translation.z, 1.0f}};
   }

   glm::mat3 TransformComponent::normalMatrix() {
      const float c3 = glm::cos(rotation.z);
      const float s3 = glm::sin(rotation.z);
      const float c2 = glm::cos(rotation.x);
      const float s2 = glm::sin(rotation.x);
      const float c1 = glm::cos(rotation.y);
      const float s1 = glm::sin(rotation.y);
      const glm::vec3 invScale = 1.0f / scale;

      return glm::mat3{
         {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
         },
         {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
         },
         {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
         },
      };  
   }
}