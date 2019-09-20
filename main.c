/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include "song.h"

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

typedef struct Alien
{
    unsigned char key;
    int x;
    int y;
    bool visible;
} Alien;

typedef enum State
{
    WELCOME_SCREEN,
    WAIT_FOR_START,
    START_COUNT_DOWN,
    COUNT_DOWN_SCREEN,
    NEXT_LEVEL_SCREEN,
    START_LEVEL,
    PLAYING_GAME,
    LOSS_SCREEN
} State;

// Important Global Variables
int currentSecond;
int sec;

// Function Prototypes
void drawWelcome();
void drawLoss();
void drawNextLevel(int);
bool drawCountdown(void);
void drawAliens(Alien*, int);

// Write  a  function  to  configure  the  4  lab  board  buttons,  S1  through  S4
void configureButtons() {
    // P7.0, P3.6, P2.2, P7.4
    // Configure P2.2
    P2SEL &= ~(BIT2); // Select pin for DI/O
    P2DIR &= ~(BIT2); // Set pin as input
    P2REN |= (BIT2); // Enable pull-up resistor

    // Configure P3.6
    P3SEL &= ~(BIT6); // Select pin for DI/O
    P3DIR &= ~(BIT6); // Set pin as input
    P3REN |= (BIT6); // Enable pull-up resistor

    // Configure P7.0 and P7.4
    P7SEL &= ~(BIT4 | BIT0); // Select pins for DI/O
    P7DIR &= ~(BIT4 | BIT0); // Set pins as input
    P7REN |= (BIT4 | BIT0); // Enable pull-up resistors
}

void configTimerA2(void)
{
    TA2CTL = TASSEL_1 + ID_0 + MC_1;
    TA2CCR0 = 164; // = ~1/200 seconds
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled

    _BIS_SR(GIE); // Enable interrupts now, so we don't have to do it in main later
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void)
{
    int realCount = 0; // Increments every 5 milliseconds
    int leap = 0; // Helps us mitigate cumulative error
    int sec = 0; // Increments every half second

    if (leap < 141)
    {
        realCount++;
        leap++;
    } else {
        realCount += 2;
        leap = 0;
    }

    if (realCount % 200 == 0)
    {
        sec++;
    }
}

// Write  a  function  that  returns  the  state  of  the  lab  board  buttons  with  1=pressed  and  0=not pressed.
int getButtonState() {
    char ret = 0x00;
    // P2.2
    if(P2IN & BIT2) ret |= BIT0; // Button 1
    // P3.6
    if(P3IN & BIT6) ret |= BIT1; // Button 2
    // P7.0
    if(P7IN & BIT0) ret |= BIT2; // Button 3
    // P7.4
    if(P7IN & BIT4) ret |= BIT3; // Button 4
    return ret;
}

void setLED(unsigned char state)
{
    unsigned char mask = 0;

    // Turn all LEDs off to start
    P6OUT &= ~(BIT4|BIT3|BIT2|BIT1);

    if (state & BIT0)
        mask |= BIT4;   // Right most LED P6.4
    if (state & BIT1)
        mask |= BIT3;   // next most right LED P.3
    if (state & BIT2)
        mask |= BIT1;   // third most left LED P6.1
    if (state & BIT3)
        mask |= BIT2;   // Left most LED on P6.2
    P6OUT |= mask;
}

// What data structure(s) will you use to store pitch, duration and the corresponding LED? What length songs will you eventually
// want to play? Given how you choose to save your notes, etc., how much memory will that require?

// Total of 3 bytes


// Declare globals here

