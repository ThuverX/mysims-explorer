#include "Camera.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Camera::Update() {
    float radYaw   = glm::radians(yaw);
    float radPitch = glm::radians(pitch);

    glm::vec3 offset;
    offset.x = distance * cos(radPitch) * cos(radYaw);
    offset.y = distance * sin(radPitch);
    offset.z = distance * cos(radPitch) * sin(radYaw);

    position = target + offset;
    up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::SetTarget(const glm::vec3 &target) {
    this->target = target;
}

void Camera::Orbit(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch += deltaPitch;
    pitch = std::clamp(pitch, -89.9f, 89.8f);
    Update();
}

void Camera::Zoom(float deltaZoom) {
    distance -= deltaZoom;
    distance = std::clamp(distance, 0.1f, 1000.0f);
    Update();
}

void Camera::Pan(float deltaX, float deltaY) {
    glm::vec3 right = glm::normalize(glm::cross(target - position, up));
    glm::vec3 upDir = glm::normalize(glm::cross(right, target - position));

    target += -right * deltaX + upDir * deltaY;
    Update();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::GetProjectionMatrix(const glm::vec2 &viewport) {
    return glm::perspective(glm::radians(45.0f), viewport.x / viewport.y, 0.1f, 100.0f);
}
