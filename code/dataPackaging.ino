// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       dataPackaging.ino
    Created:	11/22/2022 8:29:45 PM
    Author:     DESKTOP-SS9S9A0\hieud
*/

// Define User Types below here or use a .h file
//

//pakcet size
int n = 250;

//in seconds
int packetTiming = .125;
int entryTiming = packetTiming / n;
double timeCounter;
int i = 0;
// Define Function Prototypes that use User Types below here or use a .h file
//


// Define Functions below here or use other .ino or cpp files
//

struct datantime packet[250];
// The setup() function runs once each time the micro-controller starts
void setup()
{
    
}

// Add the main program code into the continuous loop() function
void loop()
{
    // time elapsed since program start
    double elapsedT = millis();
    while (elapsedT - timeCounter <= entryTiming)
    {
        //initialize a struct to hold raw data and elapsed time
        packet[i] = { readSens(),elapsedT };
        timeCounter = elapsedT;
        i++;
    }
    i = 0;

}

// 1 sensor 
int readSens()
{
    int data;
    return data;
}

struct datantime
{
    int data;
    double time;
};

/*
 We want 250 entries of data in a packet
 We want the filling process with in 1/8 of a second
 ==> 1 entry per 5e-4 second .0005s = .5ms
*/