#include <Arduino.h>
#include "GRAPHICS_MATH.h"
#include <math.h>







void GRAPHICS_MATH::normalise_Range_To_1_0_1(float xi, float xmin, float xmax, float* xn)
{
     *xn = (2 * (xi - xmin) / (xmax - xmin)) - 1;
}

void GRAPHICS_MATH::map_Value_To_Range(float Vi, float aMin, float aMax, float bMin, float bMax, int* Vn)
{
    
    float pn = (aMax - aMin) * ((Vi - bMin) / (bMax - bMin)) + aMin;
    *Vn = (int)pn;
   
    //range a bigger than range b

}

void GRAPHICS_MATH::normalize(int xi, int xMin, int xMax, float* ui)
{

    if (xi < xMin) xi = xMin;
    if (xi > xMax) xi = xMax;

    *ui = ((float)xi - (float)xMin) / ((float)xMax - (float)xMin);
}

void GRAPHICS_MATH::interp(int X1, int X2, float P, int* PX)
{
    if (P > 1) P = 1;
    if (P < 0) P = 0;


    float DX = X2 - X1;
    *PX = X1 + (DX * P);
}

void GRAPHICS_MATH::point_Liner_Interpolation(int X1, int Y1, int X2, int Y2, float P, int *PX, int *PY)//(P1   P2)
{
    float DX = X2 - X1;
    float DY = Y2 - Y1;

    *PY =  Y1 + (DY * P);
    *PX =  X1 + (DX * P);
    
}

byte GRAPHICS_MATH::check_Line_Cross_p1_p3(int x1, int y1, int x3, int y3, int x2, int y2)
{
    int pos_X;
    int pos_Y;
    static byte side = 1;

    for (float pos = 0.00f; pos < 1.00f; pos += 0.01f)
    {
        point_Liner_Interpolation(x1, y1, x3, y3, pos, &pos_X, &pos_Y);

        if (x2 <= pos_X && y2 <= pos_Y) { side = 1; }
        if (x2 >  pos_X && y2 > pos_Y)   { side = 2; }
    
    

    }

    return side;
}

void GRAPHICS_MATH::Swap_XY(int x_In1, int y_In1, int x_In2, int y_In2, int* x_Out1, int* y_Out1, int* x_Out2, int* y_Out2)
{
    *x_Out1 = x_In2;
    *y_Out1 = y_In2;
    *x_Out2 = x_In1;
    *y_Out2 = y_In1;
}

void GRAPHICS_MATH::mat4x4(float mat[4][4], float vertex[3])
{
    //ROW MAJOR FORM, LEFT TO RIGHT

    float x = vertex[0];
    float y = vertex[1];
    float z = vertex[2];
    float w = 1;



    vertex[0] =
        (x * mat[0][0]) +
        (y * mat[0][1]) +
        (z * mat[0][2]) +
        (w * mat[0][3]);


    vertex[1] =
        (x * mat[1][0]) +
        (y * mat[1][1]) +
        (z * mat[1][2]) +
        (w * mat[1][3]);

    vertex[2] =
        (x * mat[2][0]) +
        (y * mat[2][1]) +
        (z * mat[2][2]) +
        (w * mat[2][3]);
}

void GRAPHICS_MATH::translate_Matrix(float dx, float dy, float dz, float vertex[3])
{
    float translate[4][4] =
    {
        {1, 0, 0, dx},
        {0, 1, 0, dy},
        {0, 0, 1, dz},
        {0, 0, 0,  1}
    };

    mat4x4(translate, vertex);
}

void GRAPHICS_MATH::scaling_Matrix(float sf,  float vertex[3])

{
    float xsf = sf;
    float ysf = sf;
    float zsf = sf;



    float scale[4][4] =
    {
        {xsf,   0,   0, 0},
        {0,   ysf,   0, 0},
        {0,     0, zsf, 0},
        {0,     0,   0, 1}
    };
    mat4x4(scale, vertex);
}

void GRAPHICS_MATH::x_Rotation_Matrix(float anx, float vertex[3])
{
    float a = deg_To_Rad(anx);

    float rotate_X[4][4] =
    {
        {1,       0,        0,   0},
        {0,  cos(a),  -sin(a),   0},
        {0,  sin(a),   cos(a),   0},
        {0,       0,        0,   1}
    };


    mat4x4(rotate_X, vertex);

}

void GRAPHICS_MATH::y_Rotation_Matrix(float any, float vertex[3])
{
    float a = deg_To_Rad(any);
    float rotate_Y[4][4] =
    {
        { cos(a),   0, sin(a),    0},
        {      0,   1,      0,    0},
        {-sin(a),   0, cos(a),    0},
        {      0,   0,      0,    1}
    };
    mat4x4(rotate_Y, vertex);
}

void GRAPHICS_MATH::z_Rotation_Matrix(float anz, float vertex[3])
{
    float a = deg_To_Rad(anz);

    float rotate_Z[4][4] =
    {
        {   cos(a),   -sin(a),   0,   0},
        {   sin(a),    cos(a),   0,   0},
        {        0,         0,   1,   0},
        {        0,         0,   0,   1}
    };

    mat4x4(rotate_Z, vertex);

}

