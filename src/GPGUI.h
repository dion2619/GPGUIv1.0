#include <vector>
using namespace std;


#include <Arduino.h>
#include "key_Codes.h"
#include "debug_Flags.h"
#include "shaded_Colors.h"

struct vec3D
{
    //TAIL
    float Ax;
    float Ay;
    float Az;

    //HEAD
    float Bx;
    float By;
    float Bz;

    //LENGTH
    float Vl;
    float An;
};

struct vertex3D
{
    float x;
    float y;
    float z;
    float u;
    float v;
    int   c;
    int  id;
    int mesh_Type;

};


struct line
{
    vertex3D p1,p2;
    int id;
    int c;
};

struct triangle
{
    vertex3D 
        p1,
        p2,
        p3;   
        int face_Color;
        int id;
        float Nx;
        float Ny;
        float Nz;
        float Dp_Camera;
        float Dp_Light;
       
};


class  GPGUI

{
  
public:
    GPGUI();//constructor

    void clear_EEPROM();
    int read_Keyboard_Port();
    byte read_EXP_Port();
    void beep_Off();
    void GPGUI_INIT();
    void refreshScreen();










     //HARDWARE CONTROLLS
     void sendCommand_EXP_Port
     (
         byte comand, 
         byte data1,
         byte data2
     );



     void SendCommandI2c
     (
         byte i2cAddress,
         byte command,
         byte A,
         byte B,
         byte C
     );


     void set_HDD_Led
     (
         bool state
     );

   

     void read_Midi_Port
     (
         byte* control,
         byte* data1,
         byte* data2
     );

     void send_Midi_Message
     (
         byte control, 
         byte data1,
         byte data2
     );

    

     void beep
     (
         int pitch,
         int  dur
     );

     void beep_On
     (
         int note
     );

    

     //VGA/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    

     void draw_Color_Map(bool run);

     void draw_Color_Map_Shaded(bool run);

     void color_Pellet_Program();

     int get_Color_Shade(int p_Color, int s_Color);

     void drawTriangle
     (
         int x1,
         int y1n,
         int x2,
         int y2,
         int x3,
         int y3,
         byte fill_Type,
         byte color
     );

     void drawTriangle_Pipeline
     (
         int x1,
         int y1n,
         int x2,
         int y2,
         int x3,
         int y3,
         byte fill_Type,
         byte color
     );

    
     void setPixel
     (
         int x,
         int y, 
         byte color
     );

     void drawLine
     (
         int x1,
         int y1,
         int x2,
         int y2,
         byte color
     );

     void drawHollowCircle
     (
         int x,
         int y,
         int radius,
         byte color
     );

     void drawSolidCircle
     (
         int x,
         int y,
         int r,
         byte color
     );

     void drawSolidRectangle
     (
         int xStart,
         int yStart,
         int xSize,
         int ySize,
         byte color
     );

     void drawHollowRectangle
     (
         int xStart, 
         int yStart,
         int xSize,
         int ySize,
         byte color
     );

     void drawBackgroundColor
     (
         byte color
     );

     void printNumberDouble
     (
         double inputData,
         int x,
         int y, 
         byte textColor,
         byte backColor
     );

     void printNumberInt
     (
         int inputData, 
         int x, 
         int y,
         byte textColor,
         byte backColor
     );

     void print
     (
         const char* inputData,
         int x,
         int y,
         byte textColor,
         byte backColor
     );

     byte readVGAscreenBufferCell
     (
         int x,
         int y,
         bool debugPrint
     );

     void printNumberHex
     (
         int inputData, 
         int x,
         int y,
         byte textColor,
         byte backColor
     );

    // void  draw_Perspective_Grid(int x_Center, int n_Lines, int NP_Offset, int DY, int DSN, int DSF, byte grid_Color, int* p1_x, int* p1_y, int* p2_x, int* p2_y, int* p3_x, int* p3_y, int* p4_x, int* p4_y);
     void DDA_Line
     (
         int X1,
         int Y1,
         int X2,
         int Y2,
         byte color
     );


     void DDA_Line_Clip
     (
         int X1,
         int Y1,
         int X2,
         int Y2,
         byte color,
         int xStart,
         int yStart,
         int window_Size_X,
         int window_Size_Y

     );


     void draw_Vector
     (
         int PX1,
         int PY1,
         int R,
         float AN, 
         bool rotation_Dir,
         int* px2,
         int* py2,
         int color
     );

     void draw_Quad
     (
         float p1_X, 
         float p1_Y,
         float p2_X,
         float p2_Y,
         float p3_X,
         float p3_Y,
         float p4_X,
         float p4_Y,
         bool fill_Type,
         byte color
     );

