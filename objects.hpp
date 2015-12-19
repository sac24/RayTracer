#ifndef _objects_h
#define _objects_h

#include "vector.hpp"
#include "color.hpp"
#include <math.h>
#include <vector>
#include <cmath>

#define DEBUG
#define PI 3.14159

unsigned char* Read_PPM(const char*, int*, int*);

unsigned char* Read_PPM(const char* filename, int* width, int* height)
{
    FILE* fp;
    int i, w, h, d;
    unsigned char* image;
    char head[70];          /* max line <= 70 in PPM (per spec). */
    
    fp = fopen(filename, "rb");
    if (!fp) {
        perror(filename);
        return NULL;
    }
    
    /* grab first two chars of the file and make sure that it has the
       correct magic cookie for a raw PPM file. */
    fgets(head, 70, fp);
    if (strncmp(head, "P6", 2)) {
        fprintf(stderr, "%s: Not a raw PPM file\n", filename);
        return NULL;
    }
    
    /* grab the three elements in the header (width, height, maxval). */
    i = 0;
    while(i < 3) {
        fgets(head, 70, fp);
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }
    
    /* grab all the image data in one fell swoop. */
    image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
    fread(image, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);
    
    *width = w;
    *height = h;

    cout << "Read " << filename << " , width = " << w << ", height = " << h << std::endl; 

    return image;
}


class Object;

struct Intersection
{
        
        Object* obj;
        double distanceSquared;
        Vector point;
        Vector surfaceNormal;
        Intersection() : obj(NULL),distanceSquared(1.0e+5),point(),surfaceNormal(){}
        
};

typedef std::vector<Intersection> Intersection_List;

class Object 
{

	protected:
	Vector center;
	Color color;
	double reflectivity;
	Intersection_List obj_inter_list;
	
	public:
	int object_id;	
	bool texture_flag;

	struct texture_obj {
		unsigned char* image_data;
		int tex_width;
		int tex_height;
	} tex_obj;

	Object() : center(0.0,0.0,-10.0), color(0.0,0.0,0.0) , reflectivity(0.5) {}
	Object(const Vector& pos, const Color& col, const double& ref) :  center(pos), 
										    						  color(col),
																	  reflectivity(ref){}
																						

	virtual ~Object(){}
	void addTexture(const char*);	
	virtual void check_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection_List& list) = 0;
	int find_nearest_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection& inter)
    {
        obj_inter_list.clear();
        check_Intersection(vec_origin,vec_dir,obj_inter_list);
        int result = select_closest_point(obj_inter_list, inter);
        return result;
    }

    int select_closest_point(const Intersection_List& obj_inter_list, Intersection& inter);    

    Color getcolor() {
    	return this->color;
    }    

    virtual Color gettexel(const Intersection& inter) = 0;

    double getreflectivity() {
    	return this->reflectivity;
    }
};

void Object::addTexture(const char* filename)
{

	int w, h;
    tex_obj.image_data = Read_PPM(filename,&w,&h);
    
    if(tex_obj.image_data == NULL) {
    	cerr << "File type or data is invalid! Adding texture Failed!" << endl;
    }

    texture_flag = true;
    tex_obj.tex_width = w;
    tex_obj.tex_height = h;
}

int Object::select_closest_point(const Intersection_List& obj_inter_list, Intersection& inter) 
{
		int size = obj_inter_list.size();

		switch(size) {

			case 0: /*No intersection*/
					return 0;

			case 1: /*Only one point detected with intersection*/
					inter = obj_inter_list[0];
					return 1;

			default: /* More point or objects detected need to select closest */
					//cout << "Came here!" << endl;
					int begin = 0;
					int end = obj_inter_list.size();
					int closest_index = begin;
        			begin++;

        			while(begin!=end) {

        				if(obj_inter_list[begin].distanceSquared < obj_inter_list[closest_index].distanceSquared) 
        				{
        					closest_index = begin;
        				}

        				begin++;		
        			}

        			inter = obj_inter_list[closest_index];
        			return 1;
		}

}

class Sphere : public Object
{
	private:
	double radius;
	
	public:	
	Sphere() : Object(), radius(1.0) {
		object_id = 1;
		texture_flag = false;
		tex_obj.image_data = NULL;
		tex_obj.tex_height = 0;
		tex_obj.tex_width = 0;
	}

	Sphere(const Vector& pos, const double& rad, const Color& col, const double& ref, const int& id) : Object(pos,col,ref) , radius(rad) {
		object_id = id;
		texture_flag = false;
		tex_obj.image_data = NULL;
		tex_obj.tex_height = 0;
		tex_obj.tex_width = 0;
	}
	
	void check_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection_List& list);
	Color gettexel(const Intersection& inter);
};

