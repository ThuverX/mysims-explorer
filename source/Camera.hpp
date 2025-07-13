#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class Camera {
private:
    glm::vec3 target;
    float distance;
    float yaw;
    float pitch;

    glm::vec3 position;
    glm::vec3 up;

    void Update();

public:
    Camera(glm::vec3 target = glm::vec3(0.0f), float distance = 3.f, float yaw = -90.f, float pitch = 0.f)
        : target(target), distance(distance), yaw(yaw), pitch(pitch) {
            Update();
        }

    void Orbit(float deltaYaw, float deltaPitch);
    void Zoom(float deltaZoom);
    void Pan(float deltaX, float deltaY);

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(const glm::vec2 viewport) const;

    inline glm::vec3 GetPosition() const {
        return position;
    }

    inline glm::vec3 GetTarget() const {
        return target;
    }
};
