#pragma once
//#include "vector3f.hpp"

class Vector2f
{
public:
    //Constructors and destructor
    Vector2f();
    Vector2f(float p_x, float p_y);
    ~Vector2f();

    //Operators
    Vector2f& operator+(const Vector2f& other);
    Vector2f& operator-(const Vector2f& other);
    Vector2f operator/(const Vector2f& other) const
        { return Vector2f(x / other.x, y / other.y); }

    Vector2f operator/(const float& val) const
        { return Vector2f(x / val, y / val); }

    bool operator==(const Vector2f& v)
        { return x == v.x && y == v.y; }
    bool operator!=(const Vector2f& v)
        { return x != v.x || y != v.y; }

    float x,y;
    void set(float p_x, float p_y);

    //Transformation methods
    //Cycle refence
    /*Vector3f  makeVectorXZ(Vector2f vec2f);
    Vector3f  makeVectorXY(Vector2f vec2f);
    Vector3f  makeVectorYZ(Vector2f vec2f);*/

private:
};