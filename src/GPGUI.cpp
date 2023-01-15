//MUST USE ESP32 ARDUINO CORE V 2.0!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



#include <vector>
#include <Arduino.h>
#include "GPGUI.h"
#include "Wire.h"
#include "WirePacker.h"
#include "UART/HardwareSerial.h"
#include "VGA6Bit.h"
#include "Font6x8.h"
#include "Tone32.h"
#include "KEYBOARD/PS2KeyAdvanced.h"
#include "EEPROM.h"
#include "MATH/GRAPHICS_MATH.h"
#include "origin_Mesh.h"
#include "cube_Mesh.h"


using namespace std;


#define EEPROM_SIZE     4096
#define DATA_PIN        34   //keyboard data
#define CLK_PIN         4    //keyboard clock
#define testLed         19
#define SDA_1           21
#define SCL_1           22
#define BUZZER_PIN      23
#define BUZZER_CHANNEL  0


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

# define ScreenWidth   320
# define ScreenHeight  240


//VGA PORT PINS
const int hsyncPin = 32;
const int vsyncPin = 33;

const int redPins[]   = { 25, 26 };    //R0,R1
const int greenPins[] = { 27, 14 };    //G0,G1
const int bluePins[]  = { 12, 13 };    //B0,B1


//float z_Buffer[240][240];



//apperance
byte desktop_Color =            black;
byte bar_Color =                 grey;
byte window_Bar_Color =         black;
byte window_Bar_Text_Color =      red;
byte border_Line_Color =        white;




//SYSTEM FLAGS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
bool system_Sound_Enable                           =  true;//
bool serialDebugEnabledFlag                        = false;//

                                                   

//GLOBAL VALUES

int keyboard_Data;
int menu_Down_Pitch  = 200;
int menu_Up_Pitch    = 100;
int enter_Beep_Pitch = 500;
int exit_Beep_Pitch  =  50;
byte COLORS[15] = { 0,63,3,12,48,15,60,51,42,2,10,8,34,40,32 };


//OBJECTS
HardwareSerial EXP_Port(1);
HardwareSerial midi_Port(2);
VGA6Bit vga;
PS2KeyAdvanced keyboard;
GRAPHICS_MATH m;
shaded_Colors cs;
 




//##############################################################################################################
//#////////////////////////////////////////////HARDWARE CONTROL////////////////////////////////////////////////#
//##############################################################################################################

GPGUI::GPGUI()//CONSTRUCTOR
{
    pinMode(testLed,OUTPUT);
    EXP_Port.begin(115200, SERIAL_8N1, 5, 18);//rx1 tx1
    midi_Port.begin(31250, SERIAL_8N1, 16, 17);//rx2 tx2
    Serial.begin(115200);
    keyboard.begin(DATA_PIN, CLK_PIN);
    keyboard.setNoBreak(false);          // No break codes for keys (when key released)
    keyboard.setNoRepeat( true);         // Don't repeat shift ctrl etc
    Wire.begin(SDA_1, SCL_1);  
}

void GPGUI::clear_EEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.writeByte(i, 0);
    }
}

void GPGUI::GPGUI_INIT()
{
    EEPROM.begin(EEPROM_SIZE);
    vga.setFrameBufferCount(2);
    vga.init(vga.MODE320x240, redPins, greenPins, bluePins, hsyncPin, vsyncPin);
}

int GPGUI::read_Keyboard_Port()
{
    
    while (keyboard.available())
    {
        keyboard_Data = keyboard.read() & 0xffff;
        Serial.println(keyboard_Data);
      
    }
 
    return keyboard_Data;
}

void GPGUI::set_HDD_Led(bool state)
{
    digitalWrite(testLed, state);
}

void GPGUI::beep(int pitch, int  dur)
{
    if (system_Sound_Enable)
    {
        tone(BUZZER_PIN, pitch);
        delay(dur);
        noTone(BUZZER_PIN);
    }
}

void GPGUI::beep_On(int note)
{
    switch (note)
    {
    case 60:  tone(BUZZER_PIN, 261); break;//C4 mid c
    case 61:  tone(BUZZER_PIN, 277); break;//C#
    case 62:  tone(BUZZER_PIN, 293); break;//D
    case 63:  tone(BUZZER_PIN, 311); break;//D#
    case 64:  tone(BUZZER_PIN, 329); break;//E
    case 65:  tone(BUZZER_PIN, 349); break;//F
    case 66:  tone(BUZZER_PIN, 369); break;//F#
    case 67:  tone(BUZZER_PIN, 392); break;//G
    case 68:  tone(BUZZER_PIN, 415); break;//G#
    case 69:  tone(BUZZER_PIN, 440); break;//A
    case 70:  tone(BUZZER_PIN, 466); break;//A#
    case 71:  tone(BUZZER_PIN, 493); break;//B
    default: break;
    } 
}

void GPGUI::beep_Off()
{
    noTone(BUZZER_PIN);
}




//COM PORTS#########################################################################

void GPGUI::SendCommandI2c
(
    byte i2cAddress,
    byte command,
    byte A,
    byte B,
    byte C
)
{
    WirePacker packer;
    packer.write(command);
    packer.write(A);
    packer.write(B);
    packer.write(C);
    packer.end();

    // now transmit the packed data
    Wire.beginTransmission(i2cAddress);
    while (packer.available())                   
    {
        Wire.write(packer.read());
    }
    Wire.endTransmission();                 
}

void GPGUI::read_Midi_Port
(
    byte* control, 
    byte* data1,
    byte* data2
)
{
    static byte m_Control;
    static byte m_data1;
    static byte m_data2;




   while (midi_Port.available())
   {

        //READ SERIAL BUFF FOR VALID CONTROL BYTE
        m_Control = midi_Port.read();
        m_data1 = midi_Port.read();
        m_data2 = midi_Port.read();

    

        if(m_Control == 144 || m_Control == 128)
            Serial.println(m_Control);
            Serial.println(m_data1);
            Serial.println(m_data2);
        *control = m_Control;
        *data1 = m_data1;
        *data2 = m_data2;
        




         

  
    }
}

void GPGUI::read_PS2_Controller
(
    int* button,
    int* state
)
{
    byte control;
    *button = 0;
    *state = 0;
    while (midi_Port.available())
    {
        set_HDD_Led(1);
        control = midi_Port.read();

        if (control == 144)
        {
            *button = midi_Port.read();
            *state = 1;
        }

        if (control == 128)
        {
            *button = midi_Port.read();
            *state = 0;
        }

        if (control == 176)
        {
            *button = midi_Port.read();
            *state =  midi_Port.read();
        }

    }
    set_HDD_Led(0);
}

void GPGUI::send_Midi_Message
(
    byte control,
    byte data1,
    byte data2
)
{
    midi_Port.write(control);
    midi_Port.write(data1);
    midi_Port.write(data2);
}

byte GPGUI::read_EXP_Port()
{
    byte data;
    if (EXP_Port.available())
    {
        data = EXP_Port.read();
    }
    return data;
}

void GPGUI::sendCommand_EXP_Port
(
    byte comand,
    byte data1,
    byte data2
)
{
    EXP_Port.write(comand);
    EXP_Port.write(data1);
    EXP_Port.write(data2);
}




//SOUND CARD COMAND SCC##############################################################
void GPGUI::SCC_Note_On
(
    byte channel,
    byte note,
    byte velocity
)
{
    if (channel < 1)  channel = 1;
    if (channel > 16) channel = 16;

    set_HDD_Led(1);
    send_Midi_Message(143+channel, note, velocity);
    set_HDD_Led(0);
}

void GPGUI::SCC_Note_Off
(
    byte channel,
    byte note
)
{
    if (channel < 1)  channel = 1;
    if (channel > 16) channel = 16;

    set_HDD_Led(1);
    send_Midi_Message(127 + channel, note, 0);
    set_HDD_Led(0);
}





//##############################################################################################################
//#//////////////////////////////////////////VGA BUFFER CONTROL////////////////////////////////////////////////#
//##############################################################################################################

int GPGUI::get_Color_Shade(int p_Color, int s_Color)
{
    int color = cs.shaded_Color[p_Color][s_Color];
    return color;

}




void GPGUI::setPixel
(
    int x,
    int y,
    byte color
)//https://www.rapidtables.com/web/color/RGB_Color.html
{
 

    vga.dotFast(x, y, color);
}

void GPGUI::drawLine
(
    int x1,
    int y1,
    int x2,
    int y2,
    byte color
)
{
    DDA_Line(x1, y1, x2, y2, color);
}

void GPGUI::draw_Vector
(
    int PX1,
    int PY1,
    int R,
    float AN,
    bool rotation_Dir,
    int* px2,
    int* py2,
    int color
)
{
    int PX2 = R;
    int PY2 = 0;

    m.rotate_Point(&PX2, &PY2, AN, rotation_Dir);
    PX2 += PX1;
    PY2 += PY1;

    *px2 = PX2;//return end point of vector
    *py2 = PY2;

    DDA_Line(PX1, PY1, PX2, PY2, color);
}

void GPGUI::drawBackgroundColor(byte color)
{
    for (int y = 0; y < ScreenHeight; y++)
    {
        for (int x = 0; x < ScreenWidth; x++)
        {
            vga.dotFast(x, y, color);
        }
    }
}

void GPGUI::drawHollowRectangle
(
    int xStart, 
    int yStart,
    int xSize, 
    int ySize,
    byte color
)
{
        for (int y = yStart; y < (yStart + ySize); y++)//left vert line
        {
            vga.dotFast(xStart, y, color);
        }

        for (int y = yStart; y < (yStart + ySize); y++)//RIGHT vert line
        {
            vga.dotFast((xStart + xSize) -1, y, color);

        }

        for (int x = xStart; x < (xStart + xSize) -1; x++)//top horizontal line
        {
            vga.dotFast(x, yStart, color);
        }

        for (int x = xStart; x < (xStart + xSize); x++)//bottom horizontal line
        {
            vga.dotFast(x, (yStart + ySize ) , color);
        }
}

void GPGUI::drawHollowCircle
(
    int x,
    int y,
    int radius,
    byte color
)
{
    vga.circle(x, y, radius, color);
}

void GPGUI::drawSolidCircle
(
    int x,
    int y,
    int radius,
    byte color
)
{
    vga.fillCircle(x, y, radius, color);
}

void GPGUI::drawSolidRectangle
(
    int xStart,
    int yStart,
    int xSize,
    int ySize, 
    byte color
)
{
    if (xStart <= ScreenWidth - xSize && yStart <= ScreenHeight - ySize)//only draw if within the screen size
    {
        for (int y = yStart; y < (yStart + ySize); y++)
        {
            for (int x = xStart; x < (xStart + xSize); x++)
            {
                setPixel(x, y, color);
            }
        }
    }
}

void GPGUI::drawTriangle
(
    int x1,
    int y1, 
    int x2,
    int y2,
    int x3, 
    int y3,
    byte fill_Type,
    byte color
)
{
    byte color_a;
    byte color_b;
    byte color_c;



    if (triangle_Debug)
    {
        color_a = red;
        color_b = green;
        color_c = blue;

    }
    else
    {
        color_a = color;
        color_b = color;
        color_c = color;
    }

    m.triangle_CW_Sort(x1, y1, x2, y2, x3, y3);






    bool line_Debug_Color = false;
    byte triangle_1_Color = color;
    byte triangle_2_Color = color;
    byte debug_Color_1 = red;
    byte debug_Color_2 = green;

    byte side;
    byte n_Triangles = 0;
    float DX_Left = x1 - x3;
    float DY_Left = y3 - y1;
    float Xinc_Left = DX_Left / DY_Left;
    float XL = x1;
    //
    float DX_Right = x2 - x1;
    float DY_Right = y2 - y1;
    float Xinc_Right = DX_Right / DY_Right;//SLOPE
    float XR = x1;
    int top_Lines_N = DY_Right;
    float lines_To_Draw_Buffer[250][2];//left x right x
    float x4 = x3;//x3
    float y4 = y2;
  





    if (fill_Type == 0)//WIRE FRAME MODE
    {
        drawLine(x1, y1, x2, y2, color_a);
        drawLine(x2, y2, x3, y3, color_b);
        drawLine(x3, y3, x1, y1, color_c);
    }





    if (triangle_Debug)
    {
        triangle_1_Color = debug_Color_1;
        triangle_2_Color = debug_Color_2;
    }
    else
    {
        triangle_1_Color = color;
        triangle_2_Color = color;
    }



    //X LINE FILL FLAT BOTTOM//////////////////////////////////////////////////////////////////////////////////////////

    if (fill_Type == 1)
    {


        drawLine(x1, y1, x2, y2, color_a);
        drawLine(x2, y2, x3, y3, color_b);
        drawLine(x3, y3, x1, y1, color_c);


        //left line points in quad 2 from p1
        for (int y = 0; y < DY_Left; y++)
        {
            lines_To_Draw_Buffer[y][0] = round(XL);
            XL -= Xinc_Left;
        }



        //right line points in quad 1 from p1
        for (int y = 0; y < DY_Right; y++)
        {
            lines_To_Draw_Buffer[y][1] = round(XR);
            XR += Xinc_Right;
        }

        x4 = lines_To_Draw_Buffer[top_Lines_N][0];//b_Line left start

        if (x2 <= x4) side = 1;
        if (x2 >= x4) side = 2;

   
     
   
        //DRAW X LINES LEFT TO RIGHT down to flat bottom
        for (int y = 0; y < top_Lines_N; y++)
        {

        
       
           float step_Size = ((lines_To_Draw_Buffer[y][1] - lines_To_Draw_Buffer[y][0])/100) /100;

                   //DRAW FILL LINES LEFT TO RIGHT
            if (side == 2)
            {
               for (float x = lines_To_Draw_Buffer[y][0]; x < lines_To_Draw_Buffer[y][1]; x++)
                {
                    setPixel((int)x, y + y1, triangle_1_Color);
                

                   float step_Size = (lines_To_Draw_Buffer[y][1] - lines_To_Draw_Buffer[y][0]) /100;

              
                               
                }
                y4 = y2;
            }

            //DRAW FILL LINES RIGHT TO LEFT
            if (side == 1)
            {
                for (float x = lines_To_Draw_Buffer[y][0]; x > lines_To_Draw_Buffer[y][1]; x--)
                {
                    setPixel((int)x, y + y1, triangle_1_Color);
                }
                y4 = y3;
            }
        }




        if (y2 == y3 || y2 == y1 || y3 == y1)   n_Triangles = 1;
        else n_Triangles = 2;

        //END DRAW DOWN TO FLAT BOTTOM






        ////DRAW FROM FLAT TOP DOWN

        float DX_Left_B;
        float DY_Bottom;
        float Xinc_Left_B;
        float XL_B;
        ////

        //LEFT LINE POINTS
        if (side == 2)
        {
            DX_Left_B = x4 - x3;
            DY_Bottom = y3 - y4;
            Xinc_Left_B = DX_Left_B / DY_Bottom;
            XL_B = x4;


            for (int y = 0; y < DY_Bottom; y++)
            {

                 //setPixel(round(XL_B), y + y4, random(1, 64));
                lines_To_Draw_Buffer[y][0] = round(XL_B);
                XL_B -= Xinc_Left_B;
            }
        }


        if (side == 1)
        {
            DX_Left_B = x2 - x3;
            DY_Bottom = y2 - y3;
            XL_B = x3;
            Xinc_Left_B = DX_Left_B / DY_Bottom;

            for (int y = 0; y < DY_Bottom; y++)
            {
                 //setPixel(round(XL_B), y + y3, random(1, 64));
                lines_To_Draw_Buffer[y][0] = round(XL_B);
                XL_B += Xinc_Left_B;
            }
        }
        //////END LEFT LINE PONTS


          //  Right LINE POINTS
        if (side == 2)
        {
            DX_Left_B = x2 - x3;
            DY_Bottom = y3 - y2;
            Xinc_Left_B = DX_Left_B / DY_Bottom;
            XL_B = x2;

            for (int y = 0; y < DY_Bottom; y++)
            {
                 // setPixel(round(XL_B), y + y2, random(1, 64));
                lines_To_Draw_Buffer[y][1] = round(XL_B);
                XL_B -= Xinc_Left_B;
            }
        }



        if (side == 1)
        {
            DX_Left_B = x2 - x4;
            DY_Bottom = y2 - y4;
            XL_B = x4;
            Xinc_Left_B = DX_Left_B / DY_Bottom;

            for (int y = 0; y < DY_Bottom; y++)
            {
                 // setPixel(round(XL_B), y + y4, random(1, 64));
                lines_To_Draw_Buffer[y][1] = round(XL_B);
                XL_B += Xinc_Left_B;
            }
        }
        //END Right LINE PONTS



        if (side == 1)
        {
            //DRAW X LINES LEFT TO RIGHT down to flat bottom
            for (int y = 0; y < DY_Bottom; y++)
            {
                //DRAW FILL LINES LEFT TO RIGHT

                for (float x = lines_To_Draw_Buffer[y][0]; x < lines_To_Draw_Buffer[y][1]; x++)
                {
                   // setPixel((int)x, y + y4, triangle_2_Color);
                }
            }
        }

        if (side == 2)
        {
            //DRAW X LINES LEFT TO RIGHT down to flat bottom
            for (int y = 0; y < DY_Bottom; y++)
            {
                //DRAW FILL LINES LEFT TO RIGHT

                for (float x = lines_To_Draw_Buffer[y][0]; x < lines_To_Draw_Buffer[y][1]; x++)
                {
                   // setPixel((int)x, y + y2, triangle_2_Color);
                }
            }
        }
    }







    if (triangle_Debug)
    {
        print("TRIANGLES = ", 0, 220, red, black); printNumberInt(n_Triangles, 75, 220, green, black);
        print("P2 Side   = ", 0, 230, red, black); printNumberInt(side, 75, 230, green, black);
        setPixel(x1, y1, random(1, 64));
        setPixel(x2, y2, green);
        setPixel(x3, y3, blue);
        // if (y2 != y3) setPixel(x4, y4, yellow);


         print("P1", x1 + 5, y1 + 5, red, black);
         print("P2", x2 + 5, y2 + 5, green, black);
         print("P3", x3 + 5, y3 + 5, blue, black);

    }







    ////print lines buffer
  /*  for (int i = 0; i < DY_Bottom; i++)
    {
        Serial.print(i); Serial.print("    ");
        Serial.print(lines_To_Draw_Buffer[i][0]);
        Serial.print("    ");
        Serial.println(lines_To_Draw_Buffer[i][1]);
        delay(1);
    }*/


}







