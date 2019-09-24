/**************** ECE2049 Lab 2 ******************/
/***************  13 March 2019 ******************/
/******  Benjamin Ward, Jonathan Ferreira ********/
/*************************************************/

#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <inc/song.h>
#include <inc/peripherals.h>


// States utilized in main program state machine
typedef enum State
{
    WELCOME_SCREEN,
    WAIT_FOR_START,
    SONG_SELECT,
    SONG_SELECT_CHOOSE,
    START_COUNT_DOWN,
    COUNT_DOWN_SCREEN,
    NEXT_LEVEL_SCREEN,
    START_LEVEL,
    PLAYING_GAME,
    WIN_SCREEN,
    LOSS_SCREEN,
    RESET
} State;

// Global variables
int currentSecond;
int clock; // 5ms (200Hz) resolution clock based on timer A2
int leap = 0; // Helps us mitigate cumulative error in timer-based clock
char countdownState = 0; // Track state for countdown screen
int current = 0;
char* songSelection[4] = {"Windmill Hut",
                          "Second Song",
                          "Third Song",
                          "Fourth Song",};

// Function prototypes
void drawWelcome();
void drawLoss();
void drawWin();
void drawSongSelect(int);
bool drawCountdown(int);
void setLEDs(unsigned char);
void configSmolLEDs();
void setSmolLEDs(unsigned char);
unsigned char getButtonState();
void configTimerA2();
void configButtons();