     void sine_Wave_Bounce
     (
         float* Xpos, 
         float* Ypos,
         bool dir,
         int height,
         float X_Distance,
         float start_Speed,
         float velocity,
         unsigned long currentMillis, 
         bool enable,
         bool stop,
         bool* state
     );

     void read_PS2_Controller
     (
         int* button,
         int* state
     );

  

     void SCC_Note_On
     (
         byte channel,
         byte note, 
         byte velocity
     );

     void SCC_Note_Off
     (
         byte channel,
         byte note
     );

     

     void map_Scanline_From_Texture
     (
         int x1,
         int x2,
         int yi,
         float texXstart,
         float texXend,
         float texYstart, 
         byte tex[32][32]
     );





     void QUAD_DEBUG_PROGRAM();
     void texture_Scan_Line_Maping_Debug_Program();
     void DEFAULT_PROGRAM();
     void screen_Space_Texture_Mapping_Program();

     void draw_Perspective_Plane
     (
         int xStart,
         int yStart,
         int screen_Width,
         int screen_Height,
         int zFar_Offset,
         int zNear_Offset, 
         int zFar_Plane_Length,
         int zNear_Plane_Length, 
         byte tex[32][32]
     );





























     //3D ENGINE
     void translate_XY_Veiw_Plane_To_Screen_Plane
     (
         int xStart,
         int yStart,
         int window_Width,
         int window_Height,
         float camera_X,
         float camera_Y,
         float camera_Z,
         byte space_Color,
         int *vertex_Counter,
         vector <vertex3D>& to_Be_Translated_Buffer,
         vector <vertex3D>& Translated_Buffer
     );

     void vertex_Buff_2D_Parallel_Projection_Pipeline
     (
         int xStart,
         int yStart,
         int screen_Plane_Size,
         float cameraX,
         float cameraY,
         float cameraZ,
         int space_Color,
         int *v_Count,
         int *z_Cliped,
         vector <vertex3D>& world_Vertex_Buffer
     );

     void pixel_Bypass
     (
         bool enable,
         bool z_Clip,
         int *z_Cliped,
         int window_Clip_Size,
         int window_Start_X,
         int window_Start_Y,
         vector<vertex3D>& buffer
     );


   

     void Vertex_Pipeline_Program(bool run);


     void write_Origin_Cross_Mesh_To_Vector
     (
         vector<vertex3D>& mesh,
         float  origin_Cross_Size,
         bool full_Cross, int* vc
     );

     void write_Clusteter_Cube_Mesh_To_Vector
     (
         int Q,
         float cube_Size,
         float Xo,
         float Yo,
         float Zo,
         int fill_Type,
         vector<vertex3D>& buffer
     );




     void rotate_Mesh
     (
         vector<vertex3D>& buffer,
         int axis,
         float angle_Delta
     
     );


     void draw_Axis_Dials
     (
         int x_Start,
         int y_Start,
         float X,
         float Y,
         float Z
     );


     void XY_Plane_Debug_Program();



     void line_Gen_3D
     (
         float p1x,
         float p1y,
         float p1z,
         float p2x,
         float p2y,
         float p2z,
         int color,
         int id,
         vector<vertex3D>& world_Space_Vertex
     );


     void line_Decoder
     (
         bool enable,
         int screen_Plane_Size,
         int xStart,
         int yStart,
         vector<vertex3D>& screen_Space_pixel_Buffer
     );


     void triangle_Gen_3D
     (
         float p1x,
         float p1y,
         float p1z,
         float p2x,
         float p2y,
         float p2z,
         float p3x,
         float p3y,
         float p3z,
         float U,
         float V,
         int color,
         int id,
         vector<vertex3D>& world_Space_Vertex

     );

     void triangle_Decoder
     (
         bool enable,
         int screen_Plane_Size,
         int xStart,
         int yStart,
         bool fill_Type,
         vector<vertex3D>& screen_Space_pixel_Buffer
     );


     void translate_Cube_Vertex_Buff_To_3D_Triangles
     (
         float cube_Buff[8][3],
         vector<vertex3D>& world_Space_Vertex_Buffer
     );




     void triangle_Cube_Program();

     void translate_Triangle_Buffer_To_Pixel_Buffer
     (
         int xStart,
         int yStart,
         int window_Width,
         int window_Height,
         float camera_X,
         float camera_Y,
         float camera_Z,
         bool back_Face_Cliping_Enable,
         vector<triangle> &triangle_Buffer,
         vector<triangle> &pixel_Buffer
     );



     void rotate_Triangle_Vector
     (
         vector<triangle>& triangle_Buffer,
         int axis,
         float An
     );

