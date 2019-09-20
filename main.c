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

typedef enum State
{
    WELCOME_SCREEN,
    WAIT_FOR_START,
    START_COUNT_DOWN,
    COUNT_DOWN_SCREEN,
    NEXT_LEVEL_SCREEN,
    START_LEVEL,
    PLAYING_GAME,
    WIN_SCREEN,
    LOSS_SCREEN,
    RESET
} State;

// Important Global Variables
int currentSecond;
int clock; // 5ms (200Hz) resolution
int leap = 0; // Helps us mitigate cumulative error
char countdownState = 0;

// Function Prototypes
void drawWelcome();
void drawLoss();
void drawWin();
void drawNextLevel(int);
bool drawCountdown(int);
void setLEDs(unsigned char);
void configSmolLEDs();
void setSmolLEDs(unsigned char);
unsigned char getButtonState();
void configTimerA2();
void configButtons();
//void drawAliens(Alien*, int);

// What data structure(s) will you use to store pitch, duration and the corresponding LED? What length songs will you eventually
// want to play? Given how you choose to save your notes, etc., how much memory will that require?

// Total of 3 bytes

// Declare globals here

// Main
void main(void)
{
    int i, score = 0;
    unsigned char keyPressed = 0, lastKeyPressed = 0, buttonsPressed;
    unsigned long int startTime, deltaTime;
    State state = WELCOME_SCREEN;

    int noteCounter = 0;
    Song currentSong;
    Note currentNote;
    bool shouldPlay = false, currentNoteScored = false;

    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    initLeds();
    configSmolLEDs();
    configTimerA2();
    configDisplay();
    configKeypad();
    configButtons();

    while (1)
    {
        keyPressed = getKey();
        if (keyPressed == '#' && state != WAIT_FOR_START)
        {
            state = RESET;
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
                // TODO: Compress this into RESET state
                BuzzerOff();
                setLEDs(0);
                countdownState = 0;
                score = 0;
                noteCounter = 0;
            }
            break;
        case START_COUNT_DOWN:
            startTime = clock;
            state = COUNT_DOWN_SCREEN;
            break;
        case COUNT_DOWN_SCREEN:
            deltaTime = clock - startTime; // 1 deltaTime unit = 5ms
            if (drawCountdown(deltaTime))
            {
                state = START_LEVEL;
            }
            break;
        case NEXT_LEVEL_SCREEN:
//                    drawNextLevel(level);
            state = WAIT_FOR_START;
            break;
        case START_LEVEL:
            startTime = clock;
            currentSong = windmillHut;
            currentNote = currentSong.notes[0];
            BuzzerOnFreq(currentNote.frequency);
            currentNoteScored = false;
//            auxCounter2 = currentNote.eighths * 700;
            state = PLAYING_GAME;
            break;
        case PLAYING_GAME:
            // TODO: Check if we should be playing the next note based on clock
            deltaTime = clock - startTime; // 1 deltaTime unit = 5ms

            buttonsPressed = getButtonState();

            if (shouldPlay
                    && deltaTime
                            > (currentSong.noteDuration * currentNote.eighths
                                    / 5))
            {
                BuzzerOff();
                shouldPlay = false;
                startTime = clock;
            }
            else if (!shouldPlay
                    && deltaTime > (currentSong.silenceDuration / 5))
            {
                noteCounter++;
                if (noteCounter == currentSong.noteCount)
                {
                    // TODO: Revamp scoring
                    if (score > 0)
                        state = WIN_SCREEN;
                    else
                        state = LOSS_SCREEN;
                }
                currentNote = currentSong.notes[noteCounter
                        % currentSong.noteCount];
                BuzzerOnFreq(currentNote.frequency);
                setSmolLEDs(0);
                shouldPlay = true;
                if (!currentNoteScored)
                    score--;
                currentNoteScored = false;
                startTime = clock;
            }

            // Display the current note button on the board LEDs
            setLEDs(currentNote.button);
            if (!currentNoteScored && currentNote.frequency != 0)
            {
                                if (buttonsPressed)
                {
                    if (buttonsPressed & currentNote.button)
                    {
                        setSmolLEDs(BIT1);
                        score += 3;
                    }
                    else
                    {
                        setSmolLEDs(BIT0);
                        score--;
                    }
                    currentNoteScored = true;
                    // TODO: Partial credit if you fix your mistake; for best game award
                }
            }

            // If the current note has not already been scored
            // If the correct button is pressed, add to score and mark the current note as scored
            // If the note ends and the correct button has not been pressed, do not add to score

            break;
        case WIN_SCREEN:
            drawWin();
            state = WAIT_FOR_START;
            break;
        case LOSS_SCREEN:
            drawLoss();
            state = WAIT_FOR_START;
            break;
        case RESET:
            BuzzerOff();
            setLEDs(0);
            countdownState = 0;
            score = 0;
            noteCounter = 0;

            state = WELCOME_SCREEN;
            break;
        }

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
    Graphics_drawStringCentered(&g_sContext, "HERO", AUTO_STRING_LENGTH, 48, 35,
    TRANSPARENT_TEXT);

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
    Graphics_clearDisplay(&g_sContext);    // Clear the display

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

void drawWin()
{
    // *** Intro Screen ***
    Graphics_clearDisplay(&g_sContext);    // Clear the display

    // Write some text to the display
    Graphics_drawStringCentered(&g_sContext, "You Win!", AUTO_STRING_LENGTH, 48,
                                25, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, ":D", AUTO_STRING_LENGTH, 48, 35,
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

bool drawCountdown(int timePassed)
{
    if (countdownState == 0 && timePassed > 0)
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(0x08);
        setSmolLEDs(BIT0);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 1;
    }
    else if (countdownState == 1 && timePassed > 200) // One second has passed
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(0x04);
        setSmolLEDs(BIT1);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 2;
    }
    else if (countdownState == 2 && timePassed > 400) // Two seconds have passed
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(0x02);
        setSmolLEDs(BIT0);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 3;
    }
    else if (countdownState == 3 && timePassed > 600)
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(0x0E);
        setSmolLEDs(BIT1 | BIT0);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 4;
    }
    else if (countdownState == 4 && timePassed > 800)
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_flushBuffer(&g_sContext);
        setLEDs(0);
        setSmolLEDs(0);
        return true;
    }
    return false;
}

