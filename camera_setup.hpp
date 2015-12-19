#ifndef _camera_setup_h
#define _camera_setup_h

#include "vector.hpp"
#include <stdint.h>
#include <math.h>

#define DEBUG
#define PI 3.14159

class Camera_Setup 
{

private:
	Vector camera_center;
	Vector UP_VEC;
	Vector W;  /* Towards Image plane */
	Vector U;  /* 3rd vector perpendicular to W & U */
	Vector V;  /* Head direction */
	double distance;  /*Distance from image plane*/
	int im_plane_width;
	int im_plane_height;
	double aspect_ratio;
	double theta_u;
	double theta_v;

public:
	Camera_Setup():camera_center(),UP_VEC(Vector(0.0,1.0,0.0)),W(Vector(0.0,0.0,-1.0)),distance(1.0),im_plane_width(640),im_plane_height(640),theta_u(90),theta_v(90){} 
	Camera_Setup(const Vector& center,const Vector& up_vec, const Vector& w, double dist,int img_w, int img_h, double theta_u, double theta_v);
	void camera_initialization();
	Vector compute_pixel_vector(uint32_t row, uint32_t col);
	Vector get_camera_center();
	double get_pixel_height();

};

Camera_Setup::Camera_Setup(const Vector& center,const Vector& up_vec, const Vector& w, double dist,int img_w, int img_h, double theta_u, double theta_v)
{

	camera_center = center;
	UP_VEC = up_vec;
	W = w;
	distance = dist;
	im_plane_width = img_w;
	im_plane_height = img_h;
	this->theta_u = theta_u;
	this->theta_v = theta_v; 
	camera_initialization();
}

void Camera_Setup::camera_initialization() 
{
	aspect_ratio = im_plane_width/im_plane_height;
	U = U.CrossProduct(W,UP_VEC);
	U = U.unit_vector(); /* Normalize */
	V = V.CrossProduct(U,W);
	V = V.unit_vector(); /* Normalize */

	#ifdef DEBUG
		W.display_vector();
		U.display_vector();
		V.display_vector();
		camera_center.display_vector();
	#endif
}

Vector Camera_Setup::compute_pixel_vector(uint32_t row, uint32_t col)
{
	double pix_w = (2*tan((PI*theta_u/180)/2)*distance)/im_plane_width;
	double pix_h = (2*tan((PI*theta_v/180)/2)*distance)/im_plane_height;

	Vector pix_loc = camera_center + W - U*((im_plane_width/2.0)*pix_w) + V*((im_plane_height/2.0)*pix_h) + U*(pix_w/2.0) - V*(pix_h/2.0) + U*(pix_w*col) - V*(pix_h*row);
	Vector vec_dir = pix_loc - camera_center;
	vec_dir = vec_dir.unit_vector();
	//vec_dir.display_vector();
	return vec_dir;
}

Vector Camera_Setup::get_camera_center()
{
	return camera_center;
}

double Camera_Setup::get_pixel_height()
{
	double pix_h = (2*tan((PI*theta_v/180)/2)*distance)/im_plane_height;
	return pix_h;
}

#endif
