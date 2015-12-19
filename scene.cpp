#include "vector.hpp"
#include "color.hpp"
#include "camera_setup.hpp"
#include "objects.hpp"
#include "framebuffer.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdint.h>

#define DOF_ENABLED
#define DOF_NUM_RAYS 50
#define MAX_RECURSION 8
#define MAX_ARGUMENTS 2
#define NUM_CUBE_FACES 6
#define DEPTH_ITER_LIMIT 3

double dof_val = 3.0;
const double epsilon = 1e-10;

using namespace std;

static void check_color(Color&);

class Scene
{		
	private:	
	Camera_Setup camera;
	Color backgroundColor;                  

	public:

    typedef std::vector<Object*> Object_List;
    typedef std::vector<Light_Source> Light_Source_List;

    Object_List obj_list;
    Light_Source_List light_list;
	Intersection_List intersection_list;

    Scene(const Camera_Setup& cam, const Color& col) : camera(cam), backgroundColor(col) {}

    virtual ~Scene()
    {
        delete_object();
    }

        
    Object& add_Object(Object* new_obj)
    {
        obj_list.push_back(new_obj);
        return *new_obj;
    }

    void add_Light_Source(const Light_Source& source)
    {
        light_list.push_back(source);
    }

    void delete_object();

    int find_nearest_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection& inter);
    int select_closest_point(const Intersection_List& obj_inter_list, Intersection& inter); 
    Color TraceRay(const Vector& vec_origin,const Vector& vec_dir, Color& ray_intensity, int recursion_depth);
	Color GetColor(const Intersection& inter, const Vector& vec_dir, Color& ray_intensity, int recursion_depth);
	bool check_surface_to_light(Vector vec1, Vector vec2,int obj_id);
	Color Reflection(const Intersection& inter, const Vector& incident_dir, Color& ray_intensity,int recursionDepth);
	Color getAmbientLighting(const Intersection& inter, const Color& color);
   	Color getDiffuseAndSpecularLighting(const Intersection& inter, const Vector& vec_dir, const Color& color);
    void image_ppm(const char* filename, const uint32_t& width, const uint32_t& height);
    Color DOF_TraceRay(const Vector& vec_origin,const Vector& vec_dir, Color& ray_intensity, int recursion_depth, double depth_of_field);

};

void Scene::delete_object() 
{

	Object_List::iterator begin = obj_list.begin();
    Object_List::iterator end  = obj_list.end();
        
    while(begin!=end)
    {
        delete *begin;
        *begin = NULL;
        begin++;
    }
        
    obj_list.clear();

}

int Scene::find_nearest_Intersection(const Vector& vec_origin,const Vector& vec_dir,Intersection& inter)
{

        intersection_list.clear();     
        Object_List::iterator begin = obj_list.begin();
        Object_List::iterator end  = obj_list.end();

        while(begin!=end)
        {
            Object& sel_obj = *(*begin);
            sel_obj.check_Intersection(vec_origin,vec_dir,intersection_list);
            begin++;
        }

        return select_closest_point(intersection_list, inter);

}

int Scene::select_closest_point(const Intersection_List& obj_inter_list, Intersection& inter) 
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

bool Scene::check_surface_to_light(Vector vec1, Vector vec2, int obj_id)
{        
        Vector dir = vec2;
        dir -= vec1;
        double distance = dir.mag_square();

        int begin = 0;
        int end  = obj_list.size();

        while(begin!=end)
        {            
          Object *obj = obj_list[begin];

            if(obj->object_id != obj_id) {

            	Intersection nearest_intersection;
            	if (obj->find_nearest_Intersection(vec1, dir.unit_vector(), nearest_intersection) != 0)
            	{   	        
                	if (nearest_intersection.distanceSquared < distance)
                	{
                    	return false;
                	}
            	}
            }
          begin++;
        }

        /* Path from point to light is clear */
        return true;  
}