void GPGUI::drawTriangle_Pipeline
(
    int x1,
    int y1,
    int x2,
    int y2,
    int x3,
    int y3,
    byte fill_Type,
    byte color
)
{
    byte color_a;
    byte color_b;
    byte color_c;



    if (triangle_Debug)
    {
        color_a = red;
        color_b = green;
        color_c = blue;

    }
    else
    {
        color_a = color;
        color_b = color;
        color_c = color;
    }

    m.triangle_CW_Sort(x1, y1, x2, y2, x3, y3);


    bool line_Debug_Color = false;
    byte triangle_1_Color = color;
    byte triangle_2_Color = color;
    byte debug_Color_1 = red;
    byte debug_Color_2 = green;



    float DXL;
    float DYL;
    float DXR;
    float DYR;
    float slope_Left;
    float slope_Right;
    float slope_Top;
    float slope_Bottom;

    float left_Point_X;
    float right_Point_X;
    int Y;

    if (fill_Type == 0)//WIRE FRAME MODE
    {
        drawLine(x1, y1, x2, y2, color_a);
        drawLine(x2, y2, x3, y3, color_b);
        drawLine(x3, y3, x1, y1, color_c);
    }


    int t_Type;


    if (triangle_Debug)
    {
        triangle_1_Color = debug_Color_1;
        triangle_2_Color = debug_Color_2;
    }
    else
    {
        triangle_1_Color = color;
        triangle_2_Color = color;
    }



    //X LINE FILL FLAT BOTTOM//////////////////////////////////////////////////////////////////////////////////////////
    byte trig_Tri_Color;
    if (fill_Type == 1)
    {


     //   drawLine(x1, y1, x2, y2, color_a);
      //  drawLine(x2, y2, x3, y3, color_b);
       // drawLine(x3, y3, x1, y1, color_c);



        if (y2 == y3 && x1 > x2 && x1 < x3 || y2 == y3 && x1 > x3 && x1 < x2)// flat bottom 2>>3 type 1a   3>>2 type 1b center 
        {
            if (x2 < x3)//TYPE A
            {
                DXL = x1 - x2;
                DYL = y2 - y1;
                DXR = x3 - x1;
                DYR = y3 - y1;
                trig_Tri_Color = 1;
                t_Type = 1;
            }
            if (x3 < x2)//TYPE B
            {
                DXL = x1 - x3;
                DYL = y3 - y1;
                DXR = x2 - x1;
                DYR = y2 - y1;
                trig_Tri_Color = 2;
                t_Type = 2;
            }
            slope_Left = DXL / DYL;      
            slope_Right = DXR / DYR;                       
            left_Point_X  = x1;
            right_Point_X = x1;
       
            for (int i = 0; i < DYL; i++)
            {
                Y = i + y1;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                right_Point_X += slope_Right;
                left_Point_X -= slope_Left;
            }       
        }




        if (y2 == y3 && x1 <= x2 && x2 < x3 || y2 == y3 && x1 <= x3 && x2 > x3)// flat bottom 2>>3 type 2a   3>>2 type 2b LEFT OFFSET
        {
            if (x2 < x3)//TYPE A
            {
                DXL = x1 - x2;
                DYL = y2 - y1;
                DXR = x3 - x1;
                DYR = y3 - y1;
                trig_Tri_Color = 3;
            }
            if (x3 < x2)//TYPE B
            {
                DXL = x1 - x3;
                DYL = y3 - y1;
                DXR = x2 - x1;
                DYR = y2 - y1;
                trig_Tri_Color = 4;
            }
            slope_Left = DXL / DYL;
            slope_Right = DXR / DYR;
            left_Point_X = x1;
            right_Point_X = x1;
          
            for (int i = 0; i < DYL; i++)
            {
                Y = i + y1;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                right_Point_X += slope_Right;
                left_Point_X -= slope_Left;
            }
        }


        if (y2 == y3 && x1 >= x2 && x2 < x3 || y2 == y3 && x1 >= x3 && x2 > x3)// flat bottom 2>>3  3>>2  RIGHT OFFSET
        {
            if (x2 < x3)//TYPE A
            {
                DXL = x1 - x2;
                DYL = y2 - y1;
                DXR = x3 - x1;
                DYR = y3 - y1;
                trig_Tri_Color = 3;
            }
            if (x3 < x2)//TYPE B
            {
                DXL = x1 - x3;
                DYL = y3 - y1;
                DXR = x2 - x1;
                DYR = y2 - y1;
                trig_Tri_Color = 4;
            }
            slope_Left = DXL / DYL;
            slope_Right = DXR / DYR;
            left_Point_X = x1;
            right_Point_X = x1;
            for (int i = 0; i < DYL; i++)
            {
                Y = i + y1;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                right_Point_X += slope_Right;
                left_Point_X -= slope_Left;
            }
        }//end type 3












        //FLAT TOPS//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        if (y1 == y2 && x3 > x2 && x3 < x1 && x2 < x1|| y1 == y2 && x3 > x1 && x3 < x2 && x1 < x2)// flat top 1>>2 type 4a   2>>1 type 4b right offset
        {
            if (x1 < x2)//TYPE A
            {
                DXL = x3 - x1;
                DYL = y3 - y1;
                DXR = x2 - x3;
                DYR = y3 - y1;
                t_Type = 1;
            }
            if (x2 < x1)//TYPE B
            {
                DXL = x3 - x2;
                DYL = y3 - y1;
                DXR = x1 - x3;
                DYR = y3 - y1;
                t_Type = 2;
            }
            slope_Left = DXL / DYL;
            slope_Right = DXR / DYR;
            left_Point_X = x3;
            right_Point_X = x3;
            for (int i = 0; i < DYL; i++)
            {
                Y =  y1+DYL - i;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                left_Point_X -= slope_Left;
                right_Point_X += slope_Right;
            }
        }



        if (y1 == y2 && x3 <= x1 && x1 < x2 || y1 == y2 && x3 <= x2 && x2 < x1)// flat top left offest
        {
            if (x1 < x2)//TYPE A
            {
                DXL = x3 - x1;
                DYL = y3 - y1;
                DXR = x2 - x3;
                DYR = y3 - y1;
                t_Type = 1;
            }
            if (x2 < x1)//TYPE B
            {
                DXL = x3 - x2;
                DYL = y3 - y1;
                DXR = x1 - x3;
                DYR = y3 - y1;
                t_Type = 2;
            }
            slope_Left = DXL / DYL;
            slope_Right = DXR / DYR;
            left_Point_X = x3;
            right_Point_X = x3;
            for (int i = 0; i < DYL; i++)
            {
                Y = y1 + DYL - i;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                right_Point_X += slope_Right;
                left_Point_X -= slope_Left;
            }
        }


        if (y1 == y2 && x3 >= x2 && x1 < x2 || y1 == y2 && x3 >= x1 && x2 < x1)// flat top right offset
        {
            if (x1 < x2)//TYPE A
            {
                DXL = x3 - x1;
                DYL = y3 - y1;
                DXR = x2 - x3;
                DYR = y3 - y1;
                t_Type = 1;
            }
            if (x2 < x1)//TYPE B
            {
                DXL = x3 - x2;
                DYL = y3 - y1;
                DXR = x1 - x3;
                DYR = y3 - y1;
                t_Type = 2;
            }
            slope_Left = DXL / DYL;
            slope_Right = DXR / DYR;
            left_Point_X = x3;
            right_Point_X = x3;
            for (int i = 0; i < DYL; i++)
            {
                Y = y1 + DYL - i;
                for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                left_Point_X -= slope_Left;
                right_Point_X += slope_Right;
            }
        }//end type 5


        //END OF SINGLE TRIANGLES


























        if (y2 < y3)//SPLIT INTO TO TRIANGLES/////////////////////////////////////////////////////////////////////
        {
            if (x1 == x3 && y2 > y1&& y2 < y3 && x2 > x3 )//flat left  CENTER
            {             
                DXL = x2 - x1;
                DYL = y2 - y1;
                DXR = x2 - x1;
                DYR = y3 - y2;                             
                slope_Top =    DXL / DYL;
                slope_Bottom = DXR / DYR;               
                right_Point_X = x1;
                left_Point_X = x1;
                for (int i = 0; i < (DYL+DYR); i++)
                {                  
                    Y = y1 + i;
                    if (Y < y2 )
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);//top 
                        right_Point_X += slope_Top;
                    }

                    if (Y >= y2 )
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom
                        right_Point_X -= slope_Bottom;
                    }
                }
            }


            if (x1 == x3 && y2 > y1 && y2 < y3 && x2 < x3)//flat right CENTER
            {
                DXL = x1 - x2;
                DYL = y2 - y1;
                DXR = x1 - x2;
                DYR = y3 - y2;
                slope_Top = DXL / DYL;
                slope_Bottom = DXR / DYR;
                right_Point_X = x1;
                left_Point_X = x1;
                for (int i = 0; i < (DYL + DYR); i++)
                {                  
                    Y = y1 + i;
                    if (Y < y2 )
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                        left_Point_X -= slope_Top;
                    }
                    if (Y >= y2)
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom
                        left_Point_X += slope_Bottom;
                    }
                }
            }






            if (x2 == x3 && y2 < y3 && y1 < y2 && y1 < y3 && x1 > x2)//flat left upper offset 
            {
                DXL = x1 - x2;
                DYL = y2 - y1;
                DXR = x1 - x2;
                DYR = y3 - y1;
                slope_Top = DXL / DYL;
                slope_Bottom = DXR / DYR;
                right_Point_X = x1;
                left_Point_X = x1;
                for (int i = 0; i <  DYR; i++)
                {
                    Y = y1 + i;
                    if (Y < y2)
                    {
                        for (int x = left_Point_X ; x < right_Point_X; x++)   setPixel(x, Y, red);
                        left_Point_X -= slope_Top;
                        right_Point_X -= slope_Bottom;
                    }
                    if (Y >= y2)
                    {
                        left_Point_X = x2;
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom                     
                        right_Point_X -= slope_Bottom;
                    }
                }
            }





            if (x2 == x3 && y2 < y3 && y1 < y2 && y1 < y3 && x1 < x2)//flat right upper offset LEFT
            {
                DXL = x2 - x1;
                DYL = y2 - y1;
                DXR = x2 - x1;
                DYR = y3 - y1;
                slope_Top = DXL / DYL;
                slope_Bottom = DXR / DYR;
                right_Point_X = x1;
                left_Point_X = x1;
                for (int i = 0; i < DYR; i++)
                {
                    Y = y1 + i;
                    if (Y < y2)
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                        left_Point_X += slope_Bottom;
                        right_Point_X += slope_Top;
                    }
                    if (Y >= y2)
                    {
                        right_Point_X = x2;
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom
                        left_Point_X += slope_Bottom;
                    }
                }
            }



            if (x1 == x2 && y2 < y3 && y1 < y2 && y1 < y3 && x3 > x1)//flat right lower offset RIGHT
            {
                DXL = x3 - x1;
                DYL = y3 - y1;
                DXR = x3 - x1;
                DYR = y3 - y2;
                slope_Top = DXL / DYL;
                slope_Bottom = DXR / DYR;
                right_Point_X = x1;
                left_Point_X = x1;
                for (int i = 0; i < DYL; i++)
                {
                    Y = y1 + i;
                    if (Y < y2)
                    {
                        left_Point_X = x1;
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                        right_Point_X += slope_Top;
                    }
                    if (Y >= y2)
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom
                        left_Point_X += slope_Bottom;
                        right_Point_X += slope_Top;
                    }
                }
            }



            if (x1 == x2 && y2 < y3 && y1 < y2 && y1 < y3 && x3 < x1)//flat right lower offset  LEFT
            {
                DXL = x2 - x3;
                DYL = y3 - y1;
                DXR = x2 - x3;
                DYR = y3 - y2;
                slope_Top = DXL / DYL;
                slope_Bottom = DXR / DYR;
                right_Point_X = x1;
                left_Point_X = x1;
              
                for (int i = 0; i < DYL; i++)
                {
                    Y = y1 + i;
                    if (Y < y2)
                    {
                        right_Point_X = x1;
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, red);
                        left_Point_X -= slope_Top;
                    }
                    if (Y >= y2)
                    {
                        for (int x = left_Point_X; x < right_Point_X; x++)   setPixel(x, Y, green);//bottom
                        left_Point_X -= slope_Top;
                        right_Point_X -= slope_Bottom;
                    }
                }
            }














        }

    }

    print("P1", x1 + 5, y1 + 5, red, black);
    print("P2", x2 + 5, y2 + 5, green, black);
    print("P3", x3 + 5, y3 + 5, blue, black);
}



void GPGUI::draw_Quad
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
)
{

    drawTriangle_Pipeline((int)p1_X, (int)p1_Y, (int)p2_X, (int)p2_Y, (int)p3_X, (int)p3_Y, fill_Type, green);
  //  drawTriangle((int)p1_X, (int)p1_Y, (int)p3_X, (int)p3_Y, (int)p4_X, (int)p4_Y, fill_Type, red);

}

void GPGUI::map_Scanline_From_Texture
(
    int x1,
    int x2,
    int yi,
    float texXstart,
    float texXend,
    float texYstart,
    byte tex[32][32]
)

{
    int texture_Width = 32;
    float un = 0;
    int texX = 0;
    int texY = 0;
    int texX_Index = 0;
    // float z = 0.8;
    // float zi = 1 / z;

    if (texXstart == 0 && texXend == 0) texXend = 1;

    m.interp(x1, x2, texXstart, &texX_Index);

    m.interp(0, texture_Width, texYstart, &texY);

    for (int i = x1; i < x2; i++)
    {
        m.normalize(i, x1, x2, &un);
        //un = un / z;//perspective divide
        //float uc = un * 1/zi;//perspective corect


        if (un + texXstart < 1 && un + texXstart < texXend)
        {
            m.interp(0, texture_Width, un + texXstart, &texX);//get texture x pos
            setPixel((i + texX_Index) - x1, yi, tex[texY][texX]);//sample texture for color og
        }
        else { break; }
    }


}

void GPGUI::draw_Perspective_Plane
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
)
{
    bool  debug = false;
    int zi = 0;
    int zFar = yStart + zFar_Offset;
    int zNear = (yStart + screen_Height) - zNear_Offset;
    int p1x = xStart + (screen_Width - zFar_Plane_Length) / 2;
    int p1y = zFar;
    int p2x = xStart + (screen_Width - ((screen_Width - zFar_Plane_Length) / 2));
    int p2y = zFar;
    int p3x = xStart + (screen_Width - ((screen_Width - zNear_Plane_Length) / 2));
    int p3y = zNear;
    int p4x = xStart + (screen_Width - zNear_Plane_Length) / 2;
    int p4y = zNear;
    float  incXL_Float_Counter = 0;
    float dxL = p1x - p4x;
    float dyL = p4y - p1y;
    float dxR = p3x - p2x;
    float dyR = p3y - p2y;
    float mL = dxL / dyL;
    float mR = dxR / dyR;
    int yInc = 1;
    int n_Lines = zNear - zFar;
    float xL_Counter = p1x;
    float xR_Counter = p2x;
    int xL = p1x;
    int xR = p2x;
    int yL = p1y;



    //DRAW BACKGROUND WINDOW
    drawSolidRectangle(xStart, yStart, screen_Width, screen_Height, black);



    if (debug)
    {
        //DRAW WIRE FRAME
        drawLine(p1x, p1y, p2x, p2y, red);
        drawLine(p2x, p2y, p3x, p3y, green);
        drawLine(p3x, p3y, p4x, p4y, blue);
        drawLine(p4x, p4y, p1x, p1y, yellow);

        //DRAW POINTS
        setPixel(p1x, p1y, random(1, 63));
        setPixel(p2x, p2y, random(1, 63));
        setPixel(p3x, p3y, random(1, 63));
        setPixel(p4x, p4y, random(1, 63));
    }


    //GET LEFT AND RIGHT END POINTS OF SCANLINE/////////////////////

    for (int y = 0; y < n_Lines; y++)
    {
        xL_Counter -= mL;
        yL += yInc;
        xL = round(xL_Counter);
        //
        xR_Counter += mR;
        xR = round(xR_Counter);
        //
        float vi;//texture v space
        m.normalize(y + zFar, zFar, zNear, &vi);//CONVERT SCREEN SPACE DEPTH BETWEEN PLANES TO TEX V SPACE 0.0f -1.0f
        map_Scanline_From_Texture(xL, xR, yL, 0.0f, 1.0f, vi, tex);
    }
}

void GPGUI::DDA_Line
(
    int X1,
    int Y1,
    int X2,
    int Y2,
    byte color
)//(P1   P2)
{
    int xPos = X1;
    int yPos = Y1;
    double DX;//DELTA X
    double DY;//DELTA Y
    double ySlope_Counter = yPos;//FLOATING POINT COUNTER THAT GETS ROUNDED TO THE CLOSEST INT
    double xSlope_Counter = xPos;
    int steps; //THE AMOUNT OF POINTS ALONG THE LINE
    double Xinc;//X,Y STEP AMOUNT
    double Yinc;
    double M; //SLOPE OF THE LINE 
    int quad; //HOLDS WHICH 90 DEG SEGMENT OF THE COMPASS P2 IS IN RELATION TO P1



    //FIND QUAD OR QUAD LINE OF P2 /////////////////////////
    //S/E - QUAD 1
    if (X2 > X1 && Y2 > Y1)
    {
        DX = X2 - X1;
        DY = Y2 - Y1;
        quad = 1;
    }

    //S/W - QUAD 2
    if (X2 <= X1 && Y2 > Y1)
    {
        DX = X1 - X2;
        DY = Y2 - Y1;
        quad = 2;
    }


    //N/W - QUAD 3
    if (X2 <= X1 && Y2 <= Y1)
    {
        DX = X1 - X2;// find quad
        DY = Y1 - Y2;
        quad = 3;
    }

    //N/E - QUAD 4
    if (X2 > X1 && Y2 <= Y1)
    {
        DX = X2 - X1;
        DY = Y1 - Y2;
        quad = 4;
    }





    //FIND THE SMALLEST  DELTA AND DIVIDE BY THE BIGGEST DELTA TO GET M-SLOPE THEN SET STEP TO LONGEST DELTA

    if (DY <= DX)
    {
        steps = DX;
        M = DY / DX;
        //FIND  INCREMENT SIZE
        if (M < 1) { Xinc = 1; Yinc = M; }   //0-45 deg in one quad
        if (M == 1) { Xinc = 1; Yinc = 1; }   //normal
        if (M > 1) { Xinc = M; Yinc = 1; }   //45-90 deg
    }

    if (DX <= DY)
    {
        steps = DY;
        M = DX / DY;
        if (M > 1) { Xinc = 1; Yinc = M; }   //0-45 deg in one quad
        if (M == 1) { Xinc = 1; Yinc = 1; }   //normal
        if (M < 1) { Xinc = M; Yinc = 1; }   //45-90 deg
    }





    if (DDA_Line_Debug)
    {
        color = random(0, 64);
        Serial.println();
        Serial.println();
        Serial.println();
        Serial.print("DY = ");            Serial.println(DY);
        Serial.print("DX = ");            Serial.println(DX);
        Serial.print("STEPS = ");         Serial.println(steps);
        Serial.print("X increment = ");   Serial.println(Xinc);
        Serial.print("Y increment = ");   Serial.println(Yinc);
        Serial.print("QUAD = ");          Serial.println(quad);
        Serial.print("SLOPE = ");         Serial.println(M);
        Serial.println();
    }



    //CALCULATE POINTS FROM P1 - P2
    setPixel(xPos, yPos, color);
    //PICK QUAD TO FIND STEP DIRECTION
    //S/E Q1
    if (quad == 1)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter += Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter += Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            //  
            setPixel(xPos, yPos, color);

            if (DDA_Line_Debug)
            {
                Serial.print("x pos = ");    Serial.print(xPos);
                Serial.print("   y pos = "); Serial.println(yPos);
            }
        }
    }////////////////////////////////////////////////////


    //S/W Q2
    if (quad == 2)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter -= Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter += Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            //  
            setPixel(xPos, yPos, color);
            if (DDA_Line_Debug)
            {
                Serial.print("x pos = ");    Serial.print(xPos);
                Serial.print("   y pos = "); Serial.println(yPos);
            }
        }
    }////////////////////////////////////////////////////



    //S/W Q3
    if (quad == 3)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter -= Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter -= Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            setPixel(xPos, yPos, color);
            if (DDA_Line_Debug)
            {
                Serial.print("x pos = ");    Serial.print(xPos);
                Serial.print("   y pos = "); Serial.println(yPos);
            }
        }
    }///////////////////////////////////////////////////



    // S/W Q4
    if (quad == 4)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter += Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter -= Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            setPixel(xPos, yPos, color);

            if (DDA_Line_Debug)
            {
                Serial.print("x pos = ");    Serial.print(xPos);
                Serial.print("   y pos = "); Serial.println(yPos);
            }
        }
    }
}

