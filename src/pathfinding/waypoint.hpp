#pragma once
#include "../facade/vector3f.hpp"
#include "../facade/graphicNode.hpp"

class Waypoint
{
public:
    // Constructor
    explicit Waypoint(Vector3f pos);
    ~Waypoint(){}

    Vector3f position {0,0,0};   //Position of the waypoint
private:
};