Color Scene::DOF_TraceRay(const Vector& vec_origin,const Vector& vec_dir, Color& ray_intensity, int recursion_depth, double depth_of_field)
{

	Color final_color(0.0,0.0,0.0);
	double pix_h = camera.get_pixel_height();

	Vector vec_origin_arr[DOF_NUM_RAYS];
	Vector vec_dir_arr[DOF_NUM_RAYS];

	vec_origin_arr[0] = vec_origin;
	vec_dir_arr[0] = vec_dir;

	Vector dest_point(0.0,0.0,0.0);
	dest_point += vec_origin_arr[0];
	dest_point += (vec_dir_arr[0] * depth_of_field); 

	for(int i=0;i<DOF_NUM_RAYS;i++) {	

		if(i!=0)
		{
			double rand_num = pix_h * (rand()%60 - 30);
			vec_origin_arr[i] += vec_origin; 
			vec_origin_arr[i] += Vector(0.0,rand_num,0.0);

			vec_dir_arr[i] += dest_point;
			vec_dir_arr[i] -= vec_origin_arr[i];
			vec_dir_arr[i] = vec_dir_arr[i].unit_vector();
		}	

	    Intersection inter;
	    Color my_color(0.0,0.0,0.0);
		int result = find_nearest_Intersection(vec_origin_arr[i],vec_dir_arr[i],inter);

		switch(result) {

			case 0: 	/* No intersecting object found, ray need not be traced anymore*/
						my_color.ColorProduct(backgroundColor,ray_intensity);
						final_color += my_color;
						break;

			case 1:    /* Compute the color of the pixel */	
					   final_color += GetColor(inter,vec_dir,ray_intensity,recursion_depth+1);
					   break;

			default:   cerr << "Ray intersects with more than 1 point at same distance" << endl;
					   final_color += Color(0.0,0.0,0.0); 			   
		}

	}

	if(DOF_NUM_RAYS != 0) {
		final_color = (final_color)/(DOF_NUM_RAYS);
	}

	check_color(final_color);

	return final_color;

}


Color Scene::TraceRay(const Vector& vec_origin, const Vector& vec_dir, Color& ray_intensity, int recursion_depth)
{
		Intersection inter;
		int result = find_nearest_Intersection(vec_origin,vec_dir,inter);
		Color final_color;

		switch(result) {

			case 0: 	/* No intersecting object found, ray need not be traced anymore*/
						final_color.ColorProduct(backgroundColor,ray_intensity);
						return final_color;

			case 1:    /* Compute the color of the pixel */	
					   final_color = GetColor(inter,vec_dir,ray_intensity,recursion_depth+1);
					   return final_color;				
			default:   cerr << "Ray intersects with more than 1 point at same distance" << endl;
					   return Color(0.0,0.0,0.0); 			   
		}

}

Color Scene::GetColor(const Intersection& inter, const Vector& vec_dir, Color& ray_intensity, int recursion_depth)
{

   Color color = inter.obj->getcolor();
   Color ambientColor = getAmbientLighting(inter, color);
   Color diffuseAndSpecularColor = getDiffuseAndSpecularLighting(inter,vec_dir,color);

   Color reflectedColor(0.0,0.0,0.0);

   if(recursion_depth <= MAX_RECURSION) {
   	reflectedColor += Reflection(inter,vec_dir,ray_intensity,recursion_depth+1);
   }	

   Color final_color(0.0,0.0,0.0);
   final_color += ambientColor;
   final_color += diffuseAndSpecularColor;
   final_color += reflectedColor;

   check_color(final_color);

   return final_color;
}

static void check_color(Color& final_color)
{
	if(final_color.r > 1.0)
	{
		final_color.r = 1.0;
	}

	if(final_color.r < 0.0)
	{
		final_color.r = 0.0;
	}

	if(final_color.g > 1.0) 
	{
		final_color.g = 1.0;
	}

	if(final_color.g < 0.0) 
	{
		final_color.g = 0.0;
	}

	if(final_color.b > 1.0)
	{
		final_color.b = 1.0;
	}

	if(final_color.b < 0.0)
	{
		final_color.b = 0.0;
	}

}

Color Scene::getAmbientLighting(const Intersection& inter, const Color& color)
{
	Color my_color = color;
	return (my_color * 0.2);
}

