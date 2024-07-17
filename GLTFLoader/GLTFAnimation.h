#ifndef GLTFANIMATION_H
#define GLTFANIMATION_H

#include <string>
#include <vector>
#include "yyjson.h"
#include <glm/fwd.hpp>
#include "GLTFNode.h"
#include "GLTFSkeleton.h"
#include <algorithm> // any_of

class GLTFAnimation {
public:
    struct Channel {
        int sampler;
        int targetNode;
        std::string targetPath;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    struct Sampler {
        int input;
        int output;
        std::string interpolation;
        yyjson_val* extensions;
        yyjson_val* extras;
        std::vector<float> inputTimes;
        std::vector<glm::vec3> outputValuesVec3;
        std::vector<glm::quat> outputValuesQuat;
    };

    struct Animation {
        std::string name;
        std::vector<Channel> channels;
        std::vector<Sampler> samplers;
        yyjson_val* extensions;
        yyjson_val* extras;
    };

    //void parseAnimations(yyjson_val* animationsArray);
    void parseAnimations(yyjson_val* animationsArray, const GLTFAccessor& accessorManager, const GLTFBuffer& bufferManager);
    size_t getAnimationCount() const;
    const std::vector<Animation>& getAnimations() const;

    void updateAnimation(float deltaTime, GLTFNode& nodeManager, GLTFSkeleton& skeleton, GLTFMesh& mesh);
    void setAnimation(const std::string& animationName);

private:
    std::vector<Animation> animations;
    size_t currentAnimation = 0;
    float currentTime = 0.0f;

    void printAnimationInfo(const Animation& animation, size_t index);
    glm::vec3 interpolateVec3(const std::vector<float>& input, const std::vector<glm::vec3>& output, float currentTime);
    glm::quat interpolateQuat(const std::vector<float>& input, const std::vector<glm::quat>& output, float currentTime);

    bool showDebug = false;
};

#endif // GLTFANIMATION_H
