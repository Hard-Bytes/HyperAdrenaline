#include "vector2f.hpp"

Vector2f::Vector2f() 
    : x(0), y(0)
{
}

Vector2f::Vector2f(float p_x, float p_y) 
    : x(p_x), y(p_y)
{
}

Vector2f::~Vector2f()
{
}

Vector2f& Vector2f::operator+(const Vector2f& other)
{
    x += other.x; y+= other.y;
    return *this;
}

Vector2f& Vector2f::operator-(const Vector2f& other)
{
    x -= other.x; y-= other.y;
    return *this;
}

void Vector2f::set(float p_x, float p_y)
{
    x = p_x; y = p_y;
}

/* Cycle reference
Vector3f& Vector2f::makeVectorXZ(Vector2f vec2f)
{
    return Vector3f(vec2f.x,0,vec2f.y);
}

Vector3f& Vector2f::makeVectorXY(Vector2f vec2f)
{
    return Vector3f(vec2f.x,vec2f.y,0);
}

Vector3f& Vector2f::makeVectorYZ(Vector2f vec2f)
{
    return Vector3f(0,vec2f.x,vec2f.y);
}
*/
