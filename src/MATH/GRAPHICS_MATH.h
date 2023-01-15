#include <Arduino.h>


class  GRAPHICS_MATH
{
  
public:
  

    //MATRICIES////////////////////////////////////////////
     void mat4x4(float mat[4][4], float vertex[3]);
     void translate_Matrix(float dx, float dy, float dz, float vertex[3]);
     void scaling_Matrix(float sf, float vertex[3]);
     void x_Rotation_Matrix(float anx, float vertex[3]);
     void y_Rotation_Matrix(float any, float vertex[3]);
     void z_Rotation_Matrix(float anz, float vertex[3]);
     //////////////////////////////////////////////////////



     void normalize(int xi, int xMin, int xMax, float* ui);
     void normalise_Range_To_1_0_1(float xi, float xmin, float xmax, float* xn);
     void interp(int X1, int X2, float P, int* PX);
     void point_Liner_Interpolation(int X1, int Y1, int X2, int Y2, float P, int* PX, int* PY);
     void Swap_XY(int x_In1, int y_In1, int x_In2, int y_In2, int* x_Out1, int* y_Out1, int* x_Out2, int* y_Out2);
     byte check_Line_Cross_p1_p3(int x1, int y1, int x3, int y3, int x2, int y2);
     void rotate_Point(int* x_Orign, int* y_Orign, float AN, bool direction);
     void get_Point_Vector(int PX1, int PY1, int R, float AN, bool rotation_Dir, int* px2, int* py2);
     void angled_Point_Step(float angle, float step_Size, int x1, int y1, int* X_Next, int* Y_Next);
     void map_Value_To_Range(float Vi, float aMin, float aMax, float bMin, float bMax, int *Vn);
     float deg_To_Rad(float Ai);
     void triangle_CW_Sort
     (
         int &x1,
         int &y1,
         int &x2,
         int &y2,
         int &x3,
         int &y3
     );


     void world_plane_To_Screen_Plane_Vertex_Translate
     (
         float wp1_x,
         float wp1_y,
         float wp2_x,
         float wp2_y,
         float sp1_x,
         float sp1_y,
         float sp2_x,
         float sp2_y,
         float vertex[3],
         int* Xn,
         int* Yn


     );

     void vector_Cross_Product
     (
         float Va[3], 
         float Vb[3],
         float Vn[3]

     );


     void vector_Dot_Product
     (
         float Va[3],
         float Vb[3],
         float *Dp
     );
     ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};




