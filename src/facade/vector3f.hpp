#pragma once
#include <iostream>
#include <string>
#include <hyperengine/tree/node.hpp>
#include "vector2f.hpp"

class Vector3f
{
public:
    // Constructors
    Vector3f()
        : x(0), y(0), z(0) {}
    Vector3f(float p_x, float p_y, float p_z)
        : x(p_x), y(p_y), z(p_z) {}
    Vector3f(glm::vec3 in)
        : x(in.x), y(in.y), z(in.z) {}
	Vector3f(const Vector3f& copy)
        : x(copy.x), y(copy.y), z(copy.z) {}
    ~Vector3f() {}

    // Properties
    float x,y,z;

    // Operators
    Vector3f& operator=(const Vector3f& other)
        { x = other.x; y = other.y; z = other.z; return *this; }

    Vector3f operator+(const Vector3f& other) const
        { return Vector3f(x + other.x, y + other.y, z + other.z); }
    Vector3f operator-(const Vector3f& other) const
        { return Vector3f(x - other.x, y - other.y, z - other.z); }
    Vector3f operator*(const Vector3f& other) const
        { return Vector3f(x * other.x, y * other.y, z * other.z); }
    Vector3f operator/(const Vector3f& other) const
        { return Vector3f(x / other.x, y / other.y, z / other.z); }
    
    Vector3f& operator+=(const Vector3f& other)
        { x+=other.x; y+=other.y; z+=other.z; return *this; }
    Vector3f& operator-=(const Vector3f& other)
        { x-=other.x; y-=other.y; z-=other.z; return *this; }
    Vector3f& operator*=(const Vector3f& other)
        { x*=other.x; y*=other.y; z*=other.z; return *this; }
    Vector3f& operator/=(const Vector3f& other)
        { x/=other.x; y/=other.y; z/=other.z; return *this; }

    Vector3f operator*(const float value) const
        { return Vector3f(x * value, y * value, z * value); }
	Vector3f& operator*=(const float value)
        { x *= value; y *= value; z *= value; return *this; }
    Vector3f operator/(const float value) const
        { float i=(float)1.0/value; return Vector3f(x * i, y * i, z * i); }
	Vector3f& operator/=(const float value)
        { float i=(float)1.0/value; x*=i; y*=i; z*=i; return *this; }
    bool operator==(const Vector3f& v)
        { return x == v.x && y == v.y && z == v.z; }
    friend std::ostream& operator<<(std::ostream &os,const Vector3f& v)
    {
        os << "{" << v.x << ","<< v.y << ","<< v.z << "}";
        return os;
    }

    // Methods
    float length() const noexcept;
    Vector3f& normalize();
    Vector3f crossProduct(const Vector3f& other) const noexcept;
    float dotProduct(const Vector3f& other) const noexcept;
    void set(float p_x, float p_y, float p_z);
    Vector3f getHorizontalAngle() noexcept;
    std::string toNetworkString() const noexcept;

    //Transformation methods
    glm::vec3 makeGLMVector();
    Vector3f& makeVector3f(glm::vec3 irrVector3f);

    Vector2f makeVectorXZ(Vector3f vec3f);
    Vector2f makeVectorXY(Vector3f vec3f);
    Vector2f makeVectorYZ(Vector3f vec3f);
private:
    const double radiansToDegrees = 180.0 / 3.141592653;
    //const double degreesToRadians = 3.141592653 / 180.0;
};
