CC      = g++ -g
X11	= /usr/X11R6/
IFLAGS  = -I$(X11)/include 
LFLAGS  = -L$(X11)/lib 
LIBS    = -lglut -lGLU -lGL -lSM -lICE -lXmu -lXext -lXi -lX11 -lm
OBJECTS = 
HEADERS = 

.c.o: 
	$(CC) $(IFLAGS) -c $<

sample: $(OBJECTS) $(HEADERS)
	$(CC) scene.cpp -o my_raytracer