void GPGUI::sine_Wave_Bounce
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
)
{

    static int angle;
    int amp = height;//height
    static float Yinc;
    float Xinc = X_Distance / 180;
    static float Pos_speed = start_Speed;
    static unsigned long previousMillis = 0;
    static int x_Start = *Xpos;/////static
    static int y_Start = *Ypos;
    static bool run = false;


    if (enable == true)
    {
        run = true;
        *state = run;
        x_Start = *Xpos;//get starting x point of jump
    }

    if (stop == true)
    {
        run = false;
        angle = 0;
        Pos_speed = start_Speed;
        *state = run;
        if (x_Start > 320) x_Start = 320;
        if (x_Start < 0)   x_Start = 0;
        x_Start = *Xpos;//reset starting x and y to current point
        y_Start = *Ypos;
    }



    ///////////////////////////////////////////////////////////////
    if (currentMillis - previousMillis >= Pos_speed && run)
    {
        previousMillis = currentMillis;


        //CALCULATE X AND Y POSITIONS
        Yinc = sin(angle * 3.144 / 180) * amp;
        if (*Ypos > 20) *Ypos = y_Start - Yinc;
        // else stop = true;



         // CHANGE DIRECTION
        if (dir) { if (*Xpos < 320)*Xpos += Xinc; }
        if (!dir) { if (*Xpos > 0) *Xpos -= Xinc; }
        //

        angle++;
        //SLOW DOWN ON THE UP SLOPE AND DECRESS ON THE WAY DOWN

        if (angle < 90) Pos_speed += velocity;
        if (angle > 90) Pos_speed -= velocity;



        //RESET SPEED AND START POINT
        if (angle > 179) //end of arc
        {
            angle = 0;
            Pos_speed = start_Speed;
            run = false;
            *state = false;
            x_Start = *Xpos;//reset jump origin
        }

    }///////////////////////////////////////////////////////




    if (sine_Jump_Debug)
    {
        float Yinc_Line;
        float Xpos_Line = x_Start;
        float Ypos_Line = y_Start;


        //DRAW SINE LINE
        for (int i = 0; i < 180; i++)
        {
            //CALCULATE X AND Y POSITIONS
            Yinc_Line = sin(i * 3.144 / 180) * amp;
            if (Ypos_Line > 0) Ypos_Line = y_Start - Yinc_Line;
            if (Xpos_Line < 320 - Xinc && dir)              Xpos_Line += Xinc;
            if (Xpos_Line > Xinc && Xpos_Line > 0 && !dir)  Xpos_Line -= Xinc;
            if (Xpos_Line > 1 && Xpos_Line < 320 && Ypos_Line >1 && Ypos_Line < 239)
                setPixel(Xpos_Line, Ypos_Line, random(1, 64));
        }



        print("Pos_speed = ", 0, 0, red, black);  printNumberDouble(Pos_speed, 70, 0, green, black);
        print("DIR       = ", 0, 8, red, black);  printNumberInt(dir, 70, 8, green, black);
        print("X Pos     = ", 0, 16, red, black);  printNumberInt(*Xpos, 70, 16, green, black);
        print("Y Pos     = ", 0, 24, red, black);  printNumberInt(*Ypos, 70, 24, green, black);
        print("Sine angle= ", 0, 32, red, black);  printNumberInt(angle, 70, 32, green, black);
        print("X Inc     = ", 0, 40, red, black);  printNumberDouble(Xinc, 70, 40, green, black);
        print("Y Inc     = ", 0, 48, red, black);  printNumberInt(Yinc, 70, 48, green, black);
        print("X Distance= ", 0, 56, red, black);  printNumberInt(X_Distance, 70, 56, green, black);
        print("Amplitude = ", 0, 64, red, black);  printNumberInt(amp, 70, 64, green, black);
        print("X Start   = ", 0, 72, red, black);  printNumberInt(x_Start, 70, 72, green, black);
        print("Y Start   = ", 0, 80, red, black);  printNumberInt(y_Start, 70, 80, green, black);

    }
}


void GPGUI::DDA_Line_Clip
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
)//(P1   P2)
{
    int xPos = X1;
    int yPos = Y1;
    double DX;//DELTA X
    double DY;//DELTA Y
    double ySlope_Counter = yPos;//FLOATING POINT COUNTER THAT GETS ROUNDED TO THE CLOSEST INT
    double xSlope_Counter = xPos;
    int steps; //THE AMOUNT OF POINTS ALONG THE LINE
    double Xinc;//X,Y STEP AMOUNT
    double Yinc;
    double M; //SLOPE OF THE LINE 
    int quad; //HOLDS WHICH 90 DEG SEGMENT OF THE COMPASS P2 IS IN RELATION TO P1



    //FIND QUAD OR QUAD LINE OF P2 /////////////////////////
    //S/E - QUAD 1
    if (X2 > X1 && Y2 > Y1)
    {
        DX = X2 - X1;
        DY = Y2 - Y1;
        quad = 1;
    }

    //S/W - QUAD 2
    if (X2 <= X1 && Y2 > Y1)
    {
        DX = X1 - X2;
        DY = Y2 - Y1;
        quad = 2;
    }


    //N/W - QUAD 3
    if (X2 <= X1 && Y2 <= Y1)
    {
        DX = X1 - X2;// find quad
        DY = Y1 - Y2;
        quad = 3;
    }

    //N/E - QUAD 4
    if (X2 > X1 && Y2 <= Y1)
    {
        DX = X2 - X1;
        DY = Y1 - Y2;
        quad = 4;
    }





    //FIND THE SMALLEST  DELTA AND DIVIDE BY THE BIGGEST DELTA TO GET M-SLOPE THEN SET STEP TO LONGEST DELTA

    if (DY <= DX)
    {
        steps = DX;
        M = DY / DX;
        //FIND  INCREMENT SIZE
        if (M < 1) { Xinc = 1; Yinc = M; }   //0-45 deg in one quad
        if (M == 1) { Xinc = 1; Yinc = 1; }   //normal
        if (M > 1) { Xinc = M; Yinc = 1; }   //45-90 deg
    }

    if (DX <= DY)
    {
        steps = DY;
        M = DX / DY;
        if (M > 1) { Xinc = 1; Yinc = M; }   //0-45 deg in one quad
        if (M == 1) { Xinc = 1; Yinc = 1; }   //normal
        if (M < 1) { Xinc = M; Yinc = 1; }   //45-90 deg
    }





    //CALCULATE POINTS FROM P1 - P2
 
    if (xPos > xStart && xPos < xStart + (window_Size_X-1) && yPos > yStart && yPos < yStart + window_Size_Y)  setPixel(xPos, yPos, color);
  
    //PICK QUAD TO FIND STEP DIRECTION
    //S/E Q1
    if (quad == 1)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter += Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter += Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            //  
            if (xPos > xStart && xPos < xStart + (window_Size_X - 1) && yPos > yStart && yPos < yStart + window_Size_Y)   setPixel(xPos, yPos, color);
           // else { break; }
        }
    }////////////////////////////////////////////////////


    //S/W Q2
    if (quad == 2)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter -= Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter += Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            //  
            if (xPos > xStart && xPos < xStart + (window_Size_X-1) && yPos > yStart && yPos < yStart + window_Size_Y) setPixel(xPos, yPos, color);
          //  else { break; }
        }
    }////////////////////////////////////////////////////



    //S/W Q3
    if (quad == 3)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter -= Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter -= Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            if (xPos > xStart && xPos < xStart + (window_Size_X-1) && yPos > yStart && yPos < yStart + window_Size_Y) setPixel(xPos, yPos, color);
           // else { break; }
        }
    }///////////////////////////////////////////////////



    // S/W Q4
    if (quad == 4)
    {
        for (int i = 0; i < steps - 1; i++)
        {
            xSlope_Counter += Xinc;
            xPos = round(xSlope_Counter);
            //
            ySlope_Counter -= Yinc; //add float values
            yPos = round(ySlope_Counter); //round to ints
            if (xPos > xStart && xPos < xStart + (window_Size_X-1) && yPos > yStart && yPos < yStart + window_Size_Y) setPixel(xPos, yPos, color);
           // else { break; }
        }
    }
}


//TXT AND DATA DISPLAY#########################################################################

void GPGUI::printNumberInt
(
    int inputData,
    int x, 
    int y,
    byte textColor,
    byte backColor
)
{
    static byte r;
    static byte g;
    static byte b;
    static byte rB;
    static byte gB;
    static byte bB;

    switch (textColor)//base 16 colors
    {
    case 0: r = 0;   g = 0;   b = 0;   break;//BLACK
    case 63:r = 255; g = 255; b = 255; break;//WHITE
    case 3: r = 255; g = 0;   b = 0;   break;//RED
    case 12:r = 0;   g = 255; b = 0;   break;//LIME
    case 48:r = 0;   g = 0;   b = 255; break;//BLUE
    case 15:r = 255; g = 255; b = 0;   break;//YELLOW
    case 60:r = 0;   g = 255; b = 255; break;//CYAN/AQUA
    case 51:r = 255; g = 0;   b = 255; break;//MAGENTA/FUCHSIA
    case 42:r = 128; g = 128; b = 128; break;//GREY
    case 2: r = 128; g = 0;   b = 0;   break;//MAROON
    case 10:r = 128; g = 128; b = 0;   break;//OLIVE
    case 8: r = 0;   g = 128; b = 0;   break;//GREEN
    case 34:r = 128; g = 0;   b = 128; break;//PURPLE
    case 40:r = 0;   g = 128; b = 128; break;//TEAL
    case 32:r = 0;   g = 0;   b = 128; break;//NAVY
    //END BASE 16 COLORS
    }


    switch (backColor)//base 16 colors
    {
    case 0: rB = 0; gB = 0; bB = 0; break;//BLACK
    case 63:rB = 255; gB = 255; bB = 255; break;//WHITE
    case 3: rB = 255; gB = 0; bB = 0; break;//RED
    case 12:rB = 0; gB = 255; bB = 0; break;//LIME
    case 48:rB = 0; gB = 0; bB = 255; break;//BLUE
    case 15:rB = 255; gB = 255; bB = 0; break;//YELLOW
    case 60:rB = 0; gB = 255; bB = 255; break;//CYAN/AQUA
    case 51:rB = 255; gB = 0; bB = 255; break;//MAGENTA/FUCHSIA
    case 42:rB = 128; gB = 128; bB = 128; break;//GREY
    case 2: rB = 128; gB = 0; bB = 0; break;//MAROON
    case 10:rB = 128; gB = 128; bB = 0; break;//OLIVE
    case 8: rB = 0; gB = 128; bB = 0; break;//GREEN
    case 34:rB = 128; gB = 0; bB = 128; break;//PURPLE
    case 40:rB = 0; gB = 128; bB = 128; break;//TEAL
    case 32:rB = 0; gB = 0; bB = 128; break;//NAVY
    //END BASE 16 COLORS
    }

    vga.setFont(Font6x8);
    vga.setTextColor(vga.RGB(r, g, b), vga.RGB(rB, gB, bB));
    vga.setCursor(x, y);
    vga.print(inputData, 10, 1);
}

void GPGUI::printNumberHex
(
    int inputData, 
    int x,
    int y,
    byte textColor,
    byte backColor
)
{
    static byte r;
    static byte g;
    static byte b;
    static byte rB;
    static byte gB;
    static byte bB;

    switch (textColor)//base 16 colors
    {
    case 0: r = 0;   g = 0;   b = 0;   break;//BLACK
    case 63:r = 255; g = 255; b = 255; break;//WHITE
    case 3: r = 255; g = 0;   b = 0;   break;//RED
    case 12:r = 0;   g = 255; b = 0;   break;//LIME
    case 48:r = 0;   g = 0;   b = 255; break;//BLUE
    case 15:r = 255; g = 255; b = 0;   break;//YELLOW
    case 60:r = 0;   g = 255; b = 255; break;//CYAN/AQUA
    case 51:r = 255; g = 0;   b = 255; break;//MAGENTA/FUCHSIA
    case 42:r = 128; g = 128; b = 128; break;//GREY
    case 2: r = 128; g = 0;   b = 0;   break;//MAROON
    case 10:r = 128; g = 128; b = 0;   break;//OLIVE
    case 8: r = 0;   g = 128; b = 0;   break;//GREEN
    case 34:r = 128; g = 0;   b = 128; break;//PURPLE
    case 40:r = 0;   g = 128; b = 128; break;//TEAL
    case 32:r = 0;   g = 0;   b = 128; break;//NAVY
    //END BASE 16 COLORS
    }


    switch (backColor)//base 16 colors
    {
    case 0: rB = 0;   gB = 0;   bB = 0;   break;//BLACK
    case 63:rB = 255; gB = 255; bB = 255; break;//WHITE
    case 3: rB = 255; gB = 0;   bB = 0;   break;//RED
    case 12:rB = 0;   gB = 255; bB = 0;   break;//LIME
    case 48:rB = 0;   gB = 0;   bB = 255; break;//BLUE
    case 15:rB = 255; gB = 255; bB = 0;   break;//YELLOW
    case 60:rB = 0;   gB = 255; bB = 255; break;//CYAN/AQUA
    case 51:rB = 255; gB = 0;   bB = 255; break;//MAGENTA/FUCHSIA
    case 42:rB = 128; gB = 128; bB = 128; break;//GREY
    case 2: rB = 128; gB = 0;   bB = 0;   break;//MAROON
    case 10:rB = 128; gB = 128; bB = 0;   break;//OLIVE
    case 8: rB = 0;   gB = 128; bB = 0;   break;//GREEN
    case 34:rB = 128; gB = 0;   bB = 128; break;//PURPLE
    case 40:rB = 0;   gB = 128; bB = 128; break;//TEAL
    case 32:rB = 0;   gB = 0;   bB = 128; break;//NAVY
    //END BASE 16 COLORS
    }

    vga.setFont(Font6x8);
    vga.setTextColor(vga.RGB(r, g, b), vga.RGB(rB, gB, bB));
    vga.setCursor(x, y);
    vga.print(inputData, 16, 1);
}

void GPGUI::printNumberDouble
(
    double inputData,
    int x,
    int y,
    byte textColor,
    byte backColor
)
{
    static byte r;
    static byte g;
    static byte b;
    static byte rB;
    static byte gB;
    static byte bB;

    switch (textColor)//base 16 colors
    {
    case 0:r = 0; g = 0; b = 0; break;//BLACK
    case 63:r = 255; g = 255; b = 255; break;//WHITE
    case 3:r = 255; g = 0; b = 0; break;//RED
    case 12:r = 0; g = 255; b = 0; break;//LIME
    case 48:r = 0; g = 0; b = 255; break;//BLUE
    case 15:r = 255; g = 255; b = 0; break;//YELLOW
    case 60:r = 0; g = 255; b = 255; break;//CYAN/AQUA
    case 51:r = 255; g = 0; b = 255; break;//MAGENTA/FUCHSIA
    case 42:r = 128; g = 128; b = 128; break;//GREY
    case 2:r = 128; g = 0; b = 0; break;//MAROON
    case 10:r = 128; g = 128; b = 0; break;//OLIVE
    case 8:r = 0; g = 128; b = 0; break;//GREEN
    case 34:r = 128; g = 0; b = 128; break;//PURPLE
    case 40:r = 0; g = 128; b = 128; break;//TEAL
    case 32:r = 0; g = 0; b = 128; break;//NAVY
    //END BASE 16 COLORS
    }


    switch (backColor)//base 16 colors
    {
    case 0: rB = 0; gB = 0; bB = 0; break;//BLACK
    case 63: rB = 255; gB = 255; bB = 255; break;//WHITE
    case 3: rB = 255; gB = 0; bB = 0; break;//RED
    case 12: rB = 0; gB = 255; bB = 0; break;//LIME
    case 48: rB = 0; gB = 0; bB = 255; break;//BLUE
    case 15: rB = 255; gB = 255; bB = 0; break;//YELLOW
    case 60: rB = 0; gB = 255; bB = 255; break;//CYAN/AQUA
    case 51: rB = 255; gB = 0; bB = 255; break;//MAGENTA/FUCHSIA
    case 42: rB = 128; gB = 128; bB = 128; break;//GREY
    case 2: rB = 128; gB = 0; bB = 0; break;//MAROON
    case 10: rB = 128; gB = 128; bB = 0; break;//OLIVE
    case 8: rB = 0; gB = 128; bB = 0; break;//GREEN
    case 34: rB = 128; gB = 0; bB = 128; break;//PURPLE
    case 40: rB = 0; gB = 128; bB = 128; break;//TEAL
    case 32: rB = 0; gB = 0; bB = 128; break;//NAVY
    //END BASE 16 COLORS
    }

    vga.setFont(Font6x8);
    vga.setTextColor(vga.RGB(r, g, b), vga.RGB(rB, gB, bB));
    vga.setCursor(x, y);
    vga.print(inputData, 2, 1);
}

void GPGUI::print
(
    const char* inputData,
    int x,
    int y,
    byte textColor,
    byte backColor
)
{

    static byte r;
    static byte g;
    static byte b;
    static byte rB;
    static byte gB;
    static byte bB;

    switch (backColor)//base 16 colors
    {
    case  0: rB = 0; gB = 0; bB = 0; break;//BLACK
    case 63:rB = 255; gB = 255; bB = 255; break;//WHITE
    case 3: rB = 255; gB = 0; bB = 0; break;//RED
    case 12:rB = 0; gB = 255; bB = 0; break;//LIME
    case 48:rB = 0; gB = 0; bB = 255; break;//BLUE
    case 15:rB = 255; gB = 255; bB = 0; break;//YELLOW
    case 60:rB = 0; gB = 255; bB = 255; break;//CYAN/AQUA
    case 51:rB = 255; gB = 0; bB = 255; break;//MAGENTA/FUCHSIA
    case 42:rB = 128; gB = 128; bB = 128; break;//GREY
    case 2: rB = 128; gB = 0; bB = 0; break;//MAROON
    case 10:rB = 128; gB = 128; bB = 0; break;//OLIVE
    case 8: rB = 0; gB = 128; bB = 0; break;//GREEN
    case 34:rB = 128; gB = 0; bB = 128; break;//PURPLE
    case 40:rB = 0; gB = 128; bB = 128; break;//TEAL
    case 32:rB = 0; gB = 0; bB = 128; break;//NAVY
    //END BASE 16 COLORS
    }


    switch (textColor)//base 16 colors
    {
    case 0:r = 0; g = 0; b = 0; break;//BLACK
    case 63:r = 255; g = 255; b = 255; break;//WHITE
    case 3:r = 255; g = 0; b = 0; break;//RED
    case 12:r = 0; g = 255; b = 0; break;//LIME
    case 48:r = 0; g = 0; b = 255; break;//BLUE
    case 15:r = 255; g = 255; b = 0; break;//YELLOW
    case 60:r = 0; g = 255; b = 255; break;//CYAN/AQUA
    case 51:r = 255; g = 0; b = 255; break;//MAGENTA/FUCHSIA
    case 42:r = 128; g = 128; b = 128; break;//GREY
    case 2:r = 128; g = 0; b = 0; break;//MAROON
    case 10:r = 128; g = 128; b = 0; break;//OLIVE
    case 8:r = 0; g = 128; b = 0; break;//GREEN
    case 34:r = 128; g = 0; b = 128; break;//PURPLE
    case 40:r = 0; g = 128; b = 128; break;//TEAL
    case 32:r = 0; g = 0; b = 128; break;//NAVY
    //END BASE 16 COLORS
    }

    vga.setFont(Font6x8);
    vga.setTextColor(vga.RGB(r, g, b), vga.RGB(rB, gB, bB));

    vga.setCursor(x, y);
    vga.print(inputData);
}


//SYSTEM VGA FUNCTIONS##########################################################################

void GPGUI::refreshScreen()
{
    vga.show();//MUST CALL AFTER EVERY FRAME LOAD  
}