Color Scene::getDiffuseAndSpecularLighting(const Intersection& inter, const Vector& vec_dir, const Color& color)
{

   Color diffuseColor(0.0, 0.0, 0.0);
   Color specularColor(0.0, 0.0, 0.0);
   Color result_Color(0.0, 0.0, 0.0);
   Color my_color = color;
   /*Shadow ray towards light source*/
		for(int i=0;i<light_list.size();i++) {
			
			Vector light_pos,light_vec,dummy;
			light_pos = light_list[i].location;
			light_vec = light_pos - inter.point;
			double check_val = dummy.DotProduct(inter.surfaceNormal,light_vec.unit_vector());
			
			if(check_val < 0.0) {
				check_val = 0.0;
			}	

			//Find bisector value for specular shading//
			 Vector v_opp(0.0,0.0,0.0);
			 v_opp -= vec_dir;
			 Vector h_temp(0.0,0.0,0.0);
			 h_temp += v_opp;
			 h_temp += light_vec;
			 h_temp = h_temp.unit_vector();

			 double spec_val = h_temp.DotProduct(inter.surfaceNormal,h_temp); 

			 if(spec_val < 0.0) {
				spec_val = 0.0;
			 }	

			if(check_surface_to_light(inter.point,light_list[i].location,inter.obj->object_id)) {
				
				Color light_factor_temp(0.0,0.0,0.0);
				light_factor_temp = light_list[i].color;
				
				/* Diffuse Shading */
				Color diff_factor_temp(0.0,0.0,0.0);
				diff_factor_temp = (my_color * check_val);
				diffuseColor += diff_factor_temp;
				diffuseColor.ColorProduct(diffuseColor,light_factor_temp);

				/* Specular Shading */ 
				Color spec_factor_temp(0.0,0.0,0.0);
				spec_factor_temp.r = (light_factor_temp.r * pow(spec_val,32));
				spec_factor_temp.g = (light_factor_temp.g * pow(spec_val,32));
				spec_factor_temp.b = (light_factor_temp.b * pow(spec_val,32));
				specularColor += spec_factor_temp;

			} else {
					continue;
			}
				
		}

   result_Color += diffuseColor;
   result_Color += specularColor; 		
   
   return result_Color;

}

Color Scene::Reflection(const Intersection& inter, const Vector& incident_dir, Color& ray_intensity,int recursionDepth)
{

		double reflectivity_factor = inter.obj->getreflectivity();
		Vector vec;
		Vector normal = inter.surfaceNormal;
		double perp = 2.0 * vec.DotProduct(incident_dir,normal);
		Vector reflectDir = incident_dir;
		reflectDir -= (normal * perp);

		Vector new_point = inter.point;
		Vector perturb = (reflectDir * epsilon);
		new_point += perturb;
     	return TraceRay(new_point,reflectDir,ray_intensity,recursionDepth) * reflectivity_factor;

}

void Scene::image_ppm(const char* filename, const uint32_t& width, const uint32_t& height)
{

	FILE *fp = NULL;
	fp = fopen(filename, "wb");

	Framebuffer framebuffer(width,height);

	/* Shoot ray to from camera center to every pixel */
	Vector my_ray;
	Color pixel_color;
	Color ray_intensity(1.0,1.0,1.0);

	for (uint32_t row=0; row < height; row++) 
	{
		for(uint32_t col=0; col < width; col++)
		{
			my_ray = camera.compute_pixel_vector(row,col);

			#ifdef DOF_ENABLED
			pixel_color = DOF_TraceRay(camera.get_camera_center(),my_ray,ray_intensity,0,dof_val);
			#else
			pixel_color = TraceRay(camera.get_camera_center(),my_ray,ray_intensity,0);
			#endif

			framebuffer.set_pixel_data(row,col,pixel_color);
		}
	}

	(void) fprintf(fp, "P6\n%d %d\n255\n", width, height);
  	
    	for (uint32_t row=0; row < height; row++) 
		{
			for(uint32_t col=0; col < width; col++)
			{
      			static unsigned char color[3];
      			uint32_t temp = (framebuffer.get_pixel_data(row,col)).ToUInt32();
      			color[0] =  (temp >> 16) & 0xFF; //Red
      			color[1] = 	(temp >> 8) & 0xFF; //Green
      			color[2] =  (temp) & 0xFF; //Blue
      			(void) fwrite(color, 1, 3, fp);
   	    	}
  		}
  (void) fclose(fp);

}

