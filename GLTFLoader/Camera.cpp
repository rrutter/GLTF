#include "Camera.h"
#include <GL/freeglut_std.h>

extern COMP_SYSTEM SYS;

CCamera::CCamera()
    : currentRotationAngle(0.0f), frameInterval(0.0f) {
    Pos = glm::vec3(0.0f);
    View = glm::vec3(0.0f);
    Up = glm::vec3(0.0f);
    Strafe = glm::vec3(0.0f);
}

void CCamera::SetCamera(float x, float y, float z, float xv, float yv, float zv, float xu, float yu, float zu) {
    Pos = glm::vec3(x, y, z);
    View = glm::vec3(xv, yv, zv);
    Up = glm::vec3(xu, yu, zu);
    setProjectionMatrix(45.0f, (float)SYS.COMP_SETTINGS.getScreenWidth() / SYS.COMP_SETTINGS.getScreenHeight(), SYS.COMP_SETTINGS.ScreenNear, SYS.COMP_SETTINGS.ScreenFar);
}

void CCamera::MoveCamera(float direction) {
    glm::vec3 LookDirection = glm::normalize(View - Pos);
    UpdateCamera(LookDirection.x, LookDirection.y, LookDirection.z, direction);
}

void CCamera::StrafeCam(float direction) {
    CalculateStrafe();
    UpdateCamera(Strafe.x, Strafe.y, Strafe.z, direction);
}

void CCamera::MoveAndStrafe(float moveDirection, float strafeDirection) {
    glm::vec3 LookDirection = glm::normalize(View - Pos);
    CalculateStrafe();
    glm::vec3 combinedDirection = LookDirection * moveDirection + Strafe * strafeDirection;
    UpdateCamera(combinedDirection.x, combinedDirection.y, combinedDirection.z, 1.0f);
}

void CCamera::UpdateCamera(float x, float y, float z, float dir) {
    dir = dir * (SYS.COMP_SETTINGS.cameraSpeed);
    previousPosition = Pos;
    previousView = View;

    Pos += glm::vec3(x, y, z) * dir;
    View += glm::vec3(x, y, z) * dir;
}

void CCamera::CalculateStrafe() {
    glm::vec3 Dir = glm::normalize(View - Pos);
    Strafe = glm::normalize(glm::cross(Dir, Up));
}

void CCamera::RotateCamera(float AngleDir, float xSpeed, float ySpeed, float zSpeed) {
    glm::quat qRotation = glm::angleAxis(glm::radians(AngleDir), glm::vec3(xSpeed, ySpeed, zSpeed));
    glm::vec3 LookDirection = View - Pos;
    glm::vec3 NewView = qRotation * LookDirection;
    View = Pos + NewView;
}

void CCamera::RotateByMouse(int mousePosX, int mousePosY, int midX, int midY) {
    float yDirection = 0.0f;
    float yRotation = 0.0f;

    if ((mousePosX == midX) && (mousePosY == midY))
        return;

    yDirection = (float)((midX - mousePosX)) / SYS.COMP_SETTINGS.MouseSensitivity;
    yRotation = (float)((midY - mousePosY)) / SYS.COMP_SETTINGS.MouseSensitivity;

    currentRotationAngle -= yRotation;

    if (currentRotationAngle > 30.0f) {
        currentRotationAngle = 30.0f;
        return;
    }

    if (currentRotationAngle < -30.0f) {
        currentRotationAngle = -30.0f;
        return;
    }

    glm::vec3 Axis = glm::normalize(glm::cross(View - Pos, Up));
    RotateCamera(yRotation, Axis.x, Axis.y, Axis.z);
    RotateCamera(yDirection, 0, 1, 0);
}

void CCamera::CalculateTime() {
    static float lastFrameTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    frameInterval = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
}

float CCamera::getFieldOfView() const
{
    return fov;
}

float CCamera::getAspectRatio() const
{
    return aspectRatio;
}

float CCamera::getNearPlane() const
{
    return nearPlane;
}

float CCamera::getFarPlane() const
{
    return farPlane;
}

glm::vec3 CCamera::GetPosition() const {
    return Pos;
}

void CCamera::RevertPosition() {
    Pos = previousPosition;
    View = previousView;
}

glm::mat4 CCamera::getViewMatrix() const {
    return glm::lookAt(Pos, View, Up);
}

glm::mat4 CCamera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void CCamera::setProjectionMatrix(float fieldOfView, float aspect, float nearP, float farP) {
    fov = fieldOfView;
    aspectRatio = aspect;
    nearPlane = nearP;
    farPlane = farP;
}