byte GPGUI::readVGAscreenBufferCell
(
    int x,
    int y,
    bool debugPrint
)
{
    static byte cellColor;//rgb value 0-256.
    unsigned long int mCellData = vga.get(x, y);//read rgb data directly from the screen buffer.

    if (debugPrint == true && serialDebugEnabledFlag == false) { serialDebugEnabledFlag = true; }
    if (debugPrint) { Serial.print("CELL RGB DATA = "); Serial.print(mCellData); }
    //
    switch (mCellData)//convert to single byte per cell.
    {
    case 0:      cellColor = 0; break;//black
    case 63:     cellColor = 1; break;//white
    case 3:      cellColor = 2; break;//red
    case 12:     cellColor = 3; break;//lime
    case 48:     cellColor = 4; break;//blue
    case 15:     cellColor = 5; break;//yellow
    case 60:     cellColor = 6; break;//cyan
    case 51:     cellColor = 7; break;//magenta
    case 42:     cellColor = 8; break;//grey
    case 2:      cellColor = 9; break;//maroon
    case 10:     cellColor = 10; break;//olive
    case 8:      cellColor = 11; break;//green
    case 34:     cellColor = 12; break;//purple
    case 40:     cellColor = 13; break;//teal
    case 32:     cellColor = 14; break;//navy
    }
    //
    if (debugPrint) { Serial.print("    CELL COLOR REF # = "); Serial.print(cellColor); Serial.println(); }
    if (debugPrint == false && serialDebugEnabledFlag == true) { serialDebugEnabledFlag = false; }
    return cellColor;//return cell contents as a color ref 0-255.
}



void GPGUI::draw_Color_Map(bool run)
{
    int box_Size_X = 25;
    int box_Size_Y = 25;
    int XP;
    int YP;
    int color;
    int space = 1;
    int xStart = 10;
    int yStart = 12;


    if (run)
    {
        for (int Y = 0; Y < 6; Y++)//DRAW CUBE MAP
        {
            for (int X = 0; X < 8; X++)
            {

                drawSolidRectangle(XP + xStart, YP + yStart, box_Size_X, box_Size_Y,color);
                printNumberInt(color, XP + xStart, YP + yStart, black, grey);
                XP += space + box_Size_X;
                color++;
            }
            YP += space + box_Size_Y;
            XP = xStart;
        }
        YP = yStart;
    }

}




void GPGUI::draw_Color_Map_Shaded(bool run)
{
    int box_Size_X = 25;
    int box_Size_Y = 25;
    int XP;
    int YP;
    int color;
    int space = 1;
    int xStart = 10;

    int yStart = 12;



    //violet 19



    if (run)
    {
        for (int y = 0; y < 7; y++)//DRAW CUBE MAP
        {
            for (int x = 0; x < 4; x++)
            {

                drawSolidRectangle(XP + xStart, YP + yStart, box_Size_X, box_Size_Y, get_Color_Shade(y,x));
                printNumberInt(get_Color_Shade(y, x), XP + xStart, YP + yStart, black, grey);
                XP += space + box_Size_X;
            }
            YP += space + box_Size_Y;
            XP = xStart;
        }
        YP = yStart;
    }


}






///################################################################################################################################
///                                               VERTEX PIPELINE                 
///################################################################################################################################
//#############################################################################################################
//#/////////////////////////////////////////3D RASTOR ENGINE//////////////////////////////////////////////////#
//#############################################################################################################

void GPGUI::vertex_Buff_2D_Parallel_Projection_Pipeline
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
    vector <vertex3D> & world_Vertex_Buffer
)

{
    vector<vertex3D> screen_Space_pixel_Buffer;
 
    
    static bool pixel_Bypass_Enable         = true;
    static bool pixel_Bypass_Z_Clip_Enable  = true;
    static bool line_Decoder_Enable         = false;
    static bool triangle_Decoder_Enable     = false;
    static bool triangle_Fill_Type          = false;
                                                            //###################################
        translate_XY_Veiw_Plane_To_Screen_Plane             //# VERTEX BUFF TO PIXEL BUFF BLOCK #
        (                                                   //###################################     
            xStart,                                         
            yStart,                                         
            screen_Plane_Size,                              
            screen_Plane_Size,                              
            cameraX,                                        
            cameraY,                                        
            cameraZ,                                        
            space_Color,                                    
            &*v_Count,                                                                     
            world_Vertex_Buffer,                            
            screen_Space_pixel_Buffer                       
        );                                                  
                                                          
                                                            //######################
                                                            //# PIXEL BYPASS BLOCK #
        pixel_Bypass                                        //######################
        (
            pixel_Bypass_Enable,
            pixel_Bypass_Z_Clip_Enable,
            &*z_Cliped,
            screen_Plane_Size,
            xStart,
            yStart,
            screen_Space_pixel_Buffer
        );
        


     
        line_Decoder                                         //######################
        (                                                    //# LINE DECODER BLOCK #
            line_Decoder_Enable,                             //######################
            screen_Plane_Size,
            xStart,
            yStart,
            screen_Space_pixel_Buffer
        );


  /*      triangle_Decoder
        (
            triangle_Decoder_Enable,
            0,
            0,
            0,
            triangle_Fill_Type,
            screen_Space_pixel_Buffer
        );
*/




        screen_Space_pixel_Buffer.clear();
    
}


void GPGUI::translate_XY_Veiw_Plane_To_Screen_Plane
(
    int xStart,
    int yStart,
    int window_Width,
    int window_Height,
    float camera_X,
    float camera_Y,
    float camera_Z,
    byte space_Color,
    int* vertex_Counter,
    vector <vertex3D>& to_Be_Translated_Buffer,
    vector <vertex3D>& Translated_Buffer
)
{
    float FOV = 60;
    float fov = m.deg_To_Rad(FOV);
    float camera_Plane_X = 2 * ((tan(fov / 2)) * camera_Z);

    int v_Count = 0;


    //DEFINE WORLD PLANE SIZE
    float world_Size = 1000;
    float wp1_X = (world_Size / 2) * -1;
    float wp1_Y = (world_Size / 2) * -1;

    float wp2_X = (world_Size / 2);
    float wp2_Y = (world_Size / 2);


    //LIMIT VIEW PLANE TO WORLD SIZE
    if (camera_Plane_X > world_Size) camera_Plane_X = world_Size - 1;
    //




    //DEFINE WORLD VIEW PLANE
    float plane_Index_X = camera_X;//LOOK AT ORIGIN ( VIEW PLANE CENTER VERTEX) 
    float plane_Index_Y = camera_Y;
    //
    float vp1_X = (((camera_Plane_X / 2) * -1) + plane_Index_X);
    float vp1_Y = (((camera_Plane_X / 2) * -1) + plane_Index_Y);
    //
    float vp2_X = ((camera_Plane_X / 2) + plane_Index_X);
    float vp2_Y = ((camera_Plane_X / 2) + plane_Index_Y);
    //

    if (vp1_X < wp1_X) { vp1_X += 1; vp2_X += 1; beep(1000, 50); }
    if (vp2_X > wp2_X) { vp1_X -= 1; vp2_X -= 1; beep(1000, 50); }

    if (vp1_Y < wp1_Y) { vp1_Y += 1; vp2_Y += 1; beep(1000, 50); }
    if (vp2_Y > wp2_Y) { vp1_Y -= 1; vp2_Y -= 1; beep(1000, 50); }

    //DEFINE SCREEN PLANE
    float sp1_X = xStart;
    float sp1_Y = yStart;
    //
    float sp2_X = xStart + window_Width;
    float sp2_Y = yStart + window_Height;
    //

   

    float vertex[3];//TEMP BUFFER TO HOLD WORLD VERTEX TO BE TRANSLATED TO SCREEN SPACE
    int XN;//TRANSLATED SCREEN SPACE VERTEX
    int YN;

    float x_old = 0;
    float y_old = 0;
    float z_old = 0;





    //DRAW WORLD VOID COLOR BACKGROUND
    //drawHollowRectangle (sp1_X, sp1_Y, window_Width, window_Height, yellow);

    //GO THROUGH BUFFER OF VERTICIES TRANSLATING FROM WORLD SPACE TO SCREEN SPACE
    for (int i = 0; i != to_Be_Translated_Buffer.size(); i++)
    {
        vertex3D vertex_Vi = to_Be_Translated_Buffer[i];
        vertex3D vertex_Vi_Translated;
        float temp[3];

        temp[0] = vertex_Vi.x;
        temp[1] = vertex_Vi.y;
        temp[2] = vertex_Vi.z;                                                
        m.world_plane_To_Screen_Plane_Vertex_Translate(vp1_X, vp1_Y, vp2_X, vp2_Y, sp1_X, sp1_Y, sp2_X, sp2_Y, temp, &XN, &YN);
    
     
            vertex_Vi_Translated.x = (float)XN;
            vertex_Vi_Translated.y = (float)YN;
            vertex_Vi_Translated.z = 0;
            vertex_Vi_Translated.u = vertex_Vi.u;
            vertex_Vi_Translated.v = vertex_Vi.v;
            vertex_Vi_Translated.c = vertex_Vi.c;
            vertex_Vi_Translated.id = vertex_Vi.id;
            vertex_Vi_Translated.mesh_Type = vertex_Vi.mesh_Type;
            Translated_Buffer.push_back(vertex_Vi_Translated);
            v_Count++;
   
     
    }
    *vertex_Counter = v_Count;
};


void GPGUI::pixel_Bypass
(
    bool enable,
    bool z_Clip,
    int *z_Cliped,
    int window_Clip_Size,
    int window_Start_X,
    int window_Start_Y,
    vector<vertex3D> &buffer
)
{
    float x_Old = 0;
    float y_Old = 0;
    float z_Old = 0;
    int pc = 0;
    if (enable)
    {
        for (int i = 0; i != buffer.size(); i++)
        {
            vertex3D Vi = buffer[i];
            if (z_Clip)
            {
                if (Vi.x == x_Old && Vi.y == y_Old)
                {
                    if (Vi.z > z_Old)
                    {
                        //CLIP PIXELS AROUND SCREEN WINDOW
                        if (Vi.x > window_Start_X && Vi.x < window_Clip_Size + (window_Start_X-1) && Vi.y > window_Start_Y && Vi.y < window_Clip_Size + window_Start_Y)
                        {
                            setPixel(Vi.x, Vi.y, Vi.c);//send to vga back buffer
                            z_Old = Vi.z;
                            pc++;
                        }
                    }
                }
                else
                {
                    //CLIP PIXELS AROUND SCREEN WINDOW
                    if (Vi.x > window_Start_X && Vi.x < window_Clip_Size + (window_Start_X-1) && Vi.y > window_Start_Y && Vi.y < window_Clip_Size + window_Start_Y)
                    {
                        setPixel(Vi.x, Vi.y, Vi.c);//send to vga back buffer
                        x_Old = Vi.x,
                        y_Old = Vi.y;
                        z_Old = Vi.z;
                        pc++;
                    }
                }
            }
            else
            {
                if (Vi.x > window_Start_X && Vi.x < window_Clip_Size + (window_Start_X - 1) && Vi.y > window_Start_Y && Vi.y < window_Clip_Size + window_Start_Y)
                    setPixel(Vi.x, Vi.y, Vi.c);//send to vga back buffer
            }
        }
        *z_Cliped = pc;
    }
}



void GPGUI::line_Decoder
(
    bool enable,
    int screen_Plane_Size,
    int xStart,
    int yStart,
    vector<vertex3D>& screen_Space_pixel_Buffer
)
{
    static int p1x;
    static int p1y;
    static int p2x;
    static int p2y;
    int line_Color;
    if (enable)
    {
        for (int Vi = 0; Vi != screen_Space_pixel_Buffer.size(); Vi++)
        {
            vertex3D vertex_a = screen_Space_pixel_Buffer[Vi];
            vertex3D vertex_b = screen_Space_pixel_Buffer[Vi + 1];

            if (vertex_a.mesh_Type == 1 && vertex_b.mesh_Type == 1 && vertex_a.id == vertex_b.id)
            {
                p1x = (int)vertex_a.x;
                p1y = (int)vertex_a.y;
                line_Color = vertex_a.c;

                p2x = (int)vertex_b.x;
                p2y = (int)vertex_b.y;
                DDA_Line_Clip(p1x, p1y, p2x, p2y, line_Color, xStart, yStart, screen_Plane_Size, screen_Plane_Size);
            }
        }
    }
}


void GPGUI::triangle_Decoder//vertex vesion
(
    bool enable,
    int screen_Plane_Size,
    int xStart,
    int yStart,
    bool fill_Type,
    vector<vertex3D>& screen_Space_pixel_Buffer
)
{
    if (enable)
    {
        for (int i = 0; i != screen_Space_pixel_Buffer.size(); i++)
        {
            vertex3D p1, p2, p3;
            p1 = screen_Space_pixel_Buffer[i];
            p2 = screen_Space_pixel_Buffer[i + 1];
            p3 = screen_Space_pixel_Buffer[i + 2];

            if (p1.mesh_Type == 2 && p2.mesh_Type == 2 && p3.mesh_Type == 2)
            {
                if (p2.id == p1.id && p3.id == p1.id)
                {
                 
                    drawTriangle
                    (
                        p1.x,
                        p1.y,
                        p2.x,
                        p2.y,
                        p3.x,
                        p3.y,
                        fill_Type,
                        p1.c
                    );
                }
            }
        }
    }
}





void GPGUI::write_Origin_Cross_Mesh_To_Vector
(
    vector<vertex3D>& mesh,
    float  origin_Cross_Size,
    bool full_Cross,
    int *vc
)
{
    vertex3D vi; 
    int vertex_Counter = 0;
    float origin_Cross_Step_Size = 1;
    if (origin_Cross_Size > 50) origin_Cross_Size = 50;

    //PLACE ORIGIN VECTOR VERTICIES INTO WORLD SPACE
    for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
    {
        vi.x = i;
        vi.y = 0;
        vi.z = 0;
        vi.c = red;
        vi.mesh_Type = 0;
        vi.id = 0;
        if (i == origin_Cross_Size - origin_Cross_Step_Size) vi.c = purple;
        mesh.push_back(vi);
        vertex_Counter++;
    }

    for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
    {
        vi.x = 0;
        vi.y = i;
        vi.z = 0;
        vi.c = green;
        vi.mesh_Type = 0;
        vi.id = 0;
        if (i == origin_Cross_Size - origin_Cross_Step_Size) vi.c = purple;
        mesh.push_back(vi);
        vertex_Counter++;
    }

    for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
    {
        vi.x = 0;
        vi.y = 0;
        vi.z = i;
        vi.c = blue;
        vi.mesh_Type = 0;
        vi.id = 0;
        if (i == origin_Cross_Size - origin_Cross_Step_Size) vi.c = purple;
        mesh.push_back(vi);
        vertex_Counter++;
    }
    //////////////////////////////////////////////

    if (full_Cross)
    {
        for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
        {
            vi.x = i*-1;
            vi.y = 0*-1;
            vi.z = 0*-1;
            vi.c = red;
            vi.mesh_Type = 0;
            vi.id = 0;
            if (i == origin_Cross_Size - 1) vi.c = purple;
            mesh.push_back(vi);
            vertex_Counter++;
        }

        for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
        {
            vi.x = 0*-1;
            vi.y = i*-1;
            vi.z = 0*-1;
            vi.c = green;
            vi.mesh_Type = 0;
            vi.id = 0;
            if (i == origin_Cross_Size - 1) vi.c = purple;
            mesh.push_back(vi);
            vertex_Counter++;
        }

        for (float i = 0; i < origin_Cross_Size; i += origin_Cross_Step_Size)
        {
            vi.x = 0*-1;
            vi.y = 0*-1;
            vi.z = i*-1;
            vi.c = blue;
            vi.mesh_Type = 0;
            vi.id = 0;
            if (i == origin_Cross_Size - 1) vi.c = purple;
            mesh.push_back(vi);
            vertex_Counter++;
        }
        //////////////////////////////////////////////
    }

    *vc = vertex_Counter;



}



void GPGUI::rotate_Mesh
(
    vector<vertex3D>& buffer, 
    int axis, 
    float angle_Delta
)
{


    for (int i = 0; i != buffer.size(); i++)
    {
        vertex3D p = buffer[i];
        float xyz[3];
        xyz[0] = p.x;
        xyz[1] = p.y;
        xyz[2] = p.z;
       if(axis == 0) m.x_Rotation_Matrix(angle_Delta, xyz);
       if(axis == 1) m.y_Rotation_Matrix(angle_Delta, xyz);
       if(axis == 2) m.z_Rotation_Matrix(angle_Delta, xyz);
        p.x = xyz[0];
        p.y = xyz[1];
        p.z = xyz[2];
        buffer[i] = p;
    }
}


void GPGUI::write_Clusteter_Cube_Mesh_To_Vector
(
    int Q,
    float cube_Size,
    float Xo,
    float Yo,
    float Zo,
    int fill_Type,
    vector<vertex3D>& buffer
)
{
    vertex3D vertex_a;
    vertex3D vertex_b;
   

    //X Z PLANE
    for (float Vz = 0; Vz < cube_Size/2; Vz++)
    {     
            for (float Vx = 0; Vx < cube_Size / 2; Vx++)
            {              
                    vertex_a.x = Vx +Xo;
                    vertex_a.z = Vz +Zo;
                    vertex_a.y = Yo;
                    vertex_a.c = red;
                    //
                    vertex_b.x = Vx * -1 +Xo;
                    vertex_b.z = Vz + Zo;
                    vertex_b.y = Yo;
                    vertex_b.c = red;
               

                    //
                    buffer.push_back(vertex_a);
                    buffer.push_back(vertex_b);              
            }

            for (float Vx = 0; Vx < cube_Size / 2; Vx++)
            {            
                    vertex_a.x = Vx+Xo;
                    vertex_a.z = Vz * -1+Zo;
                    vertex_a.y = Yo;
                    vertex_a.c = red;
                    //
                    vertex_b.x = Vx * -1+Xo;
                    vertex_b.z = Vz * -1+Zo;
                    vertex_b.y = Yo;
                    vertex_b.c = red;
                    //
                    buffer.push_back(vertex_a);
                    buffer.push_back(vertex_b);               
            }
    }




    //X Y PLANE
    for (float Vy = 0; Vy < cube_Size / 2; Vy++)
    {
        for (float Vx = 0; Vx < cube_Size / 2; Vx++)
        {
            vertex_a.x = Vx + Xo;
            vertex_a.z = Zo;
            vertex_a.y = Vy + Yo;
            vertex_a.c = green;
            //
            vertex_b.x = Vx * -1 + Xo;
            vertex_b.z = Zo;
            vertex_b.y = Vy + Yo;
            vertex_b.c = green;
            //
            buffer.push_back(vertex_a);
            buffer.push_back(vertex_b);
        }

        for (float Vx = 0; Vx < cube_Size / 2; Vx++)
        {
            vertex_a.x = Vx + Xo;
            vertex_a.z = Zo;
            vertex_a.y = Vy*-1 + Yo;
            vertex_a.c = green;
            //
            vertex_b.x = Vx * -1 + Xo;
            vertex_b.z = Zo;
            vertex_b.y = Vy*-1 + Yo;
            vertex_b.c = green;
            //
            buffer.push_back(vertex_a);
            buffer.push_back(vertex_b);
        }
    }









}


void GPGUI::line_Gen_3D
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
)
{
    int line_Id = id;
    //MANUALY CREATE LINE
    vertex3D v1,v2;
  

    v1.mesh_Type = 1;
    v2.mesh_Type = 1;
    v1.id = line_Id;
    v2.id = line_Id;
    v1.c = color;
    v2.c = color;
    
    v1.x = p1x;
    v1.y = p1y;
    v1.z = p1z;
    
    v2.x = p2x;
    v2.y = p2y;
    v2.z = p2z;


    //PUSH LINE INTO LINE BUFFER
    world_Space_Vertex.push_back(v1);
    world_Space_Vertex.push_back(v2);
}



