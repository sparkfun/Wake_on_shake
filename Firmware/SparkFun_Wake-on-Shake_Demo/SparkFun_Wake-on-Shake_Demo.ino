/****************************************************************************** <filename>
SparkFun Wake-on-Shake Demo Code
Toni Klopfenstein @ SparkFun Electronics
March 2015
https://github.com/sparkfun/Wake_on_shake

Basic code to demonstrate the Wake-on-Shake's ability to cycle
an Arduino driving 4 LEDs on and off. 

Development environment specifics:
Developed in Arduino 1.6

This code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!
Distributed as-is; no warranty is given.

********************************************************/

//Define LED Pin Connections
int LED1 = 3;
int LED2 = 5;
int LED3 = 6;
int LED4 = 9;

//Define Wake-on-Shake WAKE pin connection
int WAKE = 10;

/***************************Setup Loop************************/
void setup() {
  
  //Set LED pin connections as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(WAKE, OUTPUT);
  
  //Set WAKE pin HIGH to prevent the Wake-on-Shake from 'sleeping'
  digitalWrite(WAKE, HIGH);
  
  //Functions to occur on each wake-up of the system
  //This will fade the LEDs up to full brightness one by one
  
  //Slowly birghten the first LED
  for( int i = 0; i<255; i++)
  {
    analogWrite(LED1, i);
    delay(20);
  }
  //Turn off the LED
  digitalWrite(LED1, LOW);
  
  for( int i = 0; i<255; i++)
  {
    analogWrite(LED2, i);
    delay(20);
  }
  digitalWrite(LED2, LOW);
  
  for( int i = 0; i<255; i++)
  {
    analogWrite(LED3, i);
    delay(20);
  }
  digitalWrite(LED3, LOW);
  
  for( int i = 0; i<255; i++)
  {
    analogWrite(LED4, i);
    delay(20);
  }
  digitalWrite(LED4, LOW);
  
  //Allow the Wake-on-Shake to go to sleep
  digitalWrite(WAKE, LOW);
}

/***************************Main Loop************************/
void loop() {
//No main function here as the Wake-on-Shake will return to low-power mode
}
