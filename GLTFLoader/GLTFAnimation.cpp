#include "GLTFAnimation.h"
#include <iostream>
#include "GLTFSkeleton.h"

void GLTFAnimation::parseAnimations(yyjson_val* animationsArray, const GLTFAccessor& accessorManager, const GLTFBuffer& bufferManager) {
    std::cout << "Parsing Animations..." << std::endl;

    size_t idx, max;
    yyjson_val* animation_val;
    yyjson_arr_foreach(animationsArray, idx, max, animation_val) {
        Animation animation;

        const char* name = yyjson_get_str(yyjson_obj_get(animation_val, "name"));
        animation.name = name ? name : "Unnamed";

        yyjson_val* channels_val = yyjson_obj_get(animation_val, "channels");
        yyjson_val* samplers_val = yyjson_obj_get(animation_val, "samplers");

        if (channels_val) {
            size_t ch_idx, ch_max;
            yyjson_val* channel_val;
            yyjson_arr_foreach(channels_val, ch_idx, ch_max, channel_val) {
                Channel channel;
                channel.sampler = yyjson_get_int(yyjson_obj_get(channel_val, "sampler"));
                yyjson_val* target_val = yyjson_obj_get(channel_val, "target");
                if (target_val) {
                    yyjson_val* node_val = yyjson_obj_get(target_val, "node");
                    if (node_val && yyjson_is_int(node_val)) {
                        channel.targetNode = yyjson_get_int(node_val);
                    }
                    else {
                        std::cout << "Invalid or missing target node in channel." << std::endl;
                        channel.targetNode = -1;
                    }
                    channel.targetPath = yyjson_get_str(yyjson_obj_get(target_val, "path"));
                }
                channel.extensions = yyjson_obj_get(channel_val, "extensions");
                channel.extras = yyjson_obj_get(channel_val, "extras");
                animation.channels.push_back(channel);
            }
        }

        if (samplers_val) {
            size_t samp_idx, samp_max;
            yyjson_val* sampler_val;
            yyjson_arr_foreach(samplers_val, samp_idx, samp_max, sampler_val) {
                Sampler sampler;
                sampler.input = yyjson_get_int(yyjson_obj_get(sampler_val, "input"));
                sampler.output = yyjson_get_int(yyjson_obj_get(sampler_val, "output"));
                sampler.interpolation = yyjson_get_str(yyjson_obj_get(sampler_val, "interpolation"));
                sampler.extensions = yyjson_obj_get(sampler_val, "extensions");
                sampler.extras = yyjson_obj_get(sampler_val, "extras");

                // Populate inputTimes
                sampler.inputTimes = bufferManager.getAccessorDataFloat(accessorManager.getAccessors()[sampler.input]);

                // Populate output values based on target path
                if (std::any_of(animation.channels.begin(), animation.channels.end(),
                    [&](const Channel& channel) { return channel.sampler == samp_idx && channel.targetPath == "translation"; })) {
                    sampler.outputValuesVec3 = bufferManager.getAccessorDataVec3(accessorManager.getAccessors()[sampler.output]);
                }
                else if (std::any_of(animation.channels.begin(), animation.channels.end(),
                    [&](const Channel& channel) { return channel.sampler == samp_idx && channel.targetPath == "rotation"; })) {
                    sampler.outputValuesQuat = bufferManager.getAccessorDataQuat(accessorManager.getAccessors()[sampler.output]);
                }
                else if (std::any_of(animation.channels.begin(), animation.channels.end(),
                    [&](const Channel& channel) { return channel.sampler == samp_idx && channel.targetPath == "scale"; })) {
                    sampler.outputValuesVec3 = bufferManager.getAccessorDataVec3(accessorManager.getAccessors()[sampler.output]);
                }

                animation.samplers.push_back(sampler);
            }
        }

        animation.extensions = yyjson_obj_get(animation_val, "extensions");
        animation.extras = yyjson_obj_get(animation_val, "extras");

        animations.push_back(animation);
    }

    std::cout << "Completed parsing Animations." << std::endl;
    for (size_t i = 0; i < animations.size(); ++i) {
        if (showDebug) printAnimationInfo(animations[i], i);
    }
}

size_t GLTFAnimation::getAnimationCount() const {
    return animations.size();
}

const std::vector<GLTFAnimation::Animation>& GLTFAnimation::getAnimations() const {
    return animations;
}

void GLTFAnimation::updateAnimation(float deltaTime, GLTFNode& nodeManager, GLTFSkeleton& skeleton, GLTFMesh& mesh) {
    currentTime += deltaTime;

    // Get the duration of the current animation
    float animationDuration = animations[currentAnimation].samplers[0].inputTimes.back();

    // Reset currentTime if it surpasses the animation duration to loop the animation
    if (currentTime > animationDuration) {
        currentTime = std::fmod(currentTime, animationDuration);
    }

    //std::cout << "Updating animation at " << currentTime << " / " << animationDuration << std::endl;
    for (const auto& channel : animations[currentAnimation].channels) {
        const auto& sampler = animations[currentAnimation].samplers[channel.sampler];

       // std::cout << "Channel Target Node: " << channel.targetNode
       //     << ", Target Path: " << channel.targetPath << std::endl;

        

        if (channel.targetPath == "translation") {
            glm::vec3 interpolatedValue = interpolateVec3(sampler.inputTimes, sampler.outputValuesVec3, currentTime);
            //std::cout << "Interpolated Translation: " << glm::to_string(interpolatedValue) << std::endl;
            nodeManager.setNodeTranslation(channel.targetNode, interpolatedValue);
        }
        else if (channel.targetPath == "rotation") {
            glm::quat interpolatedValue = interpolateQuat(sampler.inputTimes, sampler.outputValuesQuat, currentTime);
            //std::cout << "Interpolated Rotation: " << glm::to_string(interpolatedValue) << std::endl;
            nodeManager.setNodeRotation(channel.targetNode, interpolatedValue);
        }
        else if (channel.targetPath == "scale") {
            glm::vec3 interpolatedValue = interpolateVec3(sampler.inputTimes, sampler.outputValuesVec3, currentTime);
            //std::cout << "Interpolated Scale: " << glm::to_string(interpolatedValue) << std::endl;
            nodeManager.setNodeScale(channel.targetNode, interpolatedValue);
        }
    }

    // Update node transformations
    auto& nodes = nodeManager.getNodes();
    for (auto& node : nodes) {
       // nodeManager.updateNodeTransformation(node);
        //std::cout << "Node: " << node.name << ", Transformation: " << glm::to_string(node.transformation) << std::endl;
    }

    // Update skeleton with the new node transformations
    skeleton.updateSkeleton(nodes);

    // Update mesh vertices with the new joint transformations
    //mesh.updateVertices(skeleton.getBoneTransforms());
}


