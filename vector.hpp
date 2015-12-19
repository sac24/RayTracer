#ifndef _vector_h
#define _vector_h

#include "color.hpp"
#include <iostream>
#include <math.h>

using namespace std;

class Vector {
 
public:
double x,y,z;
 
Vector() : x(0.0),y(0.0),z(0.0) {}

Vector(double x, double y, double z) 
{
	 this->x = x; this->y = y; this->z = z;
}

 Vector operator+ (const Vector& vec2) 
 {
    Vector vec1(0.0,0.0,0.0);
    vec1.x = this->x + vec2.x;
    vec1.y = this->y + vec2.y;
    vec1.z = this->z + vec2.z;
    return vec1;
 }

 Vector operator- (const Vector& vec2)
 {
    Vector vec1(0.0,0.0,0.0);
    vec1.x = this->x - vec2.x;
    vec1.y = this->y - vec2.y;
    vec1.z = this->z - vec2.z;
    return vec1;
 }     

 Vector& operator= (const Vector& vec)
 {
	this->x = vec.x;
    this->y = vec.y;
    this->z = vec.z; 
    return *this;
 }    

bool operator!= (const Vector& vec)
 {
    if (this->x == vec.x && this->y == vec.y && this->z == vec.z)
    {
        return true;
    }    
    
    return false;
 }  

 Vector operator/ (const double& k)
 {
 	Vector v1(0.0,0.0,0.0);
    v1.x = this->x/k;
    v1.y = this->y/k;
    v1.z = this->z/k;
    return v1;
 }        

 Vector operator* (const double& k)
 {
 	Vector v1(0.0,0.0,0.0);
    v1.x = this->x * k;
    v1.y = this->y * k;
    v1.z = this->z * k;
    return v1;
 }         

 Vector& operator *= (const double& k)
 {
    this->x *= k;
    this->y *= k;
    this->z *= k;
    return *this;
 }

 // Vector operator * (const double& k, const Vector& vec1)
 // {
 //        return Vector(
 //            k * vec1.x, 
 //            k * vec1.y, 
 //            k * vec1.z);
 // }

 Vector& operator /= (const double& k)
 {
    this->x /= k;
    this->y /= k;
    this->z /= k;
    return *this;
 }

 Vector& operator += (const Vector& vec)
 {
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    return *this;
 }

 Vector& operator -= (const Vector& vec)
 {
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    return *this;
 }

 double DotProduct (const Vector& vec1, const Vector& vec2) 
 {
    return (vec1.x * vec2.x) + (vec1.y * vec2.y) + (vec1.z * vec2.z);
 }

 Vector CrossProduct (const Vector& vec1, const Vector& vec2)
 {
    return Vector(
            (vec1.y * vec2.z) - (vec1.z * vec2.y), 
            (vec1.z * vec2.x) - (vec1.x * vec2.z), 
            (vec1.x * vec2.y) - (vec1.y * vec2.x)
            );
 }

const double mag_square()
 {
    return ((x*x) + (y*y) + (z*z));
 }

const double mag()
{
    return sqrt(mag_square());
}

const Vector unit_vector()
{
    const double m = mag();
    return Vector(x/m, y/m, z/m);
}

void display_vector() 
{
	cout << "[" << x << "," << y << "," << z << "]" << endl;
}

};

/* Ray struct */
struct Ray
{
	Vector pos;
	Vector dir;
	Color  col;

}; 

#endif