void GPGUI::triangle_Gen_3D//vertex version
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

)
{
    vertex3D p0, p1, p2;

    p0.x = p1x;
    p0.y = p1y;
    p0.z = p1z;
    p0.u = U;
    p0.v = V;
    p0.mesh_Type = 2;
    p0.c = color;
    p0.id = id;


    p1.x = p2x;
    p1.y = p2y;
    p1.z = p2z;
    p1.u = U;
    p1.v = V;
    p1.mesh_Type = 2;
    p1.c = color;
    p1.id = id;



    p2.x = p3x;
    p2.y = p3y;
    p2.z = p3z;//*-1//INVERT Z AXIS
    p2.u = U;
    p2.v = V;
    p2.mesh_Type = 2;
    p2.c = color;
    p2.id = id;
    

    ////CALC CROSS PRODUCT OF TRIANGLE
    ////create 3 vectors


    //static float V1[3];
    //static float V2[3];
    //static float Vn[3];

    //
    //V1[0]  = p1.x;
    //V1[1]  = p1.y;
    //V1[2]  = p1.z;
    ////

    //V2[0] = p2.x;
    //V2[1] = p2.y;
    //V2[2] = p2.z;
    ////

    //m.vector_Cross_Product(V1, V2, Vn);

    //    //CALC LENGTH OF NORMAL VECTOR
    //    float NL =
    //    sqrt
    //    (
    //        Vn[0] * Vn[0] +
    //        Vn[1] * Vn[1] +
    //        Vn[2] * Vn[2]
    //    );
    //  

    //vertex3D norm_Point;

    //norm_Point.x = Vn[0]/NL;//DIVIDE BY NL TO NORMALIZE NORMAL VECTOR
    //norm_Point.y = Vn[1]/NL;
    //norm_Point.z = Vn[2]/NL;
    //norm_Point.c = yellow;
    //norm_Point.mesh_Type = 0;
    //norm_Point.id = 0;
    //Serial.println(Vn[0]/NL);
    //Serial.println(Vn[1]/NL);
    //Serial.println(Vn[2]/NL);

   // world_Space_Vertex.push_back(norm_Point);
    world_Space_Vertex.push_back(p0);
    world_Space_Vertex.push_back(p1);
    world_Space_Vertex.push_back(p2);
   // world_Space_Vertex.push_back(norm_Point);

}


void GPGUI::translate_Cube_Vertex_Buff_To_3D_Triangles//vertex version
(
    float cube_Buff[8][3],
    vector<vertex3D>& world_Space_Vertex_Buffer
)
{
    int ID = 1;  
    int color = green;


    //iterate through triangle vertex order buffer
    for (int i = 0; i < 12; i++)
    {
        float p1x = cube_Buff[ triangle_Vertex_Order_Cube[i][0] ]    [0];// face trirangle p1
        float p1y = cube_Buff[ triangle_Vertex_Order_Cube[i][0] ]    [1];
        float p1z = cube_Buff[ triangle_Vertex_Order_Cube[i][0] ]    [2];
        //
        float p2x = cube_Buff[ triangle_Vertex_Order_Cube[i][1] ]    [0];// face trirangle p2       
        float p2y = cube_Buff[ triangle_Vertex_Order_Cube[i][1] ]    [1];
        float p2z = cube_Buff[ triangle_Vertex_Order_Cube[i][1] ]    [2];
        //
        float p3x = cube_Buff[ triangle_Vertex_Order_Cube[i][2] ]    [0];// face trirangle p3       
        float p3y = cube_Buff[ triangle_Vertex_Order_Cube[i][2] ]    [1];
        float p3z = cube_Buff[ triangle_Vertex_Order_Cube[i][2] ]    [2];
        
        ////calc normal per triangle
        //float ux = p2x - p1x;
        //float uy = p2y - p1y;
        //float uz = p2z - p1z;
        //float wx = p3x - p1x;
        //float wy = p3y - p1y;
        //float wz = p3z - p1z;
        //float nx = ux * wx;
        //float ny = uy * wy;
        //float nz = uz * wz;
        //float dotP = (nx * camera_VX) + (ny * camera_VY) + (nz * camera_VZ);
        //

            triangle_Gen_3D
            (
                p1x,// face trirangle p1        
                p1y,
                p1z,
                //
                p2x,// face trirangle p2       
                p2y,
                p2z,
                //
                p3x,// face trirangle p3       
                p3y,
                p3z,
                0,
                0,
                color,
                ID,
                world_Space_Vertex_Buffer
            );
        ID++;
    }



}






void GPGUI::translate_Cube_Vertex_Buff_To_Triangle_Buff//vertex version
(
    float cube_Buff[8][3],
    vector<triangle>&  buff   
  
)
{
  
    int color = green;


    //iterate through triangle vertex order buffer
    for (int i = 0; i < 12; i++)
    {
        float p1x = cube_Buff[triangle_Vertex_Order_Cube[i][0]][0];// face trirangle p1
        float p1y = cube_Buff[triangle_Vertex_Order_Cube[i][0]][1];
        float p1z = cube_Buff[triangle_Vertex_Order_Cube[i][0]][2];
        //
        float p2x = cube_Buff[triangle_Vertex_Order_Cube[i][1]][0];// face trirangle p2       
        float p2y = cube_Buff[triangle_Vertex_Order_Cube[i][1]][1];
        float p2z = cube_Buff[triangle_Vertex_Order_Cube[i][1]][2];
        //
        float p3x = cube_Buff[triangle_Vertex_Order_Cube[i][2]][0];// face trirangle p3       
        float p3y = cube_Buff[triangle_Vertex_Order_Cube[i][2]][1];
        float p3z = cube_Buff[triangle_Vertex_Order_Cube[i][2]][2];



        triangle Ti;
        Ti.p1.x = p1x;
        Ti.p1.y = p1y;
        Ti.p1.z = p1z;

        Ti.p2.x = p2x;
        Ti.p2.y = p2y;
        Ti.p2.z = p2z;

        Ti.p3.x = p3x;
        Ti.p3.y = p3y;
        Ti.p3.z = p3z;
        Ti.face_Color = color;
        Ti.id = i;

        buff.push_back(Ti);
      
       // vBuff.push_back(norm_Point);
       
    }



}





void GPGUI::draw_Axis_Dials
(
    int x_Start,
    int y_Start,
    float X,
    float Y,
    float Z
)
{
    int bin;






    drawSolidCircle(x_Start, y_Start, 12, black);
    print("N", x_Start - 2, y_Start - 10, yellow, black);
    print("S", x_Start - 1, y_Start + 4, yellow, black);
    print("E", x_Start + 5, y_Start - 3, yellow, black);
    print("W", x_Start - 9, y_Start - 3, yellow, black);
    drawHollowCircle(x_Start, y_Start, 12, red);
    draw_Vector     (x_Start, y_Start, 10, X, 0, &bin, &bin, green);






    drawSolidCircle(x_Start, y_Start + 27, 12, black);
    print("N", x_Start - 2, (y_Start+27) - 10, yellow, black);
    print("S", x_Start - 1, (y_Start+27) + 4, yellow, black);
    print("E", x_Start + 5, (y_Start+27) - 3, yellow, black);
    print("W", x_Start - 9, (y_Start+27) - 3, yellow, black);
    drawHollowCircle(x_Start, y_Start + 27, 12, green);
    draw_Vector(x_Start, y_Start + 27, 10, Y, 0, &bin, &bin, green);







    drawSolidCircle(x_Start, y_Start + 54, 12, black);
    print("N", x_Start - 2,  (y_Start + 54) - 10, yellow, black);
    print("S", x_Start - 1,  (y_Start + 54) + 4, yellow, black);
    print("E", x_Start + 5,  (y_Start + 54) - 3, yellow, black);
    print("W", x_Start - 9,  (y_Start + 54) - 3, yellow, black);
    drawHollowCircle(x_Start, y_Start + 54, 12, blue);
    draw_Vector(x_Start, y_Start +      54, 10, Z, 0, &bin, &bin, green);
}


//#################################################################################################################
//                                             END VERTEX PIPE LINE
//#################################################################################################################



//#################################################################################################################
//                                             TRIANGLE PIPE LINE
//#################################################################################################################


void GPGUI::translate_Triangle_Buffer_To_Pixel_Buffer
(
    int xStart,
    int yStart,
    int window_Width,
    int window_Height,
    float camera_X,
    float camera_Y,
    float camera_Z,
    bool back_Face_Cliping_Enable,
    vector<triangle>& triangle_Buffer,
    vector<triangle>& pixel_Buffer
)

{
    static bool debug = false;
    float FOV = 60;
    float fov = m.deg_To_Rad(FOV);
    float camera_Plane_X = 2 * ((tan(fov / 2)) * camera_Z);

    int v_Count = 0;


    //DEFINE WORLD PLANE SIZE
    float world_Size = 1000;
    float wp1_X = (world_Size / 2) * -1;
    float wp1_Y = (world_Size / 2) * -1;

    float wp2_X = (world_Size / 2);
    float wp2_Y = (world_Size / 2);


    //LIMIT VIEW PLANE TO WORLD SIZE
    if (camera_Plane_X > world_Size) camera_Plane_X = world_Size - 1;
    //




    //DEFINE WORLD VIEW PLANE
    float plane_Index_X = camera_X;//LOOK AT ORIGIN ( VIEW PLANE CENTER VERTEX) 
    float plane_Index_Y = camera_Y;
    //
    float vp1_X = (((camera_Plane_X / 2) * -1) + plane_Index_X);
    float vp1_Y = (((camera_Plane_X / 2) * -1) + plane_Index_Y);
    //
    float vp2_X = ((camera_Plane_X / 2) + plane_Index_X);
    float vp2_Y = ((camera_Plane_X / 2) + plane_Index_Y);
    //

    if (vp1_X < wp1_X) { vp1_X += 1; vp2_X += 1; beep(1000, 50); }
    if (vp2_X > wp2_X) { vp1_X -= 1; vp2_X -= 1; beep(1000, 50); }

    if (vp1_Y < wp1_Y) { vp1_Y += 1; vp2_Y += 1; beep(1000, 50); }
    if (vp2_Y > wp2_Y) { vp1_Y -= 1; vp2_Y -= 1; beep(1000, 50); }

    //DEFINE SCREEN PLANE
    float sp1_X = xStart;
    float sp1_Y = yStart;
    //
    float sp2_X = xStart + window_Width;
    float sp2_Y = yStart + window_Height;
    //

 

    float vertex[3];//TEMP BUFFER TO HOLD WORLD VERTEX TO BE TRANSLATED TO SCREEN SPACE
    int Tx1;//TRANSLATED SCREEN SPACE VERTEX
    int Ty1;
    int Tx2;//TRANSLATED SCREEN SPACE VERTEX
    int Ty2;
    int Tx3;//TRANSLATED SCREEN SPACE VERTEX
    int Ty3;



    //LOOP THROUGH TRIANGLE BUFFER TRANSLATING TO PIXEL SPACE
    for (int i = 0; i != triangle_Buffer.size(); i++)
    {
            //CREATE 3 TEMP VERTICIES AND A TRIANGLE FOR TRANSLATING 
            triangle Ti = triangle_Buffer[i];
            triangle Tt;

            float vertex_To_Translate_1[3];
            float vertex_To_Translate_2[3];
            float vertex_To_Translate_3[3];

            //CALC CROSS PRODUCT OF TRiangle
            static float V1[3];
            static float V2[3];
            static float Vn[3];

            //SUBTRACT TRIANGLE ORIGIN
            V1[0] = Ti.p2.x - Ti.p1.x;   //p2x - p1x;
            V1[1] = Ti.p2.y - Ti.p1.y;   //p2y - p1y;
            V1[2] = Ti.p2.z - Ti.p1.z;   //p2z - p1z;
            //

            V2[0] = Ti.p3.x - Ti.p1.x;   //p3x - p1x;
            V2[1] = Ti.p3.y - Ti.p1.y;   //p3y - p1y;
            V2[2] = Ti.p3.z - Ti.p1.z;   //p3z - p1z;
            ////

            m.vector_Cross_Product(V1, V2, Vn);


            float Vcv[3] = {0,  0,  1};//camera vector
            float Vlv[3];//light vector

            set_Light_Vector_Dir(45,0,90,Vlv);

   
            float camera_Dot_Product;
            float light_Dot_Product;

            m.vector_Dot_Product(Vcv,Vn,&camera_Dot_Product);
            m.vector_Dot_Product(Vlv, Vn, &light_Dot_Product);
        
  

            //TRANSLATE P0
            vertex_To_Translate_1[0] = Ti.p1.x;
            vertex_To_Translate_1[1] = Ti.p1.y;
            vertex_To_Translate_1[2] = Ti.p1.z;
            m.world_plane_To_Screen_Plane_Vertex_Translate(vp1_X, vp1_Y, vp2_X, vp2_Y, sp1_X, sp1_Y, sp2_X, sp2_Y, vertex_To_Translate_1, &Tx1, &Ty1);

            //TRANSLATE P1
            vertex_To_Translate_2[0] = Ti.p2.x;
            vertex_To_Translate_2[1] = Ti.p2.y;
            vertex_To_Translate_2[2] = Ti.p2.z;
            m.world_plane_To_Screen_Plane_Vertex_Translate(vp1_X, vp1_Y, vp2_X, vp2_Y, sp1_X, sp1_Y, sp2_X, sp2_Y, vertex_To_Translate_2, &Tx2, &Ty2);

            //TRANSLATE P2
            vertex_To_Translate_3[0] = Ti.p3.x;
            vertex_To_Translate_3[1] = Ti.p3.y;
            vertex_To_Translate_3[2] = Ti.p3.z;
            m.world_plane_To_Screen_Plane_Vertex_Translate(vp1_X, vp1_Y, vp2_X, vp2_Y, sp1_X, sp1_Y, sp2_X, sp2_Y, vertex_To_Translate_3, &Tx3, &Ty3);



            //WRITE CURRENT TRIANGLE CROSS and dot PRODUCT VECTOR TO TRIANGLE
            Tt.Nx = Vn[0];
            Tt.Ny = Vn[1];
            Tt.Nz = Vn[2];
            Tt.Dp_Camera = camera_Dot_Product;
            Tt.Dp_Light = light_Dot_Product;
            


            Tt.p1.x = Tx1;
            Tt.p1.y = Ty1;
            Tt.p1.z = 0;

            Tt.p2.x = Tx2;
            Tt.p2.y = Ty2;
            Tt.p2.z = 0;

            Tt.p3.x = Tx3;
            Tt.p3.y = Ty3;
            Tt.p3.z = 0;
            Tt.face_Color = red;
            Tt.id = i;



       


            static int color;
           // Serial.println(cammera_Dot_Product);
            if (light_Dot_Product > 0)
            {              
                m.interp(1, 3, light_Dot_Product, &color);//ADD COLOR LOOK UP TABLE HERE
                Tt.face_Color = color;
            }
            else Tt.face_Color = 1;


                          
          if(camera_Dot_Product > 0 && back_Face_Cliping_Enable)  pixel_Buffer.push_back(Tt);
          if(!back_Face_Cliping_Enable) pixel_Buffer.push_back(Tt);

     

    }





}


void GPGUI::rotate_Triangle_Vector
(
    vector<triangle>& triangle_Buffer,
    int axis,
    float An
)
{
   

    for (int i = 0; i != triangle_Buffer.size(); i++)
    {
        float p1[3];
        float p2[3];
        float p3[3];
        triangle Ti = triangle_Buffer[i];
        triangle Tit;

        p1[0] = Ti.p1.x;
        p1[1] = Ti.p1.y;
        p1[2] = Ti.p1.z;

        p2[0] = Ti.p2.x;
        p2[1] = Ti.p2.y;
        p2[2] = Ti.p2.z;

        p3[0] = Ti.p3.x;
        p3[1] = Ti.p3.y;
        p3[2] = Ti.p3.z;


        switch (axis)
        {
        case 0:
            m.x_Rotation_Matrix(An, p1);
            m.x_Rotation_Matrix(An, p2);
            m.x_Rotation_Matrix(An, p3);
            break;

        case 1:
            m.y_Rotation_Matrix(An, p1);
            m.y_Rotation_Matrix(An, p2);
            m.y_Rotation_Matrix(An, p3);
            break;


        case 2:
            m.z_Rotation_Matrix(An, p1);
            m.z_Rotation_Matrix(An, p2);
            m.z_Rotation_Matrix(An, p3);
            break;



        }

        Tit.p1.x = p1[0];
        Tit.p1.y = p1[1];
        Tit.p1.z = p1[2];

        Tit.p2.x = p2[0];
        Tit.p2.y = p2[1];
        Tit.p2.z = p2[2];

        Tit.p3.x = p3[0];
        Tit.p3.y = p3[1];
        Tit.p3.z = p3[2];
        Tit.face_Color = Ti.face_Color;
        Tit.id = Ti.id;
        triangle_Buffer[i] = Tit;
    }

}




void GPGUI::rotate_World_Vertex
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
)

{


        if (x_Axis_Rotate_Enable)
        {
         
            rotate_Mesh(buff, 0, x_Angle_Delta);
        }

        if (y_Axis_Rotate_Enable)
        {
 
            rotate_Mesh(buff, 1, y_Angle_Delta);
        }

        if (z_Axis_Rotate_Enable)
        {
           
            rotate_Mesh(buff, 2, z_Angle_Delta);
        }


  
  


}



void GPGUI::rotate_Obj_Triangle
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
)

{


    if (x_Axis_Rotate_Enable)
    {

        rotate_Triangle_Vector (buff, 0, x_Angle_Delta);
    }

    if (y_Axis_Rotate_Enable)
    {

        rotate_Triangle_Vector(buff, 1, y_Angle_Delta);
    }

    if (z_Axis_Rotate_Enable)
    {

        rotate_Triangle_Vector(buff, 2, z_Angle_Delta);
    }






}


void GPGUI::move_Background_Vertex_Buffer_To_World_Vertex_Buffer
(
    vector<vertex3D>& back_Buff,
    vector<vertex3D>& world_Buff

)
{
    for (int i = 0; i != back_Buff.size(); i++)
    {
        world_Buff.push_back(back_Buff[i]);
    }
}

void GPGUI::move_Obj_Triangle_Buffer_To_World_Triangle_Buffer
(
    vector<triangle>& back_Buff,
    vector<triangle>& world_Buff

)
{
    for (int i = 0; i != back_Buff.size(); i++)
    {
        world_Buff.push_back(back_Buff[i]);
    }
}




void GPGUI::triangle_Buff_To_Screen

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
)
{
    vector<triangle> screen_Space_Triangle_Buffer;

 

    translate_Triangle_Buffer_To_Pixel_Buffer
    (
        window_Start_X,
        window_Start_Y,
        window_Size,
        window_Size,
        camera_X,
        camera_y,
        camera_z,
        back_Face_Culling_Enable,
        triangle_Buffer,
        screen_Space_Triangle_Buffer
    );
    




    //draw triangles in buffer
    for (int i = 0; i != screen_Space_Triangle_Buffer.size(); i++)
    {
        triangle Ti = screen_Space_Triangle_Buffer[i];
        drawTriangle //SEND TO VGA BACK BUFFER
        (
            (int)Ti.p1.x,
            (int)Ti.p1.y,
            (int)Ti.p2.x,
            (int)Ti.p2.y,
            (int)Ti.p3.x,
            (int)Ti.p3.y,
            wire_Frame_Enable,
            Ti.face_Color
        );
    }


    *triangle_Count = screen_Space_Triangle_Buffer.size();
    triangle_Buffer.clear();
    screen_Space_Triangle_Buffer.clear();

}





void GPGUI::set_Light_Vector_Dir
(
    float Xan,
    float Yan,
    float Zan,
    float Vlv[3]

)