glm::vec3 GLTFAnimation::interpolateVec3(const std::vector<float>& inputTimes, const std::vector<glm::vec3>& outputValues, float currentTime) {
    if (inputTimes.empty() || outputValues.empty()) return glm::vec3(0.0f);

    size_t count = inputTimes.size();
    if (currentTime <= inputTimes.front()) return outputValues.front();
    if (currentTime >= inputTimes.back()) return outputValues.back();

    for (size_t i = 0; i < count - 1; ++i) {
        if (currentTime >= inputTimes[i] && currentTime <= inputTimes[i + 1]) {
            float t = (currentTime - inputTimes[i]) / (inputTimes[i + 1] - inputTimes[i]);
            glm::vec3 result = glm::mix(outputValues[i], outputValues[i + 1], t);

           // std::cout << "Interpolating Vec3: time = " << currentTime << ", t = " << t
            //    << ", start = " << glm::to_string(outputValues[i])
             //   << ", end = " << glm::to_string(outputValues[i + 1])
              //  << ", result = " << glm::to_string(result) << std::endl;

            return result;
        }
    }

    return outputValues.back(); // fallback
}


glm::quat GLTFAnimation::interpolateQuat(const std::vector<float>& inputTimes, const std::vector<glm::quat>& outputValues, float currentTime) {
    if (inputTimes.empty() || outputValues.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    size_t count = inputTimes.size();
    if (currentTime <= inputTimes.front()) return outputValues.front();
    if (currentTime >= inputTimes.back()) return outputValues.back();

    for (size_t i = 0; i < count - 1; ++i) {
        if (currentTime >= inputTimes[i] && currentTime <= inputTimes[i + 1]) {
            float t = (currentTime - inputTimes[i]) / (inputTimes[i + 1] - inputTimes[i]);
            glm::quat result = glm::slerp(outputValues[i], outputValues[i + 1], t);

           // std::cout << "Interpolating Quat: time = " << currentTime << ", t = " << t
            //    << ", start = " << glm::to_string(outputValues[i])
             //   << ", end = " << glm::to_string(outputValues[i + 1])
             //   << ", result = " << glm::to_string(result) << std::endl;

            return result;
        }
    }

    return outputValues.back(); // fallback
}


void GLTFAnimation::setAnimation(const std::string& animationName) {
    for (size_t i = 0; i < animations.size(); ++i) {
        if (animations[i].name == animationName) {
            std::cout << "Setting the model's animation to " << animationName << std::endl;
            //printAnimationInfo(animations[i], i);
            currentAnimation = i;
            currentTime = 0.0f;
            break;
        }
    }
}

void GLTFAnimation::printAnimationInfo(const Animation& animation, size_t index) {
    std::cout << "Animation Info [" << index << "]:" << std::endl;
    std::cout << "Name: " << animation.name << std::endl;
    std::cout << "Channels: " << animation.channels.size() << std::endl;
    std::cout << "Samplers: " << animation.samplers.size() << std::endl;
    std::cout << "Extensions: " << (animation.extensions ? "Yes" : "No") << std::endl;
    std::cout << "Extras: " << (animation.extras ? "Yes" : "No") << std::endl;
    for (size_t i = 0; i < animation.channels.size(); ++i) {
        const auto& channel = animation.channels[i];
        std::cout << "  Channel [" << i << "]:" << std::endl;
        std::cout << "    Sampler: " << channel.sampler << std::endl;
        std::cout << "    Target Node: " << channel.targetNode << std::endl;
        std::cout << "    Target Path: " << channel.targetPath << std::endl;
        std::cout << "    Extensions: " << (channel.extensions ? "Yes" : "No") << std::endl;
        std::cout << "    Extras: " << (channel.extras ? "Yes" : "No") << std::endl;
    }
    for (size_t i = 0; i < animation.samplers.size(); ++i) {
        const auto& sampler = animation.samplers[i];
        std::cout << "  Sampler [" << i << "]:" << std::endl;
        std::cout << "    Input: " << sampler.input << std::endl;
        std::cout << "    Output: " << sampler.output << std::endl;
        std::cout << "    Interpolation: " << sampler.interpolation << std::endl;
        std::cout << "    Extensions: " << (sampler.extensions ? "Yes" : "No") << std::endl;
        std::cout << "    Extras: " << (sampler.extras ? "Yes" : "No") << std::endl;
    }
}
