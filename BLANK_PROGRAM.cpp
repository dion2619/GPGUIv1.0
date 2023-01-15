void GPGUI::DEFAULT_PROGRAM()
{
    









    while (1)
    {
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
        };

 

        drawBackgroundColor(16);



        print("DEFAULT", 1, 1, green, black);  printNumberInt(4000, 80, 1, red, black);
        refreshScreen();
    }
}
