#include "triangle.hpp"

Triangle::Triangle()
{}

Triangle::Triangle(const Vector3f& pA, const Vector3f& pB, const Vector3f& pC)
:pointA(pA), pointB(pB), pointC(pC)
{}

Triangle::~Triangle()
{}

/*Constains Point*/
// Returns true if the triangle contains the point specified by the Vector3f
bool Triangle::containPoint(Vector3f point)
{
    //irr::core::triangle3df triangle;
    //return triangle.isPointInside(point.makeIrrVector());

    Vector3f af64((double)pointA.x, (double)pointA.y, (double)pointA.z);
    Vector3f bf64((double)pointB.x, (double)pointB.y, (double)pointB.z);
    Vector3f cf64((double)pointC.x, (double)pointC.y, (double)pointC.z);
    Vector3f pf64((double)point.x, (double)point.y, (double)point.z);
    return (isOnSameSide(pf64, af64, bf64, cf64) &&
        isOnSameSide(pf64, bf64, af64, cf64) &&
        isOnSameSide(pf64, cf64, af64, bf64));
}

/*Is Neighbour*/
// Return true if triangle and t, have 2 common vertex 
bool Triangle::isNeighbour(Triangle *t)
{
    int c(0);   //coincident points, must be 2 to return true
    if(this->pointA == t->pointA)
        ++c;
    if(this->pointA == t->pointB)
        ++c;
    if(this->pointA == t->pointC)
        ++c;
    if(this->pointB == t->pointA)
        ++c;
    if(this->pointB == t->pointB)
        ++c;
    if(this->pointB == t->pointC)
        ++c;
    if(this->pointC == t->pointA)
        ++c;
    if(this->pointC == t->pointB)
        ++c;
    if(this->pointC == t->pointC)
        ++c;
    if(c==2) return true;
    return false;
}

/*Get Center*/
// Return a Vector3f that specifies the point, that represents the center of the triangle
Vector3f Triangle::getCenter()
{
    Vector3f center;
    center.x = (pointA.x + pointB.x + pointC.x)/3.f;
    center.y = (pointA.y + pointB.y + pointC.y)/3.f;
    center.z = (pointA.z + pointB.z + pointC.z)/3.f;

    return center;     
}

/*Is On Same Side*/
bool Triangle::isOnSameSide(
    const Vector3f& p1
    , const Vector3f& p2
    , const Vector3f& a
    , const Vector3f& b
) const noexcept
{
    Vector3f bminusa = b - a;
    Vector3f cp1 = bminusa.crossProduct(p1 - a);
    Vector3f cp2 = bminusa.crossProduct(p2 - a);
    double res = cp1.dotProduct(cp2);
    if ( res < 0 )
    {
        // This catches some floating point troubles.
        // Unfortunately slightly expensive and we don't really know the best epsilon for iszero.
        Vector3f cp1 = bminusa.normalize().crossProduct((p1 - a).normalize());
        if ( 	fabs(cp1.x) <= (double)ROUNDING_TOLERANCE
            && 	fabs(cp1.y) <= (double)ROUNDING_TOLERANCE
            && 	fabs(cp1.z) <= (double)ROUNDING_TOLERANCE )
        {
            res = 0.f;
        }
    }
    return (res >= 0.0f);
}
