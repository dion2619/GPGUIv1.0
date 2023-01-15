//14 Base colors
# define black   0  
# define white   63 
# define red     3  
# define lime    12 
# define blue    48 
# define yellow  15 
# define cyan    60 
# define magenta 51 
# define grey    42 
# define maroon  2  
# define olive   10 
# define green   8  
# define purple  34 
# define teal    40 
# define navy    32 

struct point_Mesh_3D
{
    float vertex_Buffer[20][4] =
    {
        {0,0,-20, blue},
        {1,0,-20, red},
        {2,0,-20, red},
        {3,0,-20, red},
        {4,0,-20, red},
        {5,0,-20, red},
        {6,0,-20, red},
        {7,0,-20, red},
        {8,0,-20, red},
        {9,0,-20, purple},


        {0,0,-20, blue},
        {0,1,-20, green},
        {0,2,-20, green},
        {0,3,-20, green},
        {0,4,-20, green},
        {0,5,-20, green},
        {0,6,-20, green},
        {0,7,-20, green},
        {0,8,-20, green},
        {0,9,-20, purple}

    };
};