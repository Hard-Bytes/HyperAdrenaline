#pragma once
#include "../facade/vector3f.hpp"

#define ROUNDING_TOLERANCE 0.000001f

class Triangle
{
public:
    //Constructors and destructor
    Triangle();
    Triangle(const Vector3f& pA, const Vector3f& pB, const Vector3f& pC);
    ~Triangle();

    //Functions
    bool containPoint(Vector3f point);
    bool isNeighbour(Triangle *t);

    //Getters
    Vector3f getCenter();

private:
    Vector3f pointA;
    Vector3f pointB;
    Vector3f pointC;

    // Helpers
    bool isOnSameSide(const Vector3f& p1, const Vector3f& p2, const Vector3f& a, const Vector3f& b) const noexcept;
};