{
    Vlv[0] = 1;
    Vlv[1] = 0;
    Vlv[2] = 0;
    m.z_Rotation_Matrix(Zan, Vlv);
    m.y_Rotation_Matrix(Yan, Vlv);
    m.x_Rotation_Matrix(Xan, Vlv);
    



}

//#################################################################################################################
//                                             END TRIANGLE PIPE LINE
//#################################################################################################################












void GPGUI::raycast_Map_From_Ram
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
    int* hit_Cell_Y_Left,
    int* hit_Cell_X_Center,
    int* hit_Cell_Y_Center,
    int* hit_Cell_X_Right,
    int* hit_Cell_Y_Right,
    int* target_Cell_Data,
    bool camera_Cross_Hair_Enable,
    bool debug

)

{
    //RUN ERROR CHECK ON MAP RAM
    ram_Data_Limit_Clip(ram);
    //if (FOV < 40) FOV = 40;
    //if (FOV > 90) FOV = 90;
    //
    static byte texture_Size = 16;
    static bool left;
    static bool right;
    static bool up;
    static bool down;
    static double rotationStep = 1;
    static double dirX = 1, dirY = 0; //initial direction vector -1 = left to right, +1 = right to left
    static double planeX = 0, planeY = 0.66;       // * 3.144 / 180;; //the 2d GPGUI version of camera plane 0.66 - 0.90
    static double time;
    static double oldTime;
    static double cameraX;
    static double rayDirX;
    static double rayDirY;
    static double moveSpeed;
    static double rotSpeed;
    static double frameTime;
    static int pos;
    byte texWidth = texture_Size;
    byte texHeight = texture_Size;
    int tex_Size = texture_Size * texture_Size;
    int map_Grid_Mem_Start = 10;
    static int mid_Screen = screen_Height / 2;
    float camera_UP_Down_Step = screen_Height * 0.02;
    static float camera_Angle = 0; // STARTING CAMERA ANGLE

    //CLIP VIEW PORT TO SCREEN SIZE
    if (screen_Height > 240) screen_Height = 240;
    if (screen_Width > 320) screen_Width = 320;


    //READ MAP WIDTH AND HEIGHT FROM MEMORY START LOCATION FIRST TWO ADDRESSES
    byte mapWidth = ram[0];
    byte mapHeight = ram[1];
    byte roof_Color = ram[2];
    byte floor_Color = ram[3];
    static byte posX_Start = ram[4];
    static byte posY_Start = ram[5];
    static float posX = posX_Start;
    static float posY = posY_Start;
    static float angle_Step = 0;//ROTATION DELTA IN DEGREES

    if (mapWidth > 60) mapWidth = 60;
    if (mapWidth < 0)  mapWidth = 0;

    if (mapHeight > 60) mapHeight = 60;
    if (mapHeight < 0)  mapHeight = 0;


    //LIMIT TO MAP AREA
    if (posX > (float)mapWidth - 1) { posX = (float)mapWidth - 1.05; beep(500, 10); }
    if (posY > (float)mapHeight - 1) { posY = (float)mapHeight - 1.05; beep(500, 10); }
    if (posX < 1) { posX = 1.05; beep(500, 10); }
    if (posY < 1) { posY = 1.05; beep(500, 10); }
    //

    //LOOK UP AND DOWN
    if (camera_Up && mid_Screen < screen_Height - 10) mid_Screen += camera_UP_Down_Step;//CAMERA UP
    if (camera_Down && mid_Screen > 10) mid_Screen -= camera_UP_Down_Step;//CAMERA DOWN



    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    for (int x = 0; x < screen_Width; x++)// start of raycast loop
    {
        //calculate ray position and direction
        cameraX = 2 * x / double(screen_Width) - 1; //x-coordinate in camera space
        rayDirX = dirX + planeX * cameraX;
        rayDirY = dirY + planeY * cameraX;
        double perpWallDist;//ray length
        double ray_Length;
        byte color;
        int side; //was a NS or a EW wall hit?
        unsigned int mapX = int(posX);//which box of the map we're in
        unsigned int mapY = int(posY);
        double sideDistX;  //length of ray from current position to next x or y-side
        double sideDistY;
        double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX); //length of ray from one x or y- side to next x or y- side
        double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
        int stepX; //what direction to step in x or y- direction (either +1 or -1)
        int stepY;
        int hit = 0; //was there a wall hit?


            //calculate step and initial sideDist
        if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
        else { stepX = 1;  sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
        if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
        else { stepY = 1;  sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////

            //perform DDA Cast out ray
        while (hit == 0)
        {
            //jump to next map square, either in x- direction, or in y- direction
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }

            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }


            //INDEX INTO MAP RAM
            pos = mapY * mapWidth + mapX;//calculate memory position
                  //limit ray to map boundry


            //STOP RAY AT MAP BOUNDRY
            if (mapX >= mapWidth - 1 || mapY >= mapHeight - 1 || mapX <= 0 || mapY <= 0)
            {
                hit = 1;
                color = roof_Color;
            }

            //STOP RAY AT ANY CELL ABOVE ZERO
            if (ram[pos + map_Grid_Mem_Start] > 0)
            {
                hit = 1;
            }

            //RETURN LEFT RAY CELL HIT DATA
            if (x == 0)
            {
                *hit_Cell_X_Left = mapX;
                *hit_Cell_Y_Left = mapY;
            }

            //RETURN CENTER RAY CEL HIT DATA 
            if (x == screen_Width / 2)
            {
                *hit_Cell_X_Center = mapX;
                *hit_Cell_Y_Center = mapY;
                *target_Cell_Data = ram[pos + map_Grid_Mem_Start];
            }


            //RETURN RIGHT RAY CELL HIT DATA
            if (x == screen_Width - 1)
            {
                *hit_Cell_X_Right = mapX;
                *hit_Cell_Y_Right = mapY;
            }
        }//END OF ONE RAY/////////////////////////////////////////////////////////////////////



            //read current ram cell data for wall color
        byte cell_Data = ram[pos + map_Grid_Mem_Start];

        //!!!!!!!!!!!!!!!!!RENDER WALLS AND ROOF!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if (side == 0) { perpWallDist = (sideDistX - deltaDistX); }//Calculate distance projected on camera direction to wall
        else { perpWallDist = (sideDistY - deltaDistY); }
        int lineHeight = screen_Height / perpWallDist;//Calculate height of line to draw on screeN                   
        int drawStart = -lineHeight / 2 + mid_Screen;//Calculate lowest y and highest y pixel of the vertical x line
        if (drawStart < 0) drawStart = 0;

        int drawEnd = lineHeight / 2 + mid_Screen;
        ray_Length = perpWallDist;


        //DRAW ROOF NO TEXTURE                                                                                                                                           
        drawLine(x + xStart, yStart, x + xStart, drawStart + yStart, ram[2]);


        //DRAW FLOOR NO TEXTURE
        if (drawEnd < screen_Height) drawLine(x + xStart, drawEnd + yStart, x + xStart, screen_Height + yStart, floor_Color);



        // wall texturing calculations/////////////////////////////////////////////////////////

                 //calculate value of wallX
        double wallX; //where exactly the wall was hit
        if (side == 0) wallX = posY + perpWallDist * rayDirY;
        else           wallX = posX + perpWallDist * rayDirX;
        wallX -= floor((wallX));

        //x coordinate on the texture
        int texX = 15 - int(wallX * double(texWidth));
        if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
        if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

        // How much to increase the texture coordinate per screen pixel
        double step = 1.0 * texHeight / lineHeight;

        // Starting texture coordinate
        double texPos = (drawStart - mid_Screen + lineHeight / 2) * step;
        //


            //draw wall line from texture buffer
        for (int y = drawStart; y < drawEnd; y++)
        {
            // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
            int texY = (int)texPos & (texHeight - 1);
            texPos += step;

            if (cell_Data >= 1 && cell_Data <= 16)
                color = ram_Texture[texHeight * texY + (texX + tex_Size * (cell_Data - 1))];

            //clip pixels from the top and bottom of view the port
            if (y + yStart < screen_Height + yStart && y + yStart >= yStart)
                setPixel(x + xStart, y + yStart, color);

        }//END OF WALL COLLUM TEXTURE RENDER LOOP   
    }//end OF FOV SCAN
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



    int cell_Address = ((int)posY * mapWidth + (int)posX) + map_Grid_Mem_Start;
    int cell_DATA = ram[cell_Address];


    //timing for input 
    oldTime = time;
    time = millis();
    frameTime = (time - oldTime) / 1000.0; //frameTime is the time this frame has taken, in seconds
    move_Speed = screen_Height * .02;
    moveSpeed = frameTime * move_Speed; //the constant value is in squares/second
    rotSpeed = frameTime * turn_Speed;

    angle_Step = rotSpeed * 180 / 3.142;// rotate angle delta from radians to degrees


    if (rotate_Right)//right
    {
        rotate_Left = false;
        //both camera direction and camera plane must be rotated
        double oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);

        double oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        camera_Angle += angle_Step;

    }

    //ADD OPOSITE CLEAR FLAGS
    if (rotate_Left)//left
    {
        rotate_Right = false;
        //both camera direction and camera plane must be rotated
        double oldDirX = dirX;
        dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);

        double oldPlaneX = planeX;
        planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        camera_Angle -= angle_Step;

    }

    if (move_Back)
    {
        posX -= dirX * moveSpeed;
        posY -= dirY * moveSpeed;
    }

    if (move_Foward)
    {
        if (cell_DATA == 0)
        {
            posX += dirX * moveSpeed;
            posY += dirY * moveSpeed;
        }
        if (cell_DATA > 0)
        {
            posX -= dirX * moveSpeed;
            posY -= dirY * moveSpeed;
            beep(1000, 5);
        }
    }

    //RETURN THE CURRNT MAP X AND Y POS
    *camera_Map_Pos_X = (int)posX;
    *camera_Map_Pos_Y = (int)posY;
    //

    if (camera_Angle > 359) camera_Angle = 0;
    if (camera_Angle < 0)   camera_Angle = 359;

    *camera_Angle_Out = camera_Angle;





    if (camera_Cross_Hair_Enable)
    {
        drawLine(xStart + 5, yStart + screen_Height / 2, xStart + screen_Width - 5, yStart + screen_Height / 2, red);//X LINE
        drawLine(xStart + screen_Width / 2, yStart + 5, xStart + screen_Width / 2, yStart + screen_Height - 5, green);//Y LINE

    }


    if (debug)
    {
        print("posX", 0, 10, red, black);             printNumberDouble(posX, 80, 10, green, black);
        print("posY", 0, 20, red, black);             printNumberDouble(posY, 80, 20, green, black);
        print("mapX", 0, 30, red, black);             printNumberInt((int)posX, 80, 30, green, black);
        print("mapY", 0, 40, red, black);             printNumberInt((int)posY, 80, 40, green, black);
        print("CELL ADDRESS", 0, 50, red, black);     printNumberInt(cell_Address, 80, 50, green, black);
        print("CELL DATA", 0, 60, red, black);        printNumberInt(cell_DATA, 80, 60, green, black);
        print("MAP WIDTH", 0, 80, red, black);        printNumberInt(ram[0], 80, 80, green, black);
        print("MAP HEIGHT", 0, 90, red, black);       printNumberInt(ram[1], 80, 90, green, black);
        print("CAMERA ANGLE", 0, 100, red, black);    printNumberInt(camera_Angle, 80, 100, green, black);
    }

}


void GPGUI::write_Default_Map_To_Ram(byte width, byte height, byte map_Ram[4096], byte wall)
{
    //INPUT ERROR HANDLE
    if (width < 4) width = 4;
    if (width > 60) width = 60;

    if (height < 4)  height = 4;
    if (height > 60) height = 60;





    int mem_Address = 10;
    map_Ram[0] = width;
    map_Ram[1] = height;



    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            //DRAW WALL AROUND BOARDER
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) { map_Ram[mem_Address] = wall; }
            mem_Address++;
        }
    }
}

