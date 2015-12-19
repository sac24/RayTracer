#ifndef _color_h
#define _color_h

#include <iostream>

using namespace std;

class Color
{

public:
double r,g,b,a;
 
Color() : r(0.0),g(0.0),b(0.0) {
	a = 1.0;
}

Color(double r, double g, double b) 
{
	 this->r = r; this->g = g; this->b = b; this->a = 1.0;
}

 Color operator+ (const Color& col2) 
 {
    Color col1(0.0,0.0,0.0);
    col1.r = this->r + col2.r;
    col1.g = this->g + col2.g;
    col1.b = this->b + col2.b;
    return col1;
 }

 Color operator- (const Color& col2)
 {
    Color col1(0.0,0.0,0.0);
    col1.r = this->r - col2.r;
    col1.g = this->g - col2.g;
    col1.b = this->b - col2.b;
    return col1;
 }     

 Color& operator= (const Color& col)
 {
	this->r = col.r;
    this->g = col.g;
    this->b = col.b;
    return *this; 
 }    

 Color operator/ (const double& k)
 {
 	Color col(0.0,0.0,0.0);
    col.r = this->r/k;
    col.g = this->g/k;
    col.b = this->b/k;
    return col;
 }        

 Color operator* (const double& k)
 {
 	Color col(0.0,0.0,0.0);
    col.r = this->r * k;
    col.g = this->g * k;
    col.b = this->b * k;
    return col;
 }         

 // Color operator* (const T& k, const Color& col)
 // {
 //        return Color(
 //            k * col.r, 
 //            k * col.g, 
 //            k * col.b);
 // }

 Color& operator *= (const double& k)
 {
    this->r *= k;
    this->g *= k;
    this->b *= k;
    return *this;
 }

 Color& operator /= (const double& k)
 {
    this->r /= k;
    this->g /= k;
    this->b /= k;
    return *this;
 }

 Color& operator += (const Color& col)
 {
    this->r += col.r;
    this->g += col.g;
    this->b += col.b;
    return *this;
 }

 Color& operator -= (const Color& col)
 {
    this->r -= col.r;
    this->g -= col.g;
    this->b -= col.b;
    return *this;
 }

 void display_color() 
{
	cout << "[" << r << "," << g << "," << b << "]" << endl;
}

 uint32_t ToUInt32() const
 {
	uint32_t R = (uint32_t)(r * 255.0f);
	uint32_t G = (uint32_t)(g * 255.0f);
	uint32_t B = (uint32_t)(b * 255.0f);
	uint32_t A = (uint32_t)(a * 255.0f);

	return (A << 24) | (R << 16) | (G << 8) | B;
 } 	

 void ColorProduct (const Color& col1, const Color& col2) 
 {
    this->r = col1.r * col2.r;
    this->g = col1.g * col2.g;
    this->b = col1.b * col2.b;
    return;
 }

};

#endif
