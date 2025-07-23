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
    Camera() {
        Reset();
    }

    void Reset() {
        target = glm::vec3(0.f);
        distance = 3.f;
        yaw = 90.f;
        pitch = 0.f;
        Update();
    }

    void SetTarget(const glm::vec3 &target);

    void Orbit(float deltaYaw, float deltaPitch);
    void Zoom(float deltaZoom);
    void Pan(float deltaX, float deltaY);

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    static glm::mat4 GetProjectionMatrix(const glm::vec2 &viewport);

    [[nodiscard]] inline glm::vec3 GetPosition() const {
        return position;
    }

    [[nodiscard]] inline glm::vec3 GetTarget() const {
        return target;
    }
};
