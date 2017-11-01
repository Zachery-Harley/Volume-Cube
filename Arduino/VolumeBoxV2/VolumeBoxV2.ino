#define NEXT_TRACK 6
#define PLAY_PAUSE 5
#define PREV_TRACK 7

#define RED_LED 10
#define GREEN_LED 9
#define BLUE_LED 11

#define PIN_ROTARY_A 3
#define PIN_ROTARY_B 2

#define RESTING_DELAY 5000

int volume = 0;

volatile int newVolume = 0; //The new volume, this is decided from the encoder
volatile bool waiting = false; //True when the volume has been changed and needs to be sent
unsigned long lastAction = 0;

void setup() {
  //Set the pin modes
  Serial.begin(38400);
  pinMode(NEXT_TRACK, INPUT);
  pinMode(PLAY_PAUSE, INPUT);
  pinMode(PREV_TRACK, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT); 

   //Attach inturupt for encoder
   attachInterrupt(0, doEncoder, CHANGE);//PIN 2
   lastAction = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  getStatus();
  restingAction();
  if(Serial.available()){
    String rawInput = Serial.readString();
    int temp = rawInput.toInt();
    
    volume = temp;
    newVolume = temp;
    volumeToColor();
  }
  delay(10);
}

/**
 * Get the status of all the pins and return the comma seperated
 * string that is to be read by the desktop
 */
void getStatus(){
  String output = (String)newVolume + ",";
  output = readButton(output);
  if(waiting){
    volume = newVolume;
    volumeToColor();
    waiting = false;
    Serial.println(output);
  }

}

/**
 * Read the button states from the front panel. LOW is pressed.
 * prev, next, play
 */
String readButton(String input){
  int prev = digitalRead(PREV_TRACK);
  int next = digitalRead(NEXT_TRACK);
  int play = digitalRead(PLAY_PAUSE);
  input += (String)prev + "," + (String)play + "," + (String)next;
  //Check if a waiting need to be true
  if(!prev || !next || !play){
    waiting = true;
    delay(500);
  }
  return input;
}

/**
 * Set the colour of the LED
 * Operates using PWM, range of 0 - 100
 */
void setColor(int red, int green, int blue)
{
  analogWrite(RED_LED, red*2.55);
  analogWrite(GREEN_LED, green*2.55);
  analogWrite(BLUE_LED, blue*2.55);  
}

void volumeToColor(){
  int red = 100 - volume;
  int green = volume;
  if(green > 50) green = 100-green;
  int blue = volume;
  setColor(red, green, blue);
  lastAction = millis();
}

/**
 * Function called when the volume is changed, this is called through an interupt
 */
void doEncoder() {
  /* If pinA and pinB are both high or both low, it is spinning
   * forward. If they're different, it's going backward.
   */
  if (digitalRead(PIN_ROTARY_A) == digitalRead(PIN_ROTARY_B)) {
    newVolume++;
  } else {
    newVolume--;
  }
  waiting = true;

  //Ensure volume is in bounds
  if(newVolume > 100) newVolume = 100;
  if(newVolume < 0) newVolume = 0;
  delay(10);
}

/**
 * The resting action for the arduino RGB light
 * This bit aint my code its
 * https://gist.github.com/jamesotron/766994
 */
void restingAction(){
  if(millis() - lastAction > RESTING_DELAY){
    unsigned int rgbColour[3];
    // Start off with red.
    rgbColour[0] = 100;
    rgbColour[1] = 0;
    rgbColour[2] = 0;  
    
    // Choose the colours to increment and decrement.
    for (int decColour = 0; decColour < 3; decColour += 1) {
      int incColour = decColour == 2 ? 0 : decColour + 1;
      // cross-fade the two colours.
      for(int i = 0; i < 100; i += 1) {
        rgbColour[decColour] -= 1;
        rgbColour[incColour] += 1;
        setColor(rgbColour[0], rgbColour[1], rgbColour[2]);
        delay(5);
      }
    }
  }
}

