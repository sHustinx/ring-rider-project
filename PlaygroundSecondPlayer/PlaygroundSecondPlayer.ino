
const int BUTTON_PIN_MIN = 8; //range leds -5
const int BUTTON_PIN_MAX = 11;

// Variables will change:
int lastState = LOW;  // the previous state from the input pin
int currentState;                // the current reading from the input pin

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // initialize the pushbutton pin as an pull-up input
  // the pull-up input pin will be HIGH when the switch is open and LOW when the switch is closed.

  for (int i = BUTTON_PIN_MIN; i <= BUTTON_PIN_MAX; i++){
    // button
    pinMode(i, INPUT_PULLUP);
    // LED
    pinMode(i-5, OUTPUT); 
    Serial.println(i-5);
  }
}

void loop() {
  // read the state of the switch/button:
  //currentState = digitalRead(BUTTON_PIN_1);

  for (int i = BUTTON_PIN_MIN; i <= BUTTON_PIN_MAX; i++){
     int state = digitalRead(i);
     if (state == 0){
        //Buton pressed
        digitalWrite(i-5, HIGH);
        Serial.println(i);
     }
     else{
        //Buton not pressed
        digitalWrite(i-5, LOW);
     }
  }

}
