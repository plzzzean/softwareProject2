#include <Servo.h>

#define PIN_LED   9    
#define PIN_TRIG  12   
#define PIN_ECHO  13  
#define PIN_SERVO 10   


#define SND_VEL 346.0          
#define INTERVAL 25            
#define PULSE_DURATION 10      
#define _DIST_MIN 180.0        
#define _DIST_MAX 360.0        

#define TIMEOUT ((INTERVAL/2) * 1000.0) 
#define SCALE (0.001 * 0.5 * SND_VEL)   

#define _EMA_ALPHA 0.30        

#define _DUTY_MIN 500   
#define _DUTY_MAX 2400   
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX)/2)  

float dist_prev = _DIST_MAX;   
float dist_ema  = _DIST_MAX;   
unsigned long last_sampling_time = 0;

Servo myservo;


static inline float clampf(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}
static inline float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}
static inline float mapRange(float x, float in_lo, float in_hi, float out_lo, float out_hi) {
  float t = (x - in_lo) / (in_hi - in_lo);
  return lerp(out_lo, out_hi, t);
}


float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, (unsigned long)TIMEOUT) * SCALE; 
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);         

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU); 


  dist_prev = _DIST_MAX;
  dist_ema  = dist_prev;
  last_sampling_time = millis();

  Serial.begin(57600);
}

void loop() {
  if (millis() - last_sampling_time < INTERVAL) return;
  last_sampling_time = millis();

 
  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  float dist_filtered;
  if (dist_raw == 0.0 || dist_raw < _DIST_MIN || dist_raw > _DIST_MAX) {
    dist_filtered = dist_prev;                 
  } else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;                    
  }


  dist_ema = _EMA_ALPHA * dist_filtered + (1.0f - _EMA_ALPHA) * dist_ema;


  float d_mm = clampf(dist_ema, _DIST_MIN, _DIST_MAX);
  float duty_us = mapRange(d_mm, _DIST_MIN, _DIST_MAX, _DUTY_MIN, _DUTY_MAX);

 
  myservo.writeMicroseconds((int)duty_us);


  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX && dist_raw != 0.0) {
    digitalWrite(PIN_LED, LOW);  
  } else {
    digitalWrite(PIN_LED, HIGH);  
  }

  Serial.print("Min:");    Serial.print(_DIST_MIN);
  Serial.print("\tdist:");  Serial.print(dist_raw);
  Serial.print("\tema:");   Serial.print(dist_ema);
  Serial.print("\tServo:"); Serial.print(myservo.read());
  Serial.print("\tMax:");   Serial.println(_DIST_MAX);


}
