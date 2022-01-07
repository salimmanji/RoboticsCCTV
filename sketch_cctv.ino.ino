/*
 * COMP3012 W2021 
 * Term Project
 * @author Salim Manji
 * This program manages the motion detection of 
 * my CCTV setup. If motion is detected, a specific
 * char is sent via serial to a Raspberry Pi Microprocessor.
 * After sending, the Arduino awaits feedback from the RPi to 
 * ensure all scripts have run before returning to the Armed state.
 * 
 * The Raspberry Pi handles the rest.
 *
 *  ****** PLEASE NOTE ****
 *  It is best to keep the debug OFF, as messages are being sent between the RPi and the Arduino. I noticed
 *  some negative effects when enabling debug mode. Use at your own peril.
 * 
 * References:
 * https://thesolaruniverse.wordpress.com/2020/10/01/the-rcwl-0516-doppler-radar-motion-sensor-an-arduino-nano-and-an-event-led/
 * https://www.arrow.com/en/research-and-events/articles/raspberry-pi-to-arduino-serial-communication-via-usb
 *
 * Blank State Machine provided by J. Kidney
 */

//All states
enum State
{
    Armed,
    Feedback
};

//For debugging and easier reading states
char *stateNames[] =
    {
        "Armed State",
        "Feedback State"};

boolean debug = false;       // set to false to not show debug messages
State currentState = Armed; // Set start state

#define sensorPin 2 // RCWL-0516 connected to this pin
int sensorVal = 0;  // initial RCWL-0516 output value as seen by Nano
const int detectedMotion = 0;
const int armingDelay = 5000;

void setup()
{
    Serial.begin(9600); // initialize serial communication:
    pinMode(sensorPin, INPUT);
    printDebugMessage("Machine starting up");
    delay(armingDelay);
}

void loop()
{
    printDebugMessage("Current State");
    runStateMachine();
}

//---------------------------------------------------
void printDebugMessage(char *msg)
{
    if (debug)
    {
        unsigned long myTime = millis();
        Serial.print(myTime);
        Serial.print(": (");
        Serial.print(stateNames[currentState]);
        Serial.print(") ");
        Serial.println(msg);
    }
}
//---------------------------------------------------
void runStateMachine()
{
    switch (currentState)
    {
    case Armed:
        runArmed();
        break;

    case Feedback:
        runFeedback();
        break;
    }
}

/*
 * While in the armed state, the Arduino checks to see if motion has been detected.
 * If motion has been detected, it trips the RPi to take a photo and video.
 */
void runArmed()
{
    sensorVal = digitalRead(sensorPin);

    if (sensorVal == LOW)
    {
        Serial.println(detectedMotion);
        currentState = Feedback;
        printDebugMessage("Motion has been detected, signalling RPi!");
    }
    else
    {
        delay(10000);
        printDebugMessage("Nothing to report. Delaying for 10 seconds.");
    }
}

/*
 * After signalling the RPi to handle things, the Arduino awaits feedback from
 * the microprocessor. If a "1" is received, the RPi has completed all functions
 * required, and the Arduino can go back to checking for motion.
 */
void runFeedback()
{
    printDebugMessage("Checking for feedback from RPi...");
    int piInput = 0;
    if (Serial.available() > 0)
    {
        piInput = Serial.read();
        if (piInput == 1)
        {
            printDebugMessage("RPi has completed all operations.");
            currentState = Armed;
            printDebugMessage("Returning to Armed State.");
        }
    }
}