     void rotate_World_Vertex
     (
         bool x_Axis_Rotate_Enable,
         bool y_Axis_Rotate_Enable,
         bool z_Axis_Rotate_Enable,
         bool x_Dir,
         bool y_Dir,
         bool z_Dir,
         float x_Angle_Delta,
         float y_Angle_Delta,
         float z_Angle_Delta,
         vector <vertex3D>& buff
     );


     void rotate_Obj_Triangle
     (
         bool x_Axis_Rotate_Enable,
         bool y_Axis_Rotate_Enable,
         bool z_Axis_Rotate_Enable,
         bool x_Dir,
         bool y_Dir,
         bool z_Dir,
         float x_Angle_Delta,
         float y_Angle_Delta,
         float z_Angle_Delta,
         vector <triangle>& buff
     );
     void move_Background_Vertex_Buffer_To_World_Vertex_Buffer
     (
         vector<vertex3D>& back_Buff,
         vector<vertex3D>& world_Buff

     );

     void move_Obj_Triangle_Buffer_To_World_Triangle_Buffer
     (
         vector<triangle>& back_Buff,
         vector<triangle>& world_Buff

     );

     void translate_Cube_Vertex_Buff_To_Triangle_Buff//vertex version
     (
         float cube_Buff[8][3],
         vector<triangle>& buff
        
     );


     void triangle_Buff_To_Screen

     (
         int window_Start_X,
         int window_Start_Y,
         int window_Size,
         float camera_X,
         float camera_y,
         float camera_z,
         float light_Vector_X,
         float light_Vector_Y,
         float light_Vector_Z,
         bool back_Face_Culling_Enable,
         bool light_Enable,
         bool wire_Frame_Enable,
         int* triangle_Count,
         vector<triangle>& triangle_Buffer
     );
     

     void set_Light_Vector_Dir
     (
         float Xan,
         float Yan,
         float Zan,
         float Vlv[3]

     );











     //RAYCAST ENGINE
     void write_Default_Wall_Texture_To_Ram
     (
         int mem_Address,
         byte tex_Ram[4096]
     );

     void write_Texture_To_Ram
     (
         int mem_Address, 
         byte tex_Ram[4096], 
         byte tex_Buffer[255]
     );

     // void draw_Tile_Map(int xStart, int yStart, byte map[4096], byte tex[4096], int cell_X, int cell_Y, int cell_HitX, int cell_HitY, int*posX, int*posY, int* hitX, int* hitY);
     void draw_2D_Tile
     (
         int xStart,
         int yStart,
         int tile_Index,
         byte tex_Ram[4096]
     );

     void raycast_Map_Preview
     (
         byte map_Ram[4096],
         byte tex_Ram[4096]
     );

     void ram_Data_Limit_Clip
     (
         byte map_Ram[4096]
     );

     void camera_Compass
     (
         int xStart,
         int yStart,
         int xPos,
         int yPos,
         int angle,
         byte back_Color
     );

     void write_Default_Sprite_Texture_To_Ram
     (
         int mem_Address, 
         byte tex_Ram[4096]
     );

     void write_Default_Map_To_Ram
     (
         byte width, 
         byte height,
         byte map_Ram[4096],
         byte wall
     );

     void draw_Tile_Map
     (
         int xStart,
         int yStart,
         byte map[4096],
         byte tex[4096],
         int cell_X, //camera cell pos
         int cell_Y,
         int cell_HitX_Left,//left ray hit point cell 
         int cell_HitY_Left,
         int cell_HitX_Center,//center ray hit point cell
         int cell_HitY_Center,
         int cell_HitX_Right,//right ray hit point cell
         int cell_HitY_Right,
         int* posX,//screen space camera and ray hit cell points
         int* posY,
         int* hitX_Left,
         int* hitY_Left,
         int* hitX_Center,
         int* hitY_Center,
         int* hitX_Right,
         int* hitY_Right
     );









     void raycast_Map_From_Ram
     (
         byte ram[4096],
         byte ram_Texture[4096],
         int xStart,
         int yStart,
         int screen_Width,
         int screen_Height,
         bool rotate_Left,
         bool rotate_Right,
         bool move_Foward,
         bool move_Back,
         bool cliping_En,
         int  ray_Clip_Point,
         bool camera_Up,
         bool camera_Down,
         float move_Speed,
         float turn_Speed,
         int* camera_Map_Pos_X,
         int* camera_Map_Pos_Y,
         int* camera_Angle_Out,
         int* hit_Cell_X_Left,
         int* hit_Cell_Y_left,
         int* hit_Cell_X_Center,
         int* hit_Cell_Y_Center,
         int* hit_Cell_X_Right,
         int* hit_Cell_Y_Right,
         int* target_Cell_Data,
         bool camera_Cross_Hair_Enable,
         bool debug
     );
     ////////////////////////////////////////////////////////////////////////////////////////





};