// Program entry point
void main(void)
{
    // Main loop state
    int score = 0;
    unsigned char keyPressed = 0, buttonsPressed;
    unsigned long int startTime, deltaTime;
    State state = WELCOME_SCREEN;

    // Song-playing state
    int noteCounter = 0;
    Song currentSong;
    Note currentNote;
    bool shouldPlay = false, currentNoteScored = false;

    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    // Set up and configure peripherals and I/O
    initLeds();
    configSmolLEDs();
    configTimerA2();
    configDisplay();
    configKeypad();
    configButtons();

    // Main loop
    while (1)
    {
        keyPressed = getKey();

        // Reset override
        // This works at any time except for while waiting on the welcome screen.
        if (keyPressed == '#' && state != WAIT_FOR_START)
        {
            state = RESET;
        }

        switch (state)
        {
        case WELCOME_SCREEN:
            // Draw the welcome screen and wait until key input
            drawWelcome();
            state = WAIT_FOR_START;
            break;
        case WAIT_FOR_START:
            // If the key is pressed, transition to song select
            if (keyPressed == '*')
            {
                state = SONG_SELECT;
            }
            break;
        case SONG_SELECT:
            // Draws song select with Windmill Hut being the default
            drawSongSelect(current);
            state = SONG_SELECT_CHOOSE;
            break;
        case SONG_SELECT_CHOOSE:
            if (keyPressed == '2' && current > 0)
            {
                current -= 1;
                state = SONG_SELECT;
            }
            if (keyPressed == '8' && current < 3)
             {
                 current += 1;
                 state = SONG_SELECT;
             }
            if (keyPressed == '*')
            {
                state = START_COUNT_DOWN;
            }
            break;
        case START_COUNT_DOWN:
            // Moved from WAIT_FOR_START, so I can make room for SONG_SELECT
            // TODO: Compress this into RESET state
            BuzzerOff();
            setLEDs(0);
            countdownState = 0;
            score = 0;
            noteCounter = 0;

            // Save the timestamp for the beginning of the countdown and transition to the next state
            startTime = clock;
            state = COUNT_DOWN_SCREEN;
            break;
        case COUNT_DOWN_SCREEN:
            // Once the countdown is complete, transition to the beginnning of the level
            deltaTime = clock - startTime; // 1 deltaTime unit = 5ms
            if (drawCountdown(deltaTime))
            {
                state = START_LEVEL;
            }
            break;
        case START_LEVEL:
            // Select song
            currentSong = windmillHut; // TODO: Add song select menu
            currentNote = currentSong.notes[0];

            // Start playing the first note
            BuzzerOnFreq(currentNote.frequency);
            startTime = clock;
            currentNoteScored = false;

            // Transition to main gameplay loop
            state = PLAYING_GAME;
            break;
        case PLAYING_GAME:
            buttonsPressed = getButtonState();

            // Calculate the time since the last note started
            deltaTime = clock - startTime; // 1 deltaTime unit = 5ms

            // If a note is currently playing and the duration of the note has expired,
            // turn off the buzzer and wait until the next note
            if (shouldPlay
                    && deltaTime
                            > (currentSong.noteDuration * currentNote.eighths
                                    / 5))
            {
                BuzzerOff();
                shouldPlay = false;
                startTime = clock; // Reset the timer
            }
            // If a note is not currently playing and the duration of the gap between notes
            // has expired, transition to the next note
            else if (!shouldPlay
                    && deltaTime > (currentSong.silenceDuration / 5))
            {
                // If no button was pressed during the current note duration, reduce score
                if (!currentNoteScored)
                    score--;

                // Advance to the next note in the sequence
                noteCounter++;
                currentNote = currentSong.notes[noteCounter
                        % currentSong.noteCount];

                // If the song is complete, figure out if the player has won or lost and transition to the appropriate state
                if (noteCounter == currentSong.noteCount)
                {
                    // TODO: Revamp scoring
                    if (score > 0)
                        state = WIN_SCREEN;
                    else
                        state = LOSS_SCREEN;
                }

                // Play the next note in the song
                BuzzerOnFreq(currentNote.frequency);
                setSmolLEDs(0); //
                shouldPlay = true;
                currentNoteScored = false;
                startTime = clock;
            }

            // Display the current note button on the board LEDs
            setLEDs(currentNote.button);

            // If the current note has not yet been scored (i.e. no buttons have yet been pressed)
            // and the note is not a rest, check if it should be scored
            if (!currentNoteScored && currentNote.frequency != 0)
            {
                // If any buttons are pressed, we need to score the player
                if (buttonsPressed)
                {
                    // If the correct button is pressed, light the green user LED and add to the player's score
                    if (buttonsPressed & currentNote.button)
                    {
                        setSmolLEDs(BIT1);
                        score += 3;
                    }
                    // If the incorrect button is pressed, light the red LED and subtract from the player's score
                    else
                    {
                        setSmolLEDs(BIT0);
                        score--;
                    }
                    // Mark the current note as scored
                    currentNoteScored = true;
                    // TODO: Partial credit if you fix your mistake; for best game award
                }
            }
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

void drawSongSelect(int input)
{

    Graphics_clearDisplay(&g_sContext);

    char buffer[16];
    snprintf(buffer, 15, "%s", songSelection[input]);

    Graphics_drawStringCentered(&g_sContext, "Pick a song!", AUTO_STRING_LENGTH, 48,
                                15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, (uint8_t*) buffer, AUTO_STRING_LENGTH, 48,
                                35, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "2: Scroll up", AUTO_STRING_LENGTH, 7,
                                60, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "8: Scroll down", AUTO_STRING_LENGTH, 7,
                                70, TRANSPARENT_TEXT);
    Graphics_drawString(&g_sContext, "*: Begin", AUTO_STRING_LENGTH, 7,
                                80, TRANSPARENT_TEXT);

    // Draw a box around everything because it looks nice
    Graphics_Rectangle box = { .xMin = 4, .xMax = 92, .yMin = 4, .yMax = 92 };
    Graphics_drawRectangle(&g_sContext, &box);

    Graphics_flushBuffer(&g_sContext);
}


// Draw the countdown screen, returning true if the countdown is complete
bool drawCountdown(int timePassed)
{
    // countdownState is used to track the current display state
    if (countdownState == 0 && timePassed > 0)
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(BIT3);
        setSmolLEDs(BIT0);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 1;
    }
    else if (countdownState == 1 && timePassed > 200) // One second has passed
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(BIT2);
        setSmolLEDs(BIT1);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 2;
    }
    else if (countdownState == 2 && timePassed > 400) // Two seconds have passed
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(BIT1);
        setSmolLEDs(BIT0);
        Graphics_flushBuffer(&g_sContext);
        countdownState = 3;
    }
    else if (countdownState == 3 && timePassed > 600)
    {
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, "Go!", AUTO_STRING_LENGTH, 48,
                                    50, TRANSPARENT_TEXT);
        setLEDs(BIT3 | BIT2 | BIT1 | BIT0);
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

// Configure the Timer A2 ISR
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

    // Clock has ~5ms resolution, so every 200 interrupts is a second
    if (clock % 200 == 0)
    {
        currentSecond++;
    }
}

// Configure lab board buttons
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

// Get the state of the lab board buttons
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

// Set the lab board LED state
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

// Configure the LaunchPad user LEDs
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

// Set the LaunchPad user LED state
void setSmolLEDs(unsigned char outputState)
{
    // If the first LED should be on, turn it on, otherwise turn it off.
    if (outputState & BIT0)
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;

    // If the second LED should be on, turn it on, otherwise turn it off.
    if (outputState & BIT1)
        P4OUT |= BIT7;
    else
        P4OUT &= ~BIT7;
}