void removeWhitespace(std::string& str) {
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\n' || str[i] == '\t') {
            str.erase(i, 1);
            i--;
        }
    }
}

struct sphere_obj{

	double radius;
	double reflec;
	Vector center;
	Color color;
	char* tex_file;

	sphere_obj() : radius(1.0), reflec(0.0), center(Vector(0.0,0.0,0.0)), color(Color(1.0,1.0,1.0)), tex_file(NULL) {}

};

struct plane_obj{

	double width;
	double length;
	double reflec;
	Vector center;
	Vector normal;
	Vector headup;
	Color color;
	char* tex_file;

	plane_obj() : width(1.0), length(1.0), reflec(0.0), center(Vector(0.0,0.0,0.0)), normal(Vector(0.0,1.0,0.0)), headup(Vector(0.0,1.0,0.0)), color(Color(1.0,1.0,1.0)), tex_file(NULL) {}

};

struct light_obj{

	Vector location;
	Color color;

	light_obj() : location(Vector(0.0,10.0,-5.0)), color(Color(1.0,1.0,1.0)) {}

};


int main(int argc, char *argv[]) 
{

	const int arg_count = argc;
	char* file_ptr = NULL;
	double camera_dim = 0;

	vector<sphere_obj> sphere_list; 
	vector<plane_obj> plane_list;
	vector<light_obj> light_list;

	char camera_str_ptr[] = "camera";
	char sphere_str_ptr[] = "sphere";
	char plane_str_ptr[] = "plane";
	char light_str_ptr[] = "light";
	char dimension_str_ptr[] = "dimension";
	char center_str_ptr[] = "center";
	char location_str_ptr[] = "location";
	char color_str_ptr[] = "color";
	char reflec_str_ptr[] = "reflectivity";
	char normal_str_ptr[] = "normal";
	char headup_str_ptr[] = "headup";
	char texture_str_ptr[] = "texture";

	bool sphere_flag = false;
	bool plane_flag = false;
	bool light_flag = false;

	static int sphere_count_obj = 0;
	static int plane_count_obj = 0;
	static int light_count = 0;

	if(arg_count > MAX_ARGUMENTS)
	{
		cout << "The no. of arguments to this program exceeds the limit of " << MAX_ARGUMENTS << " ,try again with less arguments" << endl;
		return 0;
	} 


	for(int i=1;i<arg_count;i++) {
		file_ptr = argv[i];
		cout << "Argument passed is file : " << file_ptr << endl;
	}

	string line;
	string filename;
    ifstream myfile (file_ptr);

    if (myfile.is_open())
    {
        while ( myfile.good() )
        {
            getline (myfile,line);
            //cout << line << endl;

            if(line.empty())
            {
            	continue;
            }	

            removeWhitespace(line);
            istringstream iss(line);

    		do
    		{
        		string sub;
        		iss >> sub;

        		if(sub.empty()) {
        			continue;
        		}

        		//cout << "Substring: " << sub << endl;

        		char* sub_ptr = &sub[0];

        		if (strcmp(sub_ptr,camera_str_ptr) == 0)
        		{
        			iss >> sub;
        			camera_dim = stod(sub); 
        		}

        		if(strcmp(sub_ptr,sphere_str_ptr) == 0)
        		{

        			sphere_list.push_back(sphere_obj());
        			sphere_flag = true;
        			plane_flag = false;
        			light_flag = false;
        			sphere_count_obj++;
        		}

        		if(strcmp(sub_ptr,plane_str_ptr) == 0)
        		{

        			plane_list.push_back(plane_obj());
        			sphere_flag = false;
        			plane_flag = true;
        			light_flag = false;
        			plane_count_obj++;
        		}

        		if(strcmp(sub_ptr,light_str_ptr) == 0)
        		{

        			light_list.push_back(light_obj());
        			sphere_flag = false;
        			plane_flag = false;
        			light_flag = true;
        			light_count++;
        		}

        		if(strcmp(sub_ptr,dimension_str_ptr) == 0)
        		{
        			if(sphere_flag == true && plane_flag == false)
        			{
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].radius = stod(sub);
        				//cout << sphere_list[sphere_count_obj].radius << endl;
        			} 

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].width = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].length = stod(sub);
        				//cout << plane_list[plane_count_obj].length << endl;
        			}
        		}


        		if(strcmp(sub_ptr,center_str_ptr) == 0)
        		{
        			if(sphere_flag == true && plane_flag == false)
        			{
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].center.x = stod(sub);
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].center.y = stod(sub);
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].center.z = stod(sub);

        				//sphere_list[sphere_count_obj-1].center.display_vector();
        			} 

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].center.x = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].center.y = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].center.z = stod(sub);

        				//plane_list[plane_count_obj-1].center.display_vector();
        			}
        		}

        		if(strcmp(sub_ptr,color_str_ptr) == 0)
        		{
        			
        			if(!light_flag) {
        			
        			if(sphere_flag == true && plane_flag == false)
        			{
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].color.r = stod(sub);
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].color.g = stod(sub);
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].color.b = stod(sub);

        				//sphere_list[sphere_count_obj-1].color.display_color();
        			} 

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].color.r = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].color.g = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].color.b = stod(sub);

        				//plane_list[plane_count_obj-1].color.display_color();
        			}

        			} else {
        				iss >> sub;
        				light_list[light_count-1].color.r = stod(sub);
        				iss >> sub;
        				light_list[light_count-1].color.g = stod(sub);
        				iss >> sub;
        				light_list[light_count-1].color.b = stod(sub);

        				//light_list[light_count-1].color.display_color();
        			}	

        		}

        		if(strcmp(sub_ptr,location_str_ptr) == 0)
        		{

        			iss >> sub;
        			light_list[light_count-1].location.x = stod(sub);
        			iss >> sub;
        			light_list[light_count-1].location.y = stod(sub);
        			iss >> sub;
        			light_list[light_count-1].location.z = stod(sub);

        			//light_list[light_count-1].location.display_vector();
        		}

        		if(strcmp(sub_ptr,reflec_str_ptr) == 0)
        		{

        			if(sphere_flag == true && plane_flag == false)
        			{
        				iss >> sub;
        				sphere_list[sphere_count_obj-1].reflec = stod(sub);
        				
        			} 

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].reflec = stod(sub);
        				
        			}

        		}	

        		if(strcmp(sub_ptr,texture_str_ptr) == 0)
        		{

        			if(sphere_flag == true && plane_flag == false)
        			{
        				iss >> sub;
        				filename = sub;
        				sphere_list[sphere_count_obj-1].tex_file = &filename[0];
        			} 

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				filename = sub;
        				plane_list[plane_count_obj-1].tex_file = &filename[0];
  						//cout << plane_list[plane_count_obj-1].tex_file << endl;	
        			}

        		}	

        		if(strcmp(sub_ptr,normal_str_ptr) == 0)
        		{

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].normal.x = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].normal.y = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].normal.z = stod(sub);
        			}

        		}	

        		if(strcmp(sub_ptr,headup_str_ptr) == 0)
        		{

        			if(sphere_flag == false && plane_flag == true)
        			{
        				iss >> sub;
        				plane_list[plane_count_obj-1].headup.x = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].headup.y = stod(sub);
        				iss >> sub;
        				plane_list[plane_count_obj-1].headup.z = stod(sub);
        			}

        		}	

		    } while (iss);
            
        }

        myfile.close();
    }
    else 
    {	
    	cout << "Unable to open file!!" << endl; 
    	return 0;
    }
    
	
    /* After file parser */
	static int num_obj_scene = sphere_count_obj + plane_count_obj;	

	for(int t=0;t<DEPTH_ITER_LIMIT;t++)
	{	

	for(int k=0;k<NUM_CUBE_FACES;k++)
	{	

	int curr_obj_scene = 0;	
	int curr_cube_face = k;

	Vector U_Vec(0.0,0.0,0.0);
	Vector W_Vec(0.0,0.0,0.0);
	
	switch(curr_cube_face)
	{

		case 0: /* Front Face */
				U_Vec = Vector(0.0,1.0,0.0);
				W_Vec = Vector(0.0,0.0,-1.0);
				break;
		case 1: /* Right Face */
				U_Vec = Vector(0.0,1.0,0.0);
				W_Vec = Vector(1.0,0.0,0.0);
				break;	
		case 2: /* Left Face */
				U_Vec = Vector(0.0,1.0,0.0);
				W_Vec = Vector(-1.0,0.0,0.0);
				break;
		case 3: /* Back Face */
				U_Vec = Vector(0.0,1.0,0.0);
				W_Vec = Vector(0.0,0.0,1.0);
				break;
		case 4: /* Top Face */
				U_Vec = Vector(0.0,0.0,1.0);
				W_Vec = Vector(0.0,1.0,0.0);
				break;	
		case 5: /* Bottom Face */
				U_Vec = Vector(0.0,0.0,-1.0);
				W_Vec = Vector(0.0,-1.0,0.0);
				break;

		default: cout << "Something wrong in iteration for cubeface!!" << endl; 
    			 return 0;		
	}

	//Camera_Setup my_cam(Vector(0.0,0.0,0.0),Vector(0.0,1.0,0.0),Vector(1.0,0.0,0.0),1.0,camera_dim,camera_dim,90,90);
	Camera_Setup my_cam(Vector(0.0,0.0,0.0),U_Vec,W_Vec,1.0,camera_dim,camera_dim,90,90);
	Scene my_scene(my_cam,Color(0.0,0.0,0.0));

    for(int i=0;i<sphere_list.size();i++)
    {
    	if(curr_obj_scene <= num_obj_scene)
    	{
    		curr_obj_scene++;
    	}

    	Sphere* my_sphere = new Sphere(sphere_list[i].center, sphere_list[i].radius, sphere_list[i].color, sphere_list[i].reflec, curr_obj_scene);
    	my_scene.add_Object(my_sphere);

    	if(sphere_list[i].tex_file != NULL)
    	{
    		my_sphere->addTexture(&(sphere_list[i].tex_file)[0]);
    	}
    }	

    
    for(int i=0;i<plane_list.size();i++)
    {
    	if(curr_obj_scene <= num_obj_scene)
    	{
    		curr_obj_scene++;
    	}

    	Plane* my_plane = new Plane(plane_list[i].center, plane_list[i].length, plane_list[i].width, plane_list[i].normal, plane_list[i].headup, plane_list[i].color, plane_list[i].reflec, curr_obj_scene);
    	my_scene.add_Object(my_plane);

    	if(plane_list[i].tex_file != NULL)
    	{
    		cout << plane_list[i].tex_file << endl;
    		my_plane->addTexture(&(plane_list[i].tex_file)[0]);
    	}
    }	

    for(int i=0;i<light_list.size();i++)
    {
    	/* Light source */
    	my_scene.add_Light_Source(Light_Source(light_list[i].location,light_list[i].color));

    }	

    char file_ext[10] = {};
    char new_filename[100] = {};
    string depth_val = to_string((int)dof_val);
    string cube_face_num = to_string(k);
    char* d_val = &depth_val[0];
    char* cf_val = &cube_face_num[0]; 

    for(int i=0;i<strlen(file_ptr);i++) {

    	if(file_ptr[i] == '.') {
    		break;
    	}

    	new_filename[i] = file_ptr[i];
    }

  	strcpy(file_ext,".ppm");
  	strcat(new_filename,"_");
  	strcat(new_filename,d_val);
  	strcat(new_filename,"_");
  	strcat(new_filename,cf_val);
  	strcat(new_filename,file_ext);

    my_scene.image_ppm(new_filename, camera_dim, camera_dim);
    cout << "Done with Ray Tracing! Output generated in file: "  << new_filename << endl;

    d_val = NULL;
    cf_val = NULL;

   } 

   dof_val += 4.0;

  } 
}