void Sphere::check_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection_List& list)
{

	/* Solve quadratic equation */
	Vector new_vec; 
	new_vec = vec_origin;
	new_vec -= center;
	
	Vector new_vec_dir = vec_dir;

	double a = new_vec_dir.mag_square();
	Vector temp;
	double b = 2.0 * temp.DotProduct(new_vec_dir,new_vec);
	double c = new_vec.mag_square() - radius*radius;

	double D = b*b - 4.0*a*c;

	double sol[2];

	if(D >= 0.0) {

	  sol[0] = (-b + sqrt(D))/(2.0*a);
	  sol[1] = (-b - sqrt(D))/(2.0*a);  

	  for(int i=0;i<2;i++) {

		Intersection new_intersect;	
		Vector vec_to_point = new_vec_dir*sol[i];

		/* Populate the struct */
		new_intersect.obj = this;
		new_intersect.distanceSquared = vec_to_point.mag_square();
		new_intersect.point = vec_origin; new_intersect.point += vec_to_point;
		new_intersect.surfaceNormal = (new_intersect.point - center).unit_vector();

		//Debug: Fixed bug - Ray may trace backwards and intersect with objects. 
		// Found myself : Check the dot product of the ray direction and the vector between the point of intersection & point of origin
		// If the dotproduct is positive then they point in same direction which is wrong behavior!
		Vector check_vec(0.0,0.0,0.0); 
		check_vec -= new_intersect.point;
		check_vec += vec_origin; 
		check_vec = check_vec.unit_vector();
		Vector check_vec_2 = vec_dir;

		Vector check_vec_3;
		double check_param = check_vec_3.DotProduct(check_vec,check_vec_2);

		// if(new_intersect.point.z < 0 && (check_param < 0)) {
		// 	list.push_back(new_intersect);			
		// }

		if(check_param < 0) {
			list.push_back(new_intersect);			
		}

		if(sol[0] == sol[1] && D == 0) { /*Just one unique solution*/
				break;
		}	
	  }
		
	} else {
		return;
	}

}

Color Sphere::gettexel(const Intersection& inter)
{

	Vector normal = inter.surfaceNormal;
	
	int u = ((atan2(normal.z,normal.x))/2*PI + 0.5 ) * tex_obj.tex_width;
	int v = (0.5 - (asin(normal.y)/PI)) * tex_obj.tex_height;

	Color tex_color(0.0,0.0,0.0);
	int index = (v*tex_obj.tex_width + u)*3;

	if(index < tex_obj.tex_width*tex_obj.tex_height*3) {
		tex_color.r = (tex_obj.image_data[index])/255.0; 
		tex_color.g = (tex_obj.image_data[index+1])/255.0; 
		tex_color.b = (tex_obj.image_data[index+2])/255.0;
	}

	return tex_color; 
}

class Plane : public Object
{
	private:
	double length;
	double width;
	Vector normal;
	Vector headup;

	public:	
	Plane() : Object(), length(10.0), width(10.0), normal(Vector(0.0,1.0,0.0)),headup(Vector(0.0,1.0,0.0)) {
		object_id = 1;
		texture_flag = false;
		tex_obj.image_data = NULL;
		tex_obj.tex_height = 0;
		tex_obj.tex_width = 0;
	}

	Plane(const Vector& pos, const double& l, const double& w, const Vector& Normal, const Vector& Headup, const Color& col, const double& ref, const int& id) : Object(pos,col,ref) , length(l), width(w), normal(Normal), headup(Headup) {
		object_id = id;
		texture_flag = false;
		tex_obj.image_data = NULL;
		tex_obj.tex_height = 0;
		tex_obj.tex_width = 0;
	}
	
	void check_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection_List& list);
	Color gettexel(const Intersection& inter);
};

void Plane::check_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection_List& list)
{

	static int count = 0;

	Vector temp;
	Vector vec_direction = vec_dir;
	double denom = temp.DotProduct(normal,vec_direction); 

    if (abs(denom) > 1e-6) { 
        
        Vector v;
        v += vec_origin;
        v -= center;
        double numer = v.DotProduct(normal,v);
        double t = (-numer)/denom; 
        
        Vector collision_point;
        collision_point += vec_origin;
        collision_point += (vec_direction * t);

        Vector up_vec;
        up_vec += collision_point;
        up_vec -= center;
        double up_val = temp.DotProduct(headup,up_vec);

        Vector right_vec;
        right_vec = right_vec.CrossProduct(normal,headup);
        Vector new_vec;
        new_vec += collision_point;
        new_vec -= center;
        double right_val = temp.DotProduct(right_vec,new_vec);

        if( (up_val <= (length/2.0) && up_val >= (-length/2.0)) && (right_val <= (width/2.0) && right_val >= (-width/2.0)) )
        {
        	Intersection new_intersect;
        	Vector vec_to_point;
        	vec_to_point += (vec_direction * t);

        	new_intersect.obj = this;
			new_intersect.distanceSquared = vec_to_point.mag_square();
		    new_intersect.point = collision_point;
		    new_intersect.surfaceNormal = normal;

		    Vector check_vec(0.0,0.0,0.0); 
			check_vec -= new_intersect.point;
			check_vec += vec_origin; 
			check_vec = check_vec.unit_vector();
			Vector check_vec_2 = vec_dir;

			Vector check_vec_3;
			double check_param = check_vec_3.DotProduct(check_vec,check_vec_2);

		 //    if(new_intersect.point.z < 0 && (check_param < 0)) {				
			// list.push_back(new_intersect);			
			// }

			if(check_param < 0) {				
			list.push_back(new_intersect);			
			}
        }

    } 
 
    return; 

}


Color Plane::gettexel(const Intersection& inter)
{

	Vector point = inter.point;
	Vector vec = point;
	vec -= center;

	Vector right_vec;
    right_vec = right_vec.CrossProduct(normal,headup);

	int u = ( (vec.DotProduct(vec,right_vec))/width + 0.5 ) * tex_obj.tex_width;
	int v = ( (vec.DotProduct(vec,headup))/length + 0.5 ) * tex_obj.tex_height;

	Color tex_color(0.0,0.0,0.0);
	int index = (v*tex_obj.tex_width + u)*3;

	tex_color.r = (tex_obj.image_data[index])/255.0; 
	tex_color.g = (tex_obj.image_data[index+1])/255.0; 
	tex_color.b = (tex_obj.image_data[index+2])/255.0;

	return tex_color; 
}

struct Light_Source
{
       
    Vector  location;
    Color   color;
    Light_Source(const Vector& loc, const Color& col) : location(loc), color(col){}
        
};

#endif