void GPGUI::write_Default_Sprite_Texture_To_Ram(int mem_Address, byte tex_Ram[4096])
{
    static byte texSize = 16;
    static bool bit = 0;
    int mem_Start = mem_Address;
    //write texture to texture ram location 1
    int tex_Size = texSize * texSize;
    byte tex_Color = mem_Address;

    if (mem_Start < 1) { mem_Start = 1;  mem_Address = 1; }
    if (mem_Start > 16) { mem_Start = 16; mem_Address = 16; }

    byte TEX[16][16] =
    {
      {4,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
      {4,4,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
      {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
      {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
      {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
      {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
      {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
      {1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
      {1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
      {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1},
      {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
      {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
      {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
      {1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1},
      {1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}

    };






    //index to start address
    if (mem_Address == 1)  mem_Start = 0;
    else if (mem_Address > 1 && mem_Address <= 16)  mem_Start = tex_Size * (mem_Address - 1);





    for (int y = 0; y < texSize; y++)//DEFAULT TEXTURE CROSS 
    {
        for (int x = 0; x < texSize; x++)
        {
            tex_Ram[mem_Start] = TEX[y][x];
            mem_Start++;
        }
    }


}

void GPGUI::write_Default_Wall_Texture_To_Ram(int mem_Address, byte tex_Ram[4096])
{
    static byte texSize = 16;
    static bool bit = 0;
    int mem_Start = mem_Address;

    if (mem_Start < 1) { mem_Start = 1;  mem_Address = 1; }
    if (mem_Start > 16) { mem_Start = 16; mem_Address = 16; }

    //write texture to texture ram location 1
    int tex_Size = texSize * texSize;
    byte tex_Color = mem_Address;


    //index to start address
    if (mem_Address == 1)  mem_Start = 0;
    else if (mem_Address > 1 && mem_Address <= 16)  mem_Start = tex_Size * (mem_Address - 1);





    for (int y = 0; y < texSize; y++)//DEFAULT TEXTURE CROSS HATCH BLACK AND WHITE
    {
        for (int x = 0; x < texSize; x++)
        {
            if (bit)
            {
                if (y == 0 || x == 0) { tex_Ram[mem_Start] = red;  bit = 0; }
                else { tex_Ram[mem_Start] = white; bit = 0; }
            }
            else { tex_Ram[mem_Start] = tex_Color;  bit = 1; }
            mem_Start++;
        }




        if (!bit)  bit = 1;
        else      bit = 0;
    }

    //set start of texture to green
    if (mem_Address == 1)  tex_Ram[0] = green;
    else if (mem_Address > 1 && mem_Address < 17)  tex_Ram[tex_Size * (mem_Address - 1)] = green;
}

void GPGUI::write_Texture_To_Ram(int mem_Address, byte tex_Ram[4096], byte tex_Buffer[255])
{
    const byte texture_Size = 16;
    int mem_Start = mem_Address;
    //write texture to texture ram location 1
    int tex_Size = texture_Size * texture_Size;

    //index to start address
    if (mem_Address == 1)  mem_Start = 0;
    else if (mem_Address > 1 && mem_Address < 17)  mem_Start = tex_Size * (mem_Address - 1);

    int tex_Address = 0;



    for (int y = 0; y < texture_Size; y++)
    {
        for (int x = 0; x < texture_Size; x++)
        {
            tex_Ram[mem_Start] = tex_Buffer[tex_Address];
            tex_Address++;
            mem_Start++;
        }
    }
}

void GPGUI::draw_Tile_Map
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
)
{

    ram_Data_Limit_Clip(map);
    byte texture_Size = 16;
    byte mapWidth = map[0];
    byte mapHeight = map[1];
    int xpos = xStart;
    int ypos = yStart;
    int mem_Address = 10;

    //limit camera pos
    if (cell_X < 1 || cell_X > mapWidth)  cell_X = mapWidth / 2;
    if (cell_Y < 1 || cell_Y > mapHeight) cell_Y = mapHeight / 2;




    static byte grid_Window_Square_Size = 150;
    static byte tile_Size = grid_Window_Square_Size / map[0];

    //SCALE GRID TO WINDOW AREA
    if (xStart + grid_Window_Square_Size > 320) xStart = 320 - grid_Window_Square_Size;
    if (yStart + grid_Window_Square_Size > 240) yStart = 240 - grid_Window_Square_Size;
    //
    if (tile_Size * map[0] > grid_Window_Square_Size) tile_Size--;
    if (tile_Size * map[1] > grid_Window_Square_Size) tile_Size--;

    //DRAW BACKGROUND
    drawSolidRectangle(xStart - 1, yStart - 1, 181, 181, map[2]);
    // drawHollowRectangle(xStart-1, yStart-1, 181, 181, red);


    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {





            //DRAW FLOOR TILE
            if (map[mem_Address] == 0)
            {
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, map[3]);
            }

            //DRAW OBJECT TILE
            if (map[mem_Address] > 16)
            {
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, map[mem_Address]);
                //  printNumberInt(map[mem_Address],x + xpos, y + ypos, red, black);
            }

            //DRAW TEXTURE TILE
            if (map[mem_Address] <= 16 && map[mem_Address] > 0)
            {
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, map[mem_Address]);
                if (tile_Size > 16) draw_2D_Tile(x + xpos, y + ypos, map[mem_Address], tex);
            }

            //HIGHLIGHT CURRENT CAMERA CELL 
            if (cell_X == x && cell_Y == y)
            {
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, random(1, 64));

                *posX = x + xpos + (tile_Size / 2);
                *posY = y + ypos + (tile_Size / 2);
                //return *posX_Center  *posY_Center 
            }

            //return ray hit cells

            if (x == cell_HitX_Left && cell_HitY_Left == y)
            {
                *hitX_Left = x + xpos + (tile_Size / 2);//center of cell in screen space
                *hitY_Left = y + ypos + (tile_Size / 2);
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, random(1, 64));
            }



            if (x == cell_HitX_Center && cell_HitY_Center == y)
            {
                *hitX_Center = x + xpos + (tile_Size / 2);//center of cell in screen space
                *hitY_Center = y + ypos + (tile_Size / 2);
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, random(1, 64));
            }



            if (x == cell_HitX_Right && cell_HitY_Right == y)
            {
                *hitX_Right = x + xpos + (tile_Size / 2);//center of cell in screen space
                *hitY_Right = y + ypos + (tile_Size / 2);
                drawSolidRectangle(x + xpos, y + ypos, tile_Size, tile_Size, random(1, 64));
            }



            xpos += tile_Size;
            mem_Address++;
        }
        xpos = xStart;
        ypos += tile_Size;
    }
}

void GPGUI::draw_2D_Tile(int xStart, int yStart, int tile_Index, byte tex_Ram[4096])
{
    static int mem_Address = 0;
    static byte tex_Width = 16;
    switch (tile_Index)
    {
    case 1:  mem_Address = 0;       break;
    case 2:  mem_Address = 256 * 1;   break;
    case 3:  mem_Address = 256 * 2;   break;
    case 4:  mem_Address = 256 * 3;   break;
    case 5:  mem_Address = 256 * 4;   break;
    case 6:  mem_Address = 256 * 5;   break;
    case 7:  mem_Address = 256 * 6;   break;
    case 8:  mem_Address = 256 * 7;   break;
    case 9:  mem_Address = 256 * 8;   break;
    case 10: mem_Address = 256 * 9;   break;
    case 11: mem_Address = 256 * 10;  break;
    case 12: mem_Address = 256 * 11;  break;
    case 13: mem_Address = 256 * 12;  break;
    case 14: mem_Address = 256 * 13;  break;
    case 15: mem_Address = 256 * 14;  break;
    case 16: mem_Address = 256 * 15;  break;
    }


    for (int y = 0; y < tex_Width; y++)
    {
        for (int x = 0; x < tex_Width; x++)
        {
            setPixel(x + xStart, y + yStart, tex_Ram[mem_Address]);
            mem_Address++;
        }
    }
}

void GPGUI::raycast_Map_Preview(byte map_Ram[4096], byte tex_Ram[4096])
{
    static int ray_Hit_Cell_X_Left = 0;
    static int ray_Hit_Cell_Y_Left = 0;
    static int ray_Hit_Cell_X_Center = 0;
    static int ray_Hit_Cell_Y_Center = 0;
    static int ray_Hit_Cell_X_Right = 0;
    static int ray_Hit_Cell_Y_Right = 0;
    static int target_Cell_Data = 0;
    static int hitx_Left = 0;
    static int hity_Left = 0;
    static int hitx_Center = 0;
    static int hity_Center = 0;
    static int hitx_Right = 0;
    static int hity_Right = 0;
    static int posX = 0;
    static int posY = 0;
    static int camera_Map_Pos_Y = map_Ram[4];
    static int camera_Map_Pos_X = map_Ram[5];
    static int view_Port_Start_X = 183;
    static int view_Port_Start_Y = 1;
    static int view_Port_Width = 136;
    static int view_Port_Height = 182;
    static int camera_Clip_Dist = 15;
    static bool turn_Left = true;
    static bool turn_Right = false;
    static bool move_Fwd = false;
    static bool move_Back = false;
    static bool clipping_Enable = false;
    static bool camera_Up = false;
    static bool camera_Down = false;
    static float move_Speed = 0.05;
    static float turn_Speed = 1.5;
    unsigned long current_Time = millis();
    static int camera_Angle = 0;
    static bool full_Screen = false;
    static bool crossHair_Enable = false;
    ////////////////////////////////////////////////////////////////////////////////////////////////////


        //RUN ERROR CHECK ON RAM
    ram_Data_Limit_Clip(map_Ram);
    //

    if (full_Screen)
    {
        view_Port_Start_X = 0;
        view_Port_Start_Y = 0;
        view_Port_Width = 320;
        view_Port_Height = 240;



    }
    else
    {
        view_Port_Start_X = 183;
        view_Port_Start_Y = 1;
        view_Port_Width = 136;
        view_Port_Height = 182;



    }


    //READ PS2 KEYBOAD
    switch (read_Keyboard_Port())
    {
    case UP_KEY_MAKE_CODE:           move_Fwd = true;   move_Back = false;   break;
    case UP_KEY_BREAK_CODE:          move_Fwd = false;                       break;
    case DOWN_KEY_MAKE_CODE:         move_Back = true;    move_Fwd = false;   break;
    case DOWN_KEY_BREAK_CODE:        move_Back = false;                       break;
    case LEFT_KEY_MAKE_CODE:         turn_Left = true;  turn_Right = false;   break;
    case LEFT_KEY_BREAK_CODE:        turn_Left = false;                       break;
    case RIGHT_KEY_MAKE_CODE:        turn_Right = true;   turn_Left = false;   break;
    case RIGHT_KEY_BREAK_CODE:       turn_Right = false;                       break;
    case Page_Up_Key_Make_Code:      camera_Up = true; camera_Down = false;   break;
    case Page_Up_Key_Break_Code:     camera_Up = false;                       break;
    case Page_Down_Key_Make_Code:    camera_Down = true;   camera_Up = false;   break;
    case Page_Down_Key_Break_Code:   camera_Down = false;                       break;
    case F1_KEY_MAKE_CODE: full_Screen = true; break;
    case F2_KEY_MAKE_CODE: full_Screen = false; break;
    case F3_KEY_MAKE_CODE: crossHair_Enable = true; break;
    case F4_KEY_MAKE_CODE: crossHair_Enable = false; break;
    }


    if (!full_Screen)
    {
        drawBackgroundColor(16);
        draw_Tile_Map // add error check for xstart < 1
        (
            2,                     //X START
            2,                     //Y START
            map_Ram,               //TILE MAP PALET
            tex_Ram,               //TEXTURE PALET
            camera_Map_Pos_X,      //CURRENT CAMERA MAP POS X
            camera_Map_Pos_Y,      //CURRENT CAMERA MAP POS Y
            ray_Hit_Cell_X_Left,   //X END MAP CELL OF RAY, X 0
            ray_Hit_Cell_Y_Left,   //Y END MAP CELL OF RAY, X 0
            ray_Hit_Cell_X_Center, //X END MAP CELL OF RAY, X SCREEN WIDTH/2
            ray_Hit_Cell_Y_Center, //Y END MAP CELL OF RAY, X SCREEN WIDTH/2
            ray_Hit_Cell_X_Right,  //X END MAP CELL OF RAY, X XSCREEN WIDTH
            ray_Hit_Cell_Y_Right,  //Y END MAP CELL OF RAY, X XSCREEN WIDTH
            &posX,                 //X CAMERA PIXEL
            &posY,                 //Y CAMERA PIXEL
            &hitx_Left,            //X END PIXEL OF RAY,X 0
            &hity_Left,            //Y END POINT OF RAY, X0
            &hitx_Center,          //X END POINT OF RAY,X SCREEN WIDTH/2
            &hity_Center,          //Y END POINT OF RAY, X SCREEN WIDTH/2
            &hitx_Right,           //X END POINT OF RAY, XSCREEN WIDTH
            &hity_Right            //Y END POINT OF RAY, XSCREEN WIDTH

        );
    }


    raycast_Map_From_Ram
    (
        map_Ram,                   //MAP DATA BUFFER (BYTE BUFF[4096])
        tex_Ram,                   //TEXTURE BUFFER (BYTE BUFF[4097])
        view_Port_Start_X,         //CAMERA VIEW PORT START X
        view_Port_Start_Y,         //CAMERA VIEW PORT START y
        view_Port_Width,           //CAMERA VIEW PORT WIDTH
        view_Port_Height,          //CAMERA VIEW PORT HEIGHT
        turn_Left,                 //CAMERA TURN LEFT ENABLE
        turn_Right,                //CAMERA TURN RIGHT ENABLE
        move_Fwd,                  //CAMERA MOVE FWD ENABLE
        move_Back,                 //CAMERA MOVE BACK ENABLE
        clipping_Enable,           //CAMERA DISTANCE CLIPING ENABLE
        camera_Clip_Dist,          //CAMERA CLIP DISTANCE
        camera_Up,                 //CAMERA UP ENABLE
        camera_Down,               //CAMERA DOWN ENABLE
        move_Speed,                //CAMERA FOWRD/BACK SPEED
        turn_Speed,                //CAMERA ROTATION SPEED
        &camera_Map_Pos_X,         //ADDRESS OF CAMERA MAP POS X
        &camera_Map_Pos_Y,         //ADDRESS OF CAMERA MAP POS y    
        &camera_Angle,             //WORLD MAP ABSOLUTE ANGLE
        &ray_Hit_Cell_X_Left,      //END OF LEFT RAY X
        &ray_Hit_Cell_Y_Left,      //END OF LEFT RAY Y
        &ray_Hit_Cell_X_Center,    //END OF CENTER RAY X
        &ray_Hit_Cell_Y_Center,    //END OF CENTER RAY Y
        &ray_Hit_Cell_X_Right,     //END OF RIGHT RAY X
        &ray_Hit_Cell_Y_Right,     //END OF RIGHT RAY Y
        &target_Cell_Data,         //DATA OF MEM ADDRESS AT CENTER RAY HIT POINT
        crossHair_Enable,          //CAMERA CROSSHAIR ENABLE
        false                      //DEBUG DATA ENABLE
    );
    //DRAW COMPASS GRAPH
    camera_Compass(267, 1, camera_Map_Pos_X, camera_Map_Pos_Y, camera_Angle, black);

    if (!full_Screen)
    {
        //DRAW FOV TRIANGLE
        drawLine(posX, posY, hitx_Left, hity_Left, red);
        drawLine(posX, posY, hitx_Center, hity_Center, green);
        drawLine(posX, posY, hitx_Right, hity_Right, red);


        //DRAW TEXTURES
        int index = 1;
        for (int x = 20; x < 308; x += 18)
        {
            draw_2D_Tile(x, 190, index, tex_Ram);
            index++;
        }

        //HIGHLIGHT TARGET CELL
        printNumberInt(target_Cell_Data, 1, 190, white, black);
        switch (target_Cell_Data)
        {
            //WALL TEXTURES
        case 1:  drawHollowRectangle(19, 189, 18, 18, random(1, 63)); break;
        case 2:  drawHollowRectangle((18 * 2) + 1, 189, 18, 18, random(1, 63)); break;
        case 3:  drawHollowRectangle((18 * 3) + 1, 189, 18, 18, random(1, 63)); break;
        case 4:  drawHollowRectangle((18 * 4) + 1, 189, 18, 18, random(1, 63)); break;
        case 5:  drawHollowRectangle((18 * 5) + 1, 189, 18, 18, random(1, 63)); break;
        case 6:  drawHollowRectangle((18 * 6) + 1, 189, 18, 18, random(1, 63)); break;
        case 7:  drawHollowRectangle((18 * 7) + 1, 189, 18, 18, random(1, 63)); break;
        case 8:  drawHollowRectangle((18 * 8) + 1, 189, 18, 18, random(1, 63)); break;
        case 9:  drawHollowRectangle((18 * 9) + 1, 189, 18, 18, random(1, 63)); break;
        case 10: drawHollowRectangle((18 * 10) + 1, 189, 18, 18, random(1, 63)); break;
        case 11: drawHollowRectangle((18 * 11) + 1, 189, 18, 18, random(1, 63)); break;
        case 12: drawHollowRectangle((18 * 12) + 1, 189, 18, 18, random(1, 63)); break;
        case 13: drawHollowRectangle((18 * 13) + 1, 189, 18, 18, random(1, 63)); break;
        case 14: drawHollowRectangle((18 * 14) + 1, 189, 18, 18, random(1, 63)); break;
        case 15: drawHollowRectangle((18 * 15) + 1, 189, 18, 18, random(1, 63)); break;
        case 16: drawHollowRectangle((18 * 16) + 1, 189, 18, 18, random(1, 63)); break;


            //SPRITE TEXTURES
        case 17: drawHollowRectangle(19, 209, 18, 18, random(1, 63)); break;
        case 18: drawHollowRectangle((18 * 2) + 1, 209, 18, 18, random(1, 63)); break;
        case 19: drawHollowRectangle((18 * 3) + 1, 209, 18, 18, random(1, 63)); break;
        case 20: drawHollowRectangle((18 * 4) + 1, 209, 18, 18, random(1, 63)); break;
        case 21: drawHollowRectangle((18 * 5) + 1, 209, 18, 18, random(1, 63)); break;
        case 22: drawHollowRectangle((18 * 6) + 1, 209, 18, 18, random(1, 63)); break;
        case 23: drawHollowRectangle((18 * 7) + 1, 209, 18, 18, random(1, 63)); break;
        case 24: drawHollowRectangle((18 * 8) + 1, 209, 18, 18, random(1, 63)); break;
        case 25: drawHollowRectangle((18 * 9) + 1, 209, 18, 18, random(1, 63)); break;
        case 26: drawHollowRectangle((18 * 10) + 1, 209, 18, 18, random(1, 63)); break;
        case 28: drawHollowRectangle((18 * 11) + 1, 209, 18, 18, random(1, 63)); break;
        case 29: drawHollowRectangle((18 * 12) + 1, 209, 18, 18, random(1, 63)); break;
        case 30: drawHollowRectangle((18 * 13) + 1, 209, 18, 18, random(1, 63)); break;
        case 31: drawHollowRectangle((18 * 14) + 1, 209, 18, 18, random(1, 63)); break;
        case 32: drawHollowRectangle((18 * 15) + 1, 209, 18, 18, random(1, 63)); break;
        case 33: drawHollowRectangle((18 * 16) + 1, 209, 18, 18, random(1, 63)); break;
        };

        //DRAW BLACK BAR BEHIND DATA READ OUT
        drawSolidRectangle(0, 230, 320, 10, black);


        //PRINT RAM DATA
        print("SIZE X", 1, 231, red, black); printNumberInt(map_Ram[0], 40, 231, green, black);
        print("Y", 60, 231, red, black);     printNumberInt(map_Ram[1], 70, 231, green, black);
    }

    // FPS_Counter(current_Time, black);
    refreshScreen();
}

void GPGUI::ram_Data_Limit_Clip(byte map_Ram[4096])
{
    byte map_Width_Min = 4;
    byte map_Width_Max = 60;
    byte map_Height_Min = 4;
    byte map_Height_Max = 60;


    if (map_Ram[0] < map_Width_Min)  map_Ram[0] = map_Width_Min;
    if (map_Ram[0] > map_Width_Max)  map_Ram[0] = map_Width_Max;
    //
    if (map_Ram[1] < map_Height_Min) map_Ram[1] = map_Height_Min;
    if (map_Ram[1] > map_Height_Max) map_Ram[1] = map_Height_Max;
    //
    if (map_Ram[2] < 0)              map_Ram[2] = 0;   //roof color 0 - 63
    if (map_Ram[2] > 63)             map_Ram[2] = 63;  //roof color 0 - 63
    //
    if (map_Ram[3] < 0)              map_Ram[3] = 0;   //floor color 0 - 63
    if (map_Ram[3] > 63)             map_Ram[3] = 63;  //floor color 0 - 63
    //
    //if (map_Ram[4] < 1)              map_Ram[4] = 2;//camera pos x start
    //if (map_Ram[4] > map_Ram[0])     map_Ram[4] = map_Ram[0] - 2;//camera pos x start
    ////
    //if (map_Ram[5] < 1)              map_Ram[5] = 2;//camera pos y start
    //if (map_Ram[5] > map_Ram[1])     map_Ram[5] = map_Ram[1] - 2;//camera pos y start
    //
    map_Ram[6] = 0;//UNDEFINED
    map_Ram[7] = 0;//UNDEFINED
    map_Ram[8] = 0;//UNDEFINED
    map_Ram[9] = 0;//UNDEFINED


    //set roof and floor to default if they match
    if (map_Ram[2] == map_Ram[3]) { map_Ram[2] = black; map_Ram[3] = 21; }

}

void GPGUI::camera_Compass(int xStart, int yStart, int xPos, int yPos, int angle, byte back_Color)
{
    static int bin;
    static int end1_x;
    static int end1_y;
    static int end2_x;
    static int end2_y;
    static int end3_x;
    static int end3_y;
    static int grid_StartX = xStart + 1;
    static int grid_StartY = yStart + 1;
    static int color = green;

    //DRAW BACKGROUND
    drawSolidRectangle(xStart, yStart, 52, 71, back_Color);

    //DRAW GRID
    drawHollowRectangle(grid_StartX, grid_StartY, 50, 50, green);

    //DRAW VERTICAL GRID LINES
    for (int x = grid_StartX + 5; x < grid_StartX + 50; x += 5)
    {
        if (x == grid_StartX + (5 * 5)) color = yellow;//CHANGE CENTER LINE TO YELLOW
        else color = green;
        drawLine(x, grid_StartY, x, grid_StartY + 50, color);
    }

    //DRAW HORAZONTAL GRID LINES
    for (int y = grid_StartY + 5; y < grid_StartY + 50; y += 5)
    {
        if (y == grid_StartY + (5 * 5)) color = yellow;//CHANGE CENTER LINR TO YELLOW
        else color = green;
        drawLine(grid_StartX, y, grid_StartX + 50, y, color);
    }

    //DRAW FOV ICON
    draw_Vector(xStart + 26, yStart + 26, 23, (float)angle - 20, 0, &end1_x, &end1_y, random(1, 63));
    draw_Vector(xStart + 26, yStart + 26, 23, (float)angle + 20, 0, &end3_x, &end3_y, random(1, 63));

    //DISPLAY TEXT DATA
    print("X", xStart + 1, yStart + 53, red, back_Color);  printNumberInt(xPos, xStart + 10, yStart + 53, yellow, back_Color);
    print("Y", xStart + 31, yStart + 53, red, back_Color); printNumberInt(yPos, xStart + 40, yStart + 53, yellow, back_Color);
    print("AN", xStart + 1, yStart + 63, red, back_Color); printNumberInt(angle, xStart + 15, yStart + 63, yellow, back_Color);
}





































































//#############################################################################################################
//#//////////////////////////////////////////EXAMPLE PROGRAMS/////////////////////////////////////////////////#
//#############################################################################################################


void GPGUI::QUAD_DEBUG_PROGRAM()

{
    static int active_Point = 0;
    static float x1 = 20;
    static float y1 = 20;

    static float x2 = 20;
    static float y2 = 120;

    static float x3 = 120;
    static float y3 = 120;

    static float x4 = 20;
    static float y4 = 120;


    float point_Move_Speed = 0.5;

    bool point_Label_En = true;


    while (1)
    {

        switch (read_Keyboard_Port())
        {
        case LEFT_KEY_MAKE_CODE:

            switch (active_Point)
            {
            case 0:
                x1 -= point_Move_Speed;
                x2 -= point_Move_Speed;
                x3 -= point_Move_Speed;
                x4 -= point_Move_Speed;

                break;
            case 1:  x1 -= point_Move_Speed; break;
            case 2:  x2 -= point_Move_Speed; break;
            case 3:  x3 -= point_Move_Speed; break;
            case 4:  x4 -= point_Move_Speed; break;
            }

            break;


        case RIGHT_KEY_MAKE_CODE:

            switch (active_Point)
            {
            case 0:
                x1 += point_Move_Speed;
                x2 += point_Move_Speed;
                x3 += point_Move_Speed;
                x4 += point_Move_Speed;

                break;
            case 1:  x1 += point_Move_Speed; break;
            case 2:  x2 += point_Move_Speed; break;
            case 3:  x3 += point_Move_Speed; break;
            case 4:  x4 += point_Move_Speed; break;
            }

            break;



        case UP_KEY_MAKE_CODE:

            switch (active_Point)
            {
            case 0:
                y1 -= point_Move_Speed;
                y2 -= point_Move_Speed;
                y3 -= point_Move_Speed;
                y4 -= point_Move_Speed;

                break;

            case 1:  y1 -= point_Move_Speed; break;
            case 2:  y2 -= point_Move_Speed; break;
            case 3:  y3 -= point_Move_Speed; break;
            case 4:  y4 -= point_Move_Speed; break;
            }

            break;


        case DOWN_KEY_MAKE_CODE:

            switch (active_Point)
            {
            case 0:
                y1 += point_Move_Speed;
                y2 += point_Move_Speed;
                y3 += point_Move_Speed;
                y4 += point_Move_Speed;

                break;



            case 1:  y1 += point_Move_Speed;  break;
            case 2:  y2 += point_Move_Speed;  break;
            case 3:  y3 += point_Move_Speed;  break;
            case 4:  y4 += point_Move_Speed;  break;
            }

            break;

        case Page_Up_Key_Make_Code:   active_Point++; if (active_Point == 5) active_Point = 0;   beep(500, 200); break;

        case 273:
            active_Point = 0;
            x1 = 20;
            y1 = 20;

            x2 = 20;
            y2 = 120;

            x3 = 120;
            y3 = 120;

            x4 = 20;
            y4 = 120;
            break;

        case 44:




            break;

        };








        drawBackgroundColor(16);



        draw_Quad

        (
            x1, //p1
            y1,

            x2,//p2
            y2,

            x3,//p3
            y3,


            x4,//p4
            y4,
            1,
            yellow
        );



        switch (active_Point)
        {

        case 0:
            setPixel(x1, y1, random(1, 64));
            drawHollowRectangle(x1 - 3, y1 - 3, 6, 6, random(1, 63));
            if (point_Label_En)  printNumberInt(1, x1 + 4, y1, green, black);

            setPixel(x2, y2, random(1, 64));
            drawHollowRectangle(x2 - 3, y2 - 3, 6, 6, random(1, 63));
            if (point_Label_En) printNumberInt(2, x2 + 4, y2, green, black);

            setPixel(x3, y3, random(1, 64));
            drawHollowRectangle(x3 - 3, y3 - 3, 6, 6, random(1, 63));
            if (point_Label_En) printNumberInt(3, x3 + 4, y3, green, black);


            setPixel(x4, y4, random(1, 64));
            drawHollowRectangle(x4 - 3, y4 - 3, 6, 6, random(1, 63));
            if (point_Label_En) printNumberInt(4, x4 + 4, y4, green, black);

            break;




        case 1:
            setPixel(x1, y1, random(1, 64));
            drawHollowRectangle(x1 - 3, y1 - 3, 6, 6, random(1, 63));
            if (point_Label_En) printNumberInt(active_Point, x1 + 4, y1, green, black);
            printNumberDouble(x1, 2, 2, red, black);
            printNumberDouble(y1, 2, 12, red, black);
            break;

        case 2:

            setPixel(x2, y2, random(1, 64));
            drawHollowRectangle(x2 - 3, y2 - 3, 6, 6, random(1, 63));
            if (point_Label_En)  printNumberInt(active_Point, x2 + 4, y2, green, black);
            printNumberDouble(x2, 2, 2, red, black);
            printNumberDouble(y2, 2, 12, red, black);
            break;



        case 3:

            setPixel(x3, y3, random(1, 64));
            drawHollowRectangle(x3 - 3, y3 - 3, 6, 6, random(1, 63));
            if (point_Label_En)  printNumberInt(active_Point, x3 + 4, y3, green, black);
            printNumberDouble(x3, 2, 2, red, black);
            printNumberDouble(y3, 2, 12, red, black);
            break;



        case 4:

            setPixel(x4, y4, random(1, 64));
            drawHollowRectangle(x4 - 3, y4 - 3, 6, 6, random(1, 63));
            if (point_Label_En)  printNumberInt(active_Point, x4 + 4, y4, green, black);
            printNumberDouble(x4, 2, 2, red, black);
            printNumberDouble(y4, 2, 12, red, black);
            break;
        };










        refreshScreen();
    }
}

void GPGUI::texture_Scan_Line_Maping_Debug_Program()
{
    //SCAN LINE
    int x1 = 50;
    int y1 = 50;
    int x2 = 270;
    int y2 = 50;


    //int w
    int xi = 160; //SCREEN SPACE
    int yi = 50;

    float xn = 0; //HOMOGENEOUS SCREEN SPACE 0 - 1

    int tex_Width = 32;
    int texX = 0;
    byte tex[tex_Width];
    bool bit = 1;
    for (int x = 0; x < tex_Width; x++)
    {
        if (bit)
        {
            tex[x] = white; bit = 0;
        }
        else
        {
            tex[x] = black; bit = 1;
        }
    }




    while (1)
    {
        switch (read_Keyboard_Port())
        {
        case LEFT_KEY_MAKE_CODE:
            if (xi > x1) xi--;

            break;


        case RIGHT_KEY_MAKE_CODE:
            if (xi < x2) xi++;
            break;

        case DOWN_KEY_MAKE_CODE:
            if (x2 - x1 > tex_Width)
            {

                x1++;
                x2--;
                xi = x1;
            }
            break;
        case UP_KEY_MAKE_CODE:
            if (x1 > 10 && x2 < 310)
            {

                x1--;
                x2++;
                xi = x1;
            }
            break;

        };







        drawBackgroundColor(16);




        //DRAW SCAN LINE         
        for (int i = x1; i < x2; i++)
        {
            m.normalize(i, x1, x2, &xn);
            m.interp(0, tex_Width, xn, &texX);//get texture x pos
            setPixel(i, yi, tex[texX]);//sample texture for color
        }
        print("xi", 130, y1 - 10, green, black);  printNumberInt(xi, 160, y1 - 10, red, black);
        //




        //DRAW TEXTURE LINE
        for (int i = 0; i < tex_Width; i++) setPixel(i + ((320 - tex_Width) / 2), 120, tex[i]);
        //TEX SAMPLE POINT
        m.normalize(xi, x1, x2, &xn);
        m.interp(0, tex_Width, xn, &texX);//get texture x pos
        drawLine(xi, yi, texX + ((320 - tex_Width) / 2), 120, red);
        print("texX", 130, 130, green, black);  printNumberInt(texX, 160, 130, red, black);
        //



        refreshScreen();
    }
}

void GPGUI::screen_Space_Texture_Mapping_Program()
{
    byte tex[32][32];
    bool bit = true;
    int texWidth = 32;
    int texHeight = 32;
    int tex_Tile_Start_X = 150;
    int tex_Tile_Start_Y = 50;
    int Xa = 10;
    int Xb = 110;
    int ya = 10;
    int yb = 110;
    int yi = 60;
    float ty = 0.0f;
    float xpos = 0;
    float xposEnd = 1;


    for (int y = 0; y < texHeight; y++)
    {
        for (int x = 0; x < texWidth; x++)
        {
            if (bit) {
                tex[y][x] = black;
                bit = false;
            }
            else {
                tex[y][x] = white;
                bit = true;
            }
        }
        if (bit) bit = 0;
        else bit = 1;
    }
    tex[0][0] = green;






    while (1)
    {
        switch (read_Keyboard_Port())
        {
        case F9_KEY_MAKE_CODE:
            if (xpos > 0) xpos -= .01f;
            beep(500, 10);
            break;


        case F10_KEY_MAKE_CODE:
            if (xpos < 1) xpos += .01f;
            beep(500, 10);
            break;

        case F11_KEY_MAKE_CODE:
            if (xposEnd > 0) xposEnd -= .01f;
            beep(500, 10);
            break;

        case F12_KEY_MAKE_CODE:
            if (xposEnd < 1) xposEnd += .01f;
            beep(500, 10);
            break;

        case UP_KEY_MAKE_CODE:
            if (yi > ya + 1) yi--;
            beep(500, 10);
            break;

        case DOWN_KEY_MAKE_CODE:
            if (yi < yb - 1) yi++;
            beep(500, 10);
            break;

        };



        drawBackgroundColor(16);


        //HIGHLIGHT SCREEN SPACE BEING MAPPED
        drawHollowRectangle(Xa, ya, (Xb - Xa), (yb - ya), random(1, 63));

        //m.interpOLATE TEXTURE AND SCREEN SPACE TO DRAW SCANLINE
        m.normalize(yi, ya, yb, &ty);//m.normalize y height
        map_Scanline_From_Texture(Xa, Xb, yi, xpos, xposEnd, ty, tex);




        //DRAW TEXTURE 1:1

        for (int y = 0; y < texHeight; y++)
        {
            for (int x = 0; x < texWidth; x++)
            {
                setPixel(x + tex_Tile_Start_X, y + tex_Tile_Start_Y, tex[y][x]);
            }
        }


        //DRAW SCANLINE REIGON ON TEXTURE 1:1

        int texScreenX;
        m.interp(tex_Tile_Start_X, tex_Tile_Start_X + (texWidth - 1), xpos, &texScreenX);

        int texScreenXend;
        m.interp(tex_Tile_Start_X, tex_Tile_Start_X + (texWidth - 1), xposEnd, &texScreenXend);


        int texScreenY;
        m.interp(tex_Tile_Start_Y, tex_Tile_Start_Y + (texHeight), ty, &texScreenY);

        drawLine(texScreenX, texScreenY, texScreenXend, texScreenY, random(1, 63));
        setPixel(texScreenX, texScreenY, red);
        setPixel(texScreenXend, texScreenY, red);



        refreshScreen();

    }
}

void GPGUI::DEFAULT_PROGRAM()
{

    int zNear_Offset = 10;
    int zFar_Offset = 10;
    int zNear_Plane_Length = 100;
    int zFar_Plane_Length = 50;

    int yStart = 10;
    int xStart = 10;
    int screen_Width = 150;
    int screen_Height = 150;

    byte tex[32][32];

    // tex[random(1, 63)][random(1, 63)];

    while (1)
    {
        tex[random(0, 32)][random(0, 32)] = random(1, 63);
        switch (read_Keyboard_Port())
        {
        case LEFT_KEY_MAKE_CODE:
            break;

        case RIGHT_KEY_MAKE_CODE:
            break;

        case DOWN_KEY_MAKE_CODE:
            break;

        case UP_KEY_MAKE_CODE:
            break;

        case F1_KEY_MAKE_CODE:
            zNear_Offset--;
            beep(500, 10);
            break;

        case F2_KEY_MAKE_CODE:
            zNear_Offset++;
            beep(500, 10);
            break;

        case F3_KEY_MAKE_CODE:
            zFar_Offset--;
            beep(500, 10);
            break;

        case F4_KEY_MAKE_CODE:
            zFar_Offset++;
            beep(500, 10);
            break;

        };




        float vertex[3] =
        {
            2.0f,
            0.0f,
            0.0f
        };




        // m.translate_Matrix(0.5f, 0.5f, 0.5f, vertex);
         //m.scaling_Matrix(0.5f, vertex);
          // m.x_Rotation_Matrix(3.14, vertex);
        // m.y_Rotation_Matrix(.1, vertex);
        // m.z_Rotation_Matrix(3.144, vertex);

        delay(500);

        for (int i = 0; i < 3; i++)
        {
            Serial.println(vertex[i]);
        }
        Serial.println();
        drawBackgroundColor(white);


        // draw_Perspective_Plane(xStart,yStart,screen_Width,screen_Height,zNear_Offset,zFar_Offset,50,100,tex);

        



        refreshScreen();
    }
}

void GPGUI::XY_Plane_Debug_Program()
{

    //vector<vertex3D> world_Vertex_Buffer;


    //
    ////write objects to vector
    //for (int i = 0; i < 10; i++)
    //{   
    //    vertex3D vertex_n;//object template

    //    vertex_n.x = i;  //modify object data
    //    vertex_n.y = 2;
    //    if (i == 500) vertex_n.z = 55;
    //    else vertex_n.z = 0;

    //    world_Vertex_Buffer.push_back(vertex_n);//push object into vector buffer   
    //}


    ////read objects from vector
    //for (int i = 0; i < world_Vertex_Buffer.size(); i++)
    //{
    //    vertex3D  temp = world_Vertex_Buffer[i];//object template copied from vector buffer
    //    float X = temp.x;
    //    float Y = temp.y;
    //    float Z = temp.z;

    //    Serial.println(X);//use data
    //    Serial.println(Y);
    //    Serial.println(Z);
    //    Serial.println();
    //}






    //float cameraX = 0;
    //float cameraY = 0;
    //float cameraZ = 20;

    ////CREATE COPY OF MESH
    //point_Mesh_3D p;
    ////

    ////X Y Z COLOR
    //float world_Space_Verticies_Buffer[20][4];


    ////COPY MESH STRUCTURE TO WORLD SPACE VERTEX BUFFER
    //for (int i = 0; i < 20; i++)
    //{
    //    world_Space_Verticies_Buffer[i][0] = p.vertex_Buffer[i][0];
    //    world_Space_Verticies_Buffer[i][1] = p.vertex_Buffer[i][1];
    //    world_Space_Verticies_Buffer[i][2] = p.vertex_Buffer[i][2];
    //    world_Space_Verticies_Buffer[i][3] = p.vertex_Buffer[i][3];
    //}
    ////

    //while (1)
    //{

    //    switch (read_Keyboard_Port())
    //    {
    //    case LEFT_KEY_MAKE_CODE:
    //        cameraX -= 0.5f;
    //        break;

    //    case RIGHT_KEY_MAKE_CODE:
    //        cameraX += 0.5f;
    //        break;

    //    case UP_KEY_MAKE_CODE:
    //        cameraY -= 0.5f;
    //        break;

    //    case DOWN_KEY_MAKE_CODE:
    //        cameraY += 0.5f;
    //        break;

    //    case HOME_MAKE_CODE:
    //        cameraX = 0;
    //        cameraY = 0;
    //        cameraZ = 20;
    //        break;

    //    case Page_Down_Key_Make_Code:// zoom out
    //        if (cameraZ < 500)
    //        {
    //            cameraZ += .5;
    //        }
    //        break;

    //    case Page_Up_Key_Make_Code://zoom in
    //        if (cameraZ > 1)
    //        {
    //            cameraZ -= .5;
    //        }
    //        break;
    //    };




    //    //MAKE CHANGES TO VERTICIES IN MODEL SPACE

    //    float vertex[3];
    //    //ROTATE EVERY WORLD VERTEX BY ANGLE  delta
    //    for (int i = 0; i < 20; i++)
    //    {
    //        vertex[0] = world_Space_Verticies_Buffer[i][0]; //copy address i
    //        vertex[1] = world_Space_Verticies_Buffer[i][1];
    //        vertex[2] = world_Space_Verticies_Buffer[i][2];
    //        m.z_Rotation_Matrix(2, vertex); //rotate copy
    //        world_Space_Verticies_Buffer[i][0] = vertex[0]; //write over vertex i with rotated in
    //        world_Space_Verticies_Buffer[i][1] = vertex[1];
    //        world_Space_Verticies_Buffer[i][2] = vertex[2];
    //        //repeat for next vertex i
    //    }
    //    // TRANSLATE VERTICES TO SCREEN SPACE

    //    drawBackgroundColor(21);
    //    translate_XY_Veiw_Plane_To_Screen_Plane(0, 0, 240, 240, cameraX, cameraY, cameraZ, black, world_Space_Verticies_Buffer);
    //    print("camera X", 10, 10, green, black);      printNumberInt(cameraX, 70, 10, red, black);
    //    print("camera Y", 10, 20, green, black);     printNumberInt(cameraY, 70, 20, red, black);
    //    print("camera Z", 10, 30, green, black);     printNumberInt(cameraZ, 70, 30, red, black);
    //    refreshScreen();
  //  }
}

void GPGUI::Vertex_Pipeline_Program(bool run)
{
    //CREATE BUFFER TO HOLD ALL WORLD VERTICIES
    vector<vertex3D> world_Space_Vertex_Buffer;
   

    int Z_Cliped = 0;
    int vertex_Counter = 0;
    int bin;
    int av_Count = 0;
    float av_Fps_Count = 0;
    float av_Fps = 0;
    float z = 100;
    int back_Color = 0;
    unsigned long frame_Time;
    float camera_X = 0;
    float camera_Y = 0;
    bool x_Axis_Rotate = false;
    bool y_Axis_Rotate = false;
    bool z_Axis_Rotate = false;

    if (run)
    {   
        write_Origin_Cross_Mesh_To_Vector(world_Space_Vertex_Buffer, 20, false, &bin);

 
        while (1)
        {
            unsigned long start_Time = millis();
            drawBackgroundColor(back_Color);//CLEAR VGA BACK BUFFER
         
         
            //PASS VERTEX BUFFER TO RENDER PIPELINE
            vertex_Buff_2D_Parallel_Projection_Pipeline
            (
                40,
                0,
                240,
                camera_X,
                camera_Y,
                z,
                black,
                &vertex_Counter,
                &Z_Cliped,
                world_Space_Vertex_Buffer
            );

          

            //CALC FPS###########################################
            unsigned long end_Time = millis();
            frame_Time = end_Time - start_Time;
            float fps = 1000 / frame_Time;
            av_Fps_Count += frame_Time;
            av_Count++;
                if(av_Count > 50) 
                { 
                    av_Fps = 1000 / (av_Fps_Count / 50);
                    av_Fps_Count = 0; 
                    av_Count = 0;
                }
            //###################################################
 
                float zoom_Speed   = 0.01  * frame_Time;
                float rotate_Speed = 0.05  * frame_Time;
                float pan_Speed    = 0.01  * frame_Time;
                switch (read_Keyboard_Port())
                {
                case Page_Up_Key_Make_Code:    if (z > 2) z -= zoom_Speed; break;
                case Page_Down_Key_Make_Code:             z += zoom_Speed; break;
                case F11_KEY_MAKE_CODE:   if (back_Color < 63) back_Color++; beep(1000, 350); break;
                case F12_KEY_MAKE_CODE:   if (back_Color > 0)  back_Color--; beep(800, 350); break;
                case LEFT_KEY_MAKE_CODE:    rotate_Mesh(world_Space_Vertex_Buffer, 1, rotate_Speed); break;//ROTATE RIGHT
                case RIGHT_KEY_MAKE_CODE:   rotate_Mesh(world_Space_Vertex_Buffer, 2, rotate_Speed); break;                
                case UP_KEY_MAKE_CODE:      rotate_Mesh(world_Space_Vertex_Buffer, 0, rotate_Speed); break; //ROTATE FOWORD
                case DOWN_KEY_MAKE_CODE:    break;
                case 16661: camera_X += pan_Speed;   break;//SHIFT DOWN LEFT KEY MAKE     //PAN CAMERA
                case 16662: camera_X -= pan_Speed;   break;//SHIFT DOWN RIGHT KEY MAKE
                case 16663: camera_Y -= pan_Speed;    break;//SHIFT DOWN UP KEY MAKE
                case 16664: camera_Y += pan_Speed;    break;//SHIFT DOWN DOWN KEY MAKE
                }
                print("Vertex Count", 1, 1, red, black); printNumberInt(vertex_Counter, 75, 1, green, black);
                print("FPS", 100, 1, red, black); printNumberInt((int)av_Fps, 120, 1, green, black);
                world_Space_Vertex_Buffer.clear();
        }//END OF RENDER LOOP######################################################################################
    }
}

void GPGUI::color_Pellet_Program()
{




    while (1)
    {
        drawBackgroundColor(0);

        draw_Color_Map_Shaded(true);



        refreshScreen();
    }
}

void GPGUI::triangle_Cube_Program()
{
    float an = 0;
    int triangle_Count;
    vector<triangle> world_Triangle_Buffer;



    vector<triangle>                 OBJ_1_Buffer;
    // vector<triangle>                 OBJ_2_Buffer;


     //SINGLE RANDOM TRIANGLE
    triangle t;
    t.p1.x = 0;
    t.p1.y = 0;
    t.p1.z = -10;

    t.p2.x = 20;
    t.p2.y = 20;
    t.p2.z = 0;

    t.p3.x = 20;
    t.p3.y = -10;
    t.p3.z = 0;
    t.face_Color = yellow;
   // OBJ_1_Buffer.push_back(t);



    //LOAD OBJECT DATA
    translate_Cube_Vertex_Buff_To_Triangle_Buff(cube_Mesh, OBJ_1_Buffer);

    rotate_Obj_Triangle
    (
        1,
        0,
        0,
        0,
        0,
        0,
        12.0,
        0.0,
        0.0,
        OBJ_1_Buffer
    );





    //RENDER LOOP
    while (1)
    {
        //clear vga back buffer
        drawBackgroundColor(16);


        //rotate obj1 in model space
        rotate_Obj_Triangle
        (
            0,
            1,
            0,

            0,
            0,
            0,
            0.70,
            0.50,
            0.50,
            OBJ_1_Buffer
        );



        //move object from model space to world space
        move_Obj_Triangle_Buffer_To_World_Triangle_Buffer
        (
            OBJ_1_Buffer,
            world_Triangle_Buffer
        );





        //RENDER WORLD TRIANGLE BUFFER
        triangle_Buff_To_Screen
        (
            40,                          // WINDOW START X
            0,                           // WINDOW START Y
            240,                         // WINDOW SIZE
            0,                           // CAMERA X
            sin(m.deg_To_Rad(an)),       // CAMERA Y
            60,                          // CAMERA Z
            0,                           // LIGHT X
            0,                           // LIGHT Y
            0,                           // LIGHT Z
            true,                        // ENABLE BACKFACE CULLING
            true,                        // LIGHT ENABLE
            false,                        // TRIANGLE FILL ENABLE
            &triangle_Count,             // TRIANGLES DRAWN
            world_Triangle_Buffer        // WORLD SPACE TRIANGLE BUFFER
        );

        an++;
        if (an >= 360) an = 0;

        print("TRIANGLES", 1, 1, red, black);
        printNumberInt(triangle_Count, 60, 1, green, black);
        refreshScreen();
    }
}


//#############################################################################################################


