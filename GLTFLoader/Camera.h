#pragma once
#ifndef CCAMERA_H
#define CCAMERA_H

#include <cmath>
#include "windows.h"
#include "System.h"
#include "PersonalGL.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class CCamera {
public:
    glm::vec3 Pos, View, Up, Strafe;
    glm::vec3 previousPosition, previousView;
    float currentRotationAngle;
    float frameInterval;

    CCamera();

    void SetCamera(float x, float y, float z, float xv, float yv, float zv, float xu, float yu, float zu);
    void MoveCamera(float direction);
    void StrafeCam(float direction);
    void MoveAndStrafe(float moveDirection, float strafeDirection);
    void UpdateCamera(float x, float y, float z, float dir);
    void CalculateStrafe();
    void RotateCamera(float AngleDir, float xSpeed, float ySpeed, float zSpeed);
    void RotateByMouse(int mousePosX, int mousePosY, int midX, int midY);
    void CalculateTime();
    float getFieldOfView() const;
    float getAspectRatio() const;
    float getNearPlane() const;
    float getFarPlane() const;
    glm::vec3 GetPosition() const;
    void RevertPosition();
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    void setProjectionMatrix(float fov, float aspectRatio, float nearPlane, float farPlane);

private:
    float fov, aspectRatio, nearPlane, farPlane = 0;
};
#endif