//void drawAliens(Alien* aliens, int alienCount)
//{
//    int i;
//    for (i = 0; i < alienCount; i++)
//    {
//        Alien currentAlien = aliens[i];
//        if (currentAlien.visible && currentAlien.y >= 0)
//        {
//            unsigned char* str = &(currentAlien.key);
//            Graphics_drawStringCentered(&g_sContext, str, 1, currentAlien.x,
//                                        currentAlien.y, TRANSPARENT_TEXT);
//        }
//    }
//    Graphics_flushBuffer(&g_sContext);
//}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void)
{
    if (leap < 141)
    {
        clock++;
        leap++;
    }
    else
    {
        clock += 2; // Account for timing error
        leap = 0;
    }

    if (clock % 200 == 0)
    {
        currentSecond++;
    }
}

// Write  a  function  to  configure  the  4  lab  board  buttons,  S1  through  S4
void configButtons()
{
    // P7.0, P3.6, P2.2, P7.4
    // Configure P2.2
    P2SEL &= ~(BIT2);    // Select pin for DI/O
    P2DIR &= ~(BIT2);    // Set pin as input
    P2REN |= (BIT2);    // Enable pull-up resistor
    P2OUT |= (BIT2);

    // Configure P3.6
    P3SEL &= ~(BIT6);    // Select pin for DI/O
    P3DIR &= ~(BIT6);    // Set pin as input
    P3REN |= (BIT6);    // Enable pull-up resistor
    P3OUT |= (BIT6);

    // Configure P7.0 and P7.4
    P7SEL &= ~(BIT4 | BIT0);    // Select pins for DI/O
    P7DIR &= ~(BIT4 | BIT0);    // Set pins as input
    P7REN |= (BIT4 | BIT0);    // Enable pull-up resistors
    P7OUT |= (BIT4 | BIT0);
}

void configTimerA2(void)
{
    TA2CTL = TASSEL_1 + ID_0 + MC_1;
    TA2CCR0 = 164; // = ~1/200 seconds
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled

    _BIS_SR(GIE); // Enable interrupts now, so we don't have to do it in main later
}

// Write  a  function  that  returns  the  state  of  the  lab  board  buttons  with  1=pressed  and  0=not pressed.
unsigned char getButtonState()
{
    unsigned char ret = 0x00;
    // P2.2
    if (~P2IN & BIT2)
        ret |= BIT1; // Button 2
    // P3.6
    if (~P3IN & BIT6)
        ret |= BIT2;    // Button 1
    // P7.0
    if (~P7IN & BIT0)
        ret |= BIT3;    // Button 0
    // P7.4
    if (~P7IN & BIT4)
        ret |= BIT0;    // Button 3
    return ret;
}

void setLEDs(unsigned char state)
{
    unsigned char mask = 0;

    // Turn all LEDs off to start
    P6OUT &= ~(BIT4 | BIT3 | BIT2 | BIT1);

    if (state & BIT0)
        mask |= BIT4;    // Right most LED P6.4
    if (state & BIT1)
        mask |= BIT3;    // next most right LED P.3
    if (state & BIT2)
        mask |= BIT1;    // third most left LED P6.1
    if (state & BIT3)
        mask |= BIT2;    // Left most LED on P6.2
    P6OUT |= mask;
}

void configSmolLEDs()
{
    // P4.7, P1.0
    // Configure P1.1
    P1SEL &= ~(BIT0);    // Select pins for DI/O
    P1DIR |= BIT0;    // Set pin as output
    P1OUT &= ~(BIT0);    // Set off by default

    // Configure P4.7
    P4SEL &= ~(BIT7);    // Select pins for DI/O
    P4DIR |= BIT7;    // Set pin as output
    P4OUT &= ~(BIT7);    // Set off by default
}

// Write a complete C function to configure and light 2 user LEDs on the MSP430F5529 Launchpad board based on the char argument
// passed. If BIT0 of the argument = 1, LED1 is lit and if BIT0=0 then LED1 is off. Similarly, if BIT1 of the argument = 1, LED2
// is lit and if BIT1=0 then LED2 is off.
void setSmolLEDs(unsigned char outputState)
{
    // xxxx | 0001 = xxx1
    // xxxx & 0001 = 000x
    if (outputState & BIT0)
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;

    if (outputState & BIT1)
        P4OUT |= BIT7;
    else
        P4OUT &= ~BIT7;
}
