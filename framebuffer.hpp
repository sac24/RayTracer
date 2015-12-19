#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include "color.hpp"

using namespace std;

struct PixelData
{
    Color color;
    //PixelData():color() {}
};

class Framebuffer
{

	private:        
        uint32_t  p_width;    
        uint32_t  p_height;     
        uint32_t  num_pixels;      
        PixelData*  pixel_array;     

    public:
    Framebuffer (uint32_t w, uint32_t h) : p_width(w), p_height(h), num_pixels(w * h)
    {
        pixel_array = new PixelData[num_pixels];
    }

    virtual ~Framebuffer()
    {
        delete[] pixel_array;
        pixel_array = NULL;
        p_width = p_height = num_pixels = 0;
    }

    Color& get_pixel_data(uint32_t row, uint32_t col)
    {
        
        return (pixel_array[row + col*p_width]).color;
        
    }

    void set_pixel_data(uint32_t row, uint32_t col, const Color& color)
	{
		if ((row < p_height) && (col < p_width))
        {
            pixel_array[row + col*p_width].color = color;
        }
        else
        {
            cerr << "Array out of bounds" << endl;
        }
	
	}

	void clear_framebuffer()
	{
		//memset(pixel_array, Color(), sizeof(PixelData) * p_width * p_height);
		for (int i=0; i<sizeof(PixelData);i++) {
			pixel_array[i].color = Color();
		}
	}

};
