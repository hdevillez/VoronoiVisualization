#include "fortune.h"
#include "draw_tools.h"
#include "draw.h"
#include "getopt.h"
#include <dirent.h>
#include <errno.h>

extern bool verbose; 
extern bool debug; 

extern float simulation_time;
extern float step_time;

extern const int NO_ANIMATION;
extern const int CONTINUOUS_ANIMATION;
extern const int STEP_ANIMATION;

extern bool DRAW_CIRCLE_OPTION;
extern bool DRAW_BEACHLINE_OPTION;
extern bool DRAW_SWEEPLINE_OPTION;
extern bool DRAW_CELL_OPTION;

extern bool gif_flag;
extern char * gif_dir;

int main(int argc, char *argv[])
{
    
    int c;
    int animation_type = CONTINUOUS_ANIMATION;
    
    while ((c = getopt (argc, argv, "a:c:C:s:b:d:v:t:g:")) != -1) {
        switch (c) {
            case 'd':
                debug = atoi(optarg); 
                break;
            case 'v':
                verbose = atoi(optarg);
                break;
            case 'a':
                if(atoi(optarg) == 0)
                    animation_type = NO_ANIMATION;
                else if(atoi(optarg) == 1)
                    animation_type = CONTINUOUS_ANIMATION;
                else if(atoi(optarg) == 2)
                    animation_type = STEP_ANIMATION;
                else
                    printf("Error : animation type not recognized\n");
                break;
            case 'c':
                DRAW_CIRCLE_OPTION = atoi(optarg);
                break;
            case 'C':
                DRAW_CELL_OPTION = atoi(optarg);
                break;
            case 'b':
                DRAW_BEACHLINE_OPTION = atoi(optarg);
                break;
            case 's':
                DRAW_SWEEPLINE_OPTION = atoi(optarg);
                break;
            case 't':
                simulation_time = atof(optarg);
                step_time = atof(optarg);
                break;
            case 'g':
                gif_flag = true;
                gif_dir = optarg;

                // check if directory exists
                opendir(gif_dir);
                if (ENOENT == errno)
                {
                    printf("Directory %s does not exists;  Gif won't be saved \n", gif_dir);
                    gif_flag = false; 
                }
                break;
            case '?':
                break;
            default:
                abort ();
        }
    }

    window_t * window = window_new(600,600, argv[0]);
	
	window_set_color(window, (float[4]) {1, 1, 1, 1.0});
    // allocations
    int n_points = 11;
    float coords[22] = {0.0, 0.6,
                       0.55, 0.375,
                        0.55, 0.425,
                       -0.4, 0.201,
                       -0.5, 0.002,
                        0.3, 0.002,
                        0.5, 0.002,
                       -0.2, -0.4,
                        0.5, -0.6,
                        -0.1, -0.6,
                        0.1, -0.6};
  
    



    fortune_algorithm(window, coords, n_points, animation_type);

    window_delete(window);
    
    return EXIT_SUCCESS;
}