// Main
void main(void)
{
    int i;
    unsigned char keyPressed = 0, lastKeyPressed = 0;
    unsigned long int mainCounter = 0, auxCounter = 0, auxCounter2 = 0;
    State state = PLAYING_GAME;

    int noteCounter = 0;
    Song currentSong;
    Note currentNote;
    bool shouldPlay = false;

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer

    initLeds();
    configTimerA2();
    configDisplay();
    configKeypad();
    configureButtons();

    while (1)
    {
        keyPressed = getKey();
//        buttonPressed = getButtonState();

        if (auxCounter2 > 0)
        {
            auxCounter2--;
            if(!auxCounter2) {
                if(shouldPlay) {
                    noteCounter++;
                    currentNote = currentSong.notes[noteCounter % currentSong.noteCount];
                    BuzzerOnFreq(currentNote.frequency);
                    auxCounter2 = currentNote.eighths * 700;
                } else {
                    BuzzerOff();
                    auxCounter2 = currentSong.msBetweenNotes * 7;
                }
                shouldPlay = !shouldPlay;
            }
        }

        switch (state)
                {
                case WELCOME_SCREEN:
                    drawWelcome();
                    state = WAIT_FOR_START;
                    break;
                case WAIT_FOR_START:
                    if (keyPressed == '*')
                    {
                        state = START_COUNT_DOWN;
                        BuzzerOff();
                    }
                    break;
                case START_COUNT_DOWN:
//                    auxCounter = mainCounter;
                    state = COUNT_DOWN_SCREEN;
                case COUNT_DOWN_SCREEN:
                    if (drawCountdown() == true)
                    {
                        state = START_LEVEL;
                    }
                    break;
                case NEXT_LEVEL_SCREEN:
//                    drawNextLevel(level);
                    state = WAIT_FOR_START;
                    break;
                case START_LEVEL:
                    state = PLAYING_GAME;
                    currentSong = windmillHut;
                    currentNote = currentSong.notes[0];
                    BuzzerOnFreq(currentNote.frequency);
                    auxCounter2 = currentNote.eighths * 700;

//                    Graphics_Line line = { .xMin = 5, .xMax = 91, .yMin = 85 };
//                    Graphics_drawLineH(&g_sContext, &line);

                    break;
                case PLAYING_GAME:
                    break;
                case LOSS_SCREEN:
                    drawLoss();
                    state = WAIT_FOR_START;
                    break;
                }

        mainCounter++;
        lastKeyPressed = keyPressed;
    }
}

void drawWelcome()
{
    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext);                // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "MSP430", AUTO_STRING_LENGTH, 48,
                                25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "HERO", AUTO_STRING_LENGTH, 48,
                                35, TRANSPARENT_TEXT);

    Graphics_drawStringCentered(&g_sContext, "Press *", AUTO_STRING_LENGTH, 48,
                                70, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to Start", AUTO_STRING_LENGTH, 48,
                                80, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = { .xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    // We are now done writing to the display.  However, if we stopped here, we would not
    // see any changes on the actual LCD.  This is because we need to send our changes
    // to the LCD, which then refreshes the display.
    // Since this is a slow operation, it is best to refresh (or "flush") only after
    // we are done drawing everything we need.
    Graphics_flushBuffer(&g_sContext);
}

void drawLoss()
{
    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext);                // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "You Lost.", AUTO_STRING_LENGTH,
                                48, 25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, ">:(", AUTO_STRING_LENGTH, 48, 35,
    TRANSPARENT_TEXT);

    Graphics_drawStringCentered(&g_sContext, "Press *", AUTO_STRING_LENGTH, 48,
                                70, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to Try Again", AUTO_STRING_LENGTH,
                                48, 80, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = { .xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    Graphics_flushBuffer(&g_sContext);
}

void drawNextLevel(int level)
{

    Graphics_clearDisplay(&g_sContext);

    char buffer[10];
    snprintf(buffer, 9, "Level %d", level);

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, buffer, AUTO_STRING_LENGTH, 48, 35,
                                TRANSPARENT_TEXT);

    Graphics_drawStringCentered(&g_sContext, "Press *", AUTO_STRING_LENGTH, 48,
                                70, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "to Begin", AUTO_STRING_LENGTH, 48,
                                80, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = { .xMin = 5, .xMax = 91, .yMin = 5, .yMax = 91 };
    Graphics_drawRectangle(&g_sContext, &box);

    Graphics_flushBuffer(&g_sContext);
}

//bool drawCountdown(int mainCounter, int auxCounter)
bool drawCountdown(void)
{
    currentSecond = sec;
    int secPassed = 0;
    while(secPassed <= 3)
    {
        secPassed = sec - currentSecond;
    }
    switch (secPassed) {
        case 0:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48,
                                        50, TRANSPARENT_TEXT);
            setLED(0x08);
            Graphics_flushBuffer(&g_sContext);
            break;
        case 1:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48,
                                        50, TRANSPARENT_TEXT);
            setLED(0x04);
            Graphics_flushBuffer(&g_sContext);
        case 2:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48,
                                        50, TRANSPARENT_TEXT);
            setLED(0x02);
            Graphics_flushBuffer(&g_sContext);
        case 3:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48,
                                        50, TRANSPARENT_TEXT);
            setLED(0x0E);
            Graphics_flushBuffer(&g_sContext);
        case 4:
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_flushBuffer(&g_sContext);
            setLED(0x00);
            return true; // Not sure what this is for, but it was here
    }
    return false;
}

void drawAliens(Alien* aliens, int alienCount)
{
    int i;
    for (i = 0; i < alienCount; i++)
    {
        Alien currentAlien = aliens[i];
        if (currentAlien.visible && currentAlien.y >= 0)
        {
            unsigned char* str = &(currentAlien.key);
            Graphics_drawStringCentered(&g_sContext, str, 1, currentAlien.x,
                                        currentAlien.y, TRANSPARENT_TEXT);
        }
    }
    Graphics_flushBuffer(&g_sContext);
}