void GRAPHICS_MATH::rotate_Point(int* x_Orign, int* y_Orign, float AN, bool direction)
{
    float Radius = *x_Orign;//read from refrence address
    AN = AN * 3.144 / 180;



    float clockwise_Rotation_Matrix[2][2] =
    {
    {cos(AN),   -sin(AN)},
    {sin(AN),    cos(AN)}
    };

    float counter_Clockwise_Rotation_Matrix[2][2] =
    {
    {cos(AN),   sin(AN)},
    {-sin(AN),  cos(AN)}
    };


    //RUN ORIGN THRUGH ROTATION MATRIX


    if (direction)
    {
        *x_Orign = (int)Radius * counter_Clockwise_Rotation_Matrix[0][0] + counter_Clockwise_Rotation_Matrix[0][1];//
        *y_Orign = (int)Radius * counter_Clockwise_Rotation_Matrix[1][0] - counter_Clockwise_Rotation_Matrix[1][1];//
    }

    if (!direction)
    {
        *x_Orign = (int)Radius * clockwise_Rotation_Matrix[0][0] + clockwise_Rotation_Matrix[0][1];//
        *y_Orign = (int)Radius * clockwise_Rotation_Matrix[1][0] - clockwise_Rotation_Matrix[1][1];//
    }
}

void GRAPHICS_MATH::get_Point_Vector(int PX1, int PY1, int R, float AN, bool rotation_Dir, int* px2, int* py2)
{
    int PX2 = R;
    int PY2 = 0;

    rotate_Point(&PX2, &PY2, AN, rotation_Dir);
    PX2 += PX1;
    PY2 += PY1;

    *px2 = PX2;//return end point of vector
    *py2 = PY2;
}

void GRAPHICS_MATH::angled_Point_Step(float angle, float step_Size, int x1, int y1, int* X_Next, int* Y_Next)
{
    //USES LENGTH OF HYPOTENUSE AND ANGLE TO GET X AND Y STEP SIZE
    angle = 30 * 3.144 / 180;
    float XL = step_Size;
    //
    float Yinc = sin(angle) * XL;
    float Xinc = sqrt((XL * XL) - (Yinc * Yinc));
    //
    float P1X = x1;
    float P1Y = y1;
    //
    float P2X = P1X + Xinc;
    float P2Y = P1Y - Yinc;
    //
    *X_Next = (int)P2X;
    *Y_Next = (int)P2Y;

}

float GRAPHICS_MATH::deg_To_Rad(float Ai)
{
    float An = Ai * 3.144 / 180;
    return An;
}

void GRAPHICS_MATH::world_plane_To_Screen_Plane_Vertex_Translate
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
    int *Xn,
    int *Yn
    

)
{
   
    int x_Screen_Space;
    int y_Screen_Space;

    //X                                
    map_Value_To_Range(vertex[0], sp1_x, sp2_x, wp1_x, wp2_x, &x_Screen_Space);

    //Y                                //SWAPED WP1 AND WP2 TO INVERT Y AXIS ON SCREEN
    map_Value_To_Range(vertex[1], sp1_y, sp2_y, wp2_y, wp1_y, &y_Screen_Space);

    *Xn = x_Screen_Space;
    *Yn = y_Screen_Space;

}


void GRAPHICS_MATH::vector_Cross_Product
(
    float Va[3],
    float Vb[3],
    float Vn[3]
)
{
    float Ax = Va[0];
    float Ay = Va[1];
    float Az = Va[2];

    float Bx = Vb[0];
    float By = Vb[1];
    float Bz = Vb[2];

    float Vi[3];

   //WRITE NORMAL VECTOR TO Vi ADDRESS
    Vi[0] = (Ay * Bz) - (Az * By);
    Vi[1] = (Az * Bx) - (Ax * Bz);
    Vi[2] = (Ax * By) - (Ay * Bx);
   



    //CALC LENGTH OF NORMAL VECTOR
    float NL =
        sqrt
        (
            Vi[0] * Vi[0] +
            Vi[1] * Vi[1] +
            Vi[2] * Vi[2]
        );

    
    //CORMALIZE NORMAL VECTOR
    Vn[0] = Vi[0] / NL;
    Vn[1] = Vi[1] / NL;
    Vn[2] = Vi[2] / NL;


}


void GRAPHICS_MATH::vector_Dot_Product
(
    float Va[3],
    float Vb[3],
    float* Dp
)
{
    *Dp = (Va[0] * Vb[0]) + (Va[1] * Vb[1]) + (Va[2] * Vb[2]);
          
}




void GRAPHICS_MATH::triangle_CW_Sort
(
    int &x1,//call by ref
    int &y1,
    int &x2,
    int &y2,
    int &x3,
    int &y3
 
)
{
    //FIND THE HIGHEST Y
    if (y2 < y1)   Swap_XY(x2, y2, x1, y1, &x2, &y2, &x1, &y1);
    if (y3 < y1)   Swap_XY(x3, y3, x1, y1, &x3, &y3, &x1, &y1);  
    if(y2 > y3)    Swap_XY(x2, y2, x3, y3, &x2, &y2, &x3, &y3);
    
}
































