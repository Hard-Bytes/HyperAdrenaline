#include "vector3f.hpp"
#include "../network/messageAliases.hpp"

float Vector3f::length() const noexcept
{
    float length = x*x + y*y + z*z;
    if (length == 0 ) // this check isn't an optimization but prevents getting NAN in the sqrt.
        return 0;
    length = sqrt(length);
    return length;
}

Vector3f& Vector3f::normalize()
{
    float length = x*x + y*y + z*z;
    if (length == 0 ) // this check isn't an optimization but prevents getting NAN in the sqrt.
        return *this;
    length = sqrt(length);

    x = (x / length);
    y = (y / length);
    z = (z / length);
    return *this;
}

Vector3f Vector3f::crossProduct(const Vector3f& other) const noexcept
{
    return Vector3f(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vector3f::dotProduct(const Vector3f& other) const noexcept
{
    return x*other.x + y*other.y + z*other.z;
}

void Vector3f::set(float p_x, float p_y, float p_z)
{
    x = p_x; y = p_y; z = p_z;
}

Vector3f Vector3f::getHorizontalAngle() noexcept
{
    Vector3f angle;

    const double tmp = (std::atan2(x, z) * radiansToDegrees);
    angle.y = tmp;

    if (angle.y < 0)
        angle.y += 360;
    if (angle.y >= 360)
        angle.y -= 360;

    const double z1 = sqrt(x*x + z*z);

    angle.x = (std::atan2(z1, y) * radiansToDegrees - 90.f);

    if (angle.x < 0)
        angle.x += 360;
    if (angle.x >= 360)
        angle.x -= 360;

    return angle;
}

std::string Vector3f::toNetworkString() const noexcept
{
    return (
        std::to_string(x)
         + SEP +
        std::to_string(y)
         + SEP +
        std::to_string(z)
    );
}

glm::vec3 Vector3f::makeGLMVector()
{
    return glm::vec3(x,y,z);
}

Vector3f& Vector3f::makeVector3f(glm::vec3 glmVector3f)
{
    x = glmVector3f.x;
    y = glmVector3f.y;
    z = glmVector3f.z;
    return *this;
}

Vector2f Vector3f::makeVectorXZ(Vector3f vec3f)
{
    return Vector2f(vec3f.x,vec3f.z);
}

Vector2f Vector3f::makeVectorXY(Vector3f vec3f)
{
    return Vector2f (vec3f.x,vec3f.y);
}

Vector2f Vector3f::makeVectorYZ(Vector3f vec3f)
{
    return Vector2f (vec3f.y,vec3f.z);
}
