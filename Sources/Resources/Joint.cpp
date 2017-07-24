#include "Joint.h"


Joint::Joint(const std::string& jointName) : parent(-1), relativeBindTransform(1.f), name(jointName) {}
JointPose::JointPose() : priority(1.f), position(0.f), rotation(1.f, 0.f, 0.f, 0.f), scale(1.f) {}

