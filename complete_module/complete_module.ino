#include <Adafruit_Fingerprint.h>
#include<NewPing.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


///////////
// Pins //
//////////

// Fingerprint
uint8_t led1 = 12; // FingerPrint matched
uint8_t led2 = 11; // FingerPrint not matched
uint8_t led3 = 10; // FingerPrint enrolled
uint8_t enroll_val = 0;

// Gear Motor
uint8_t rpwm = 9;
uint8_t lpwm = 8;

// IR Sensors
uint8_t ir_front_in_pin = 7;
uint8_t ir_back_in_pin = 6;

// Push Button
uint8_t pb_in = 5;

// Sonar Sensor
int trigPin = A0;
int echoPin = A1;


///////////////////////
// Global Variables //
//////////////////////

// Fingerprint
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id = 1;
uint8_t matched = 0;
uint8_t tries_on_enroll = 0;

// Sonar
int max_distance = 500;
int dist = 0;
int end_dist = 26;
NewPing sonar(trigPin, echoPin,max_distance);

// Motor
int motor_duration = 5000;


///////////
// Setup //
///////////
void setup()
{
  pinMode(pb_in, INPUT);
 
  pinMode(rpwm, OUTPUT);
  pinMode(lpwm, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  
  pinMode(ir_front_in_pin, INPUT);
  pinMode(ir_back_in_pin, INPUT);

  Serial.begin(9600);
  while (!Serial)
    ; // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
    {
      delay(1);
    }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);
}

//////////
// Loop //
//////////
void loop() // run over and over again
{
  clear();  // Clear the database
  enroll(); // Enroll a fingerprint
  
  // if locker is empty clear db
  dist = sonar.ping_cm();
  Serial.println(dist);
  if (dist < end_dist)
    verify(); // Verify a fingerprint

  delay(100);
}

///////////////////////
// Custom Functions //
//////////////////////

// Motor

void off()
{
  digitalWrite(lpwm, LOW);
  digitalWrite(rpwm, LOW);
}

void right()
{
  digitalWrite(lpwm, LOW);
  digitalWrite(rpwm, HIGH);
}

void left()
{
  digitalWrite(rpwm, LOW);
  digitalWrite(lpwm, HIGH);
}

// Fingerprint

void clear()
{
  matched = 0;
  tries_on_enroll = 0;
  finger.emptyDatabase();
  turn_off_led3();
  Serial.println("Now database is empty :)");
}

void enroll()
{
  Serial.println("Enter Fingerprint to Register");
  enroll_val = 0;
  while (!enroll_val)
  {
    enroll_val = getFingerprintEnroll();
    // if (digitalRead(pb_in) == HIGH)
    // {
    //   delay(500);
    //   uint8_t rotation = digitalRead(ir_back_in_pin) == LOW ? 1 : 2;   // 1 = right, 2 = left
    //   uint8_t run_motor = 1;
    //   if (rotation == 1)
    //   {
    //     while(run_motor)
    //     {
    //         // while HIGH motor need to rotate to reach dest
    //         run_motor = digitalRead(ir_front_in_pin) == LOW;
    //         right();
    //     }
    //   }
    //   else
    //   {
    //     while(run_motor)
    //     {
    //         run_motor = digitalRead(ir_back_in_pin) == HIGH;
    //         left();
    //     }
    //   }
    // }
  }
  
  uint8_t run_motor = 1;
  while(run_motor)
  {
      // while HIGH motor need to rotate to reach dest
      run_motor = digitalRead(ir_front_in_pin) == HIGH;
      left();
      delay(100);
  }
  off();
  delay(motor_duration);
  run_motor = 1;
  while(run_motor)
  {
      // while HIGH motor need to rotate to 
      run_motor = digitalRead(ir_back_in_pin) == HIGH;
      right();
      delay(100);
  }
  off(); 
}

void verify()
{
  if (tries_on_enroll < 1)
  {
    Serial.println("Enter Fingerprint to Unlock");
    while (matched == 0)
    {
      getFingerprintID();
      delay(50);
    }

  
  uint8_t run_motor = 1;
  while(run_motor)
  {
      // while HIGH motor need to rotate to reach dest
      run_motor = digitalRead(ir_front_in_pin) == HIGH;
      left();
      delay(100);
  }
  off();
  delay(motor_duration);
  run_motor = 1;
  while(run_motor)
  {
      // while HIGH motor need to rotate to 
      run_motor = digitalRead(ir_back_in_pin) == HIGH;
      right();
      delay(100);
  }
  off();
  }
  else
    blink_led3();
}

///////////////////////////
// LED Control Functions //
///////////////////////////
// LED 1 (FingerPrint matched)
void turn_on_led1()
{
  digitalWrite(led1, HIGH);
}

void turn_off_led1()
{
  digitalWrite(led1, LOW);
}

void blink_led1()
{
  turn_on_led1();
  delay(1000);
  turn_off_led1();
}

// LED 2 (FingerPrint not matched)
void blink_led2()
{
  digitalWrite(led2, HIGH);
  delay(1000);
  digitalWrite(led2, LOW);
}

// LED 3 (FingerPrint enrolled)
void turn_on_led3()
{
  digitalWrite(led3, HIGH);
}

void turn_off_led3()
{
  digitalWrite(led3, LOW);
}

void blink_led3()
{
  turn_on_led3();
  delay(1000);
  turn_off_led3();
}

///////////////////////////////////////
// Pre-defined Functions (Modified) //
///////////////////////////////////////
uint8_t getFingerprintID()
{
  uint8_t p = finger.getImage();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image taken");
    break;
  case FINGERPRINT_NOFINGER:
    // Serial.println("No finger detected");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_IMAGEFAIL:
    Serial.println("Imaging error");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Found a print match!");
    blink_led1(); // modified
    matched = 1;  // modified
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    blink_led2(); // modified
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Did not find a match");
    blink_led2(); // modified
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    blink_led2(); // modified
    return p;
  }

  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);

  return finger.fingerID;
}

uint8_t getFingerprintEnroll()
{

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print("");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER)
  {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p)
  {
  case FINGERPRINT_OK:
    Serial.println("Image converted");
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println("Image too messy");
    return p;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println("Communication error");
    return p;
  case FINGERPRINT_FEATUREFAIL:
    Serial.println("Could not find fingerprint features");
    return p;
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println("Could not find fingerprint features");
    return p;
  default:
    Serial.println("Unknown error");
    return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Prints matched!");
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH)
  {
    Serial.println("Fingerprints did not match");

    /*
     * It's long story. Rahat tried to enroll his finger and on enroll his fingerprint
     * wasn't matching no matter how many time he tried. So I am adding auto reset feature
     * on 3rd wrong match on enroll.
     *
     * That means if you are trying to enroll your finger and it's not matching more than 3 times
     * then it will reset the fingerprint sensor and you can try again.
     */
    tries_on_enroll++; // modified
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK)
  {
    Serial.println("Stored!");
    turn_on_led3(); // modified
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR)
  {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION)
  {
    Serial.println("Could not store in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR)
  {
    Serial.println("Error writing to flash");
    return p;
  }
  else
  {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
