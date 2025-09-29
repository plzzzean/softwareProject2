#define PIN_LED  9
#define PIN_TRIG 12   
#define PIN_ECHO 13   

#define SND_VEL 346.0     
#define INTERVAL 25       
#define PULSE_DURATION 10 
#define _DIST_MIN 100.0   
#define _DIST_MAX 300.0   
#define _DIST_PEAK 200.0  

#define TIMEOUT ((INTERVAL / 2) * 1000.0) 
#define SCALE (0.001 * 0.5 * SND_VEL) 

unsigned long last_sampling_time;   

static inline int distanceToPwm(float mm) {
  if (mm <= 0.0f) return 255;                       
  if (mm < _DIST_MIN || mm > _DIST_MAX) return 255; 
  if (mm <= _DIST_PEAK) {
    
    float t = (mm - _DIST_MIN) / (_DIST_PEAK - _DIST_MIN); 
    float pwm = 255.0f + t * (0.0f - 255.0f);
    return (int)(pwm + 0.5f);
  } else {
    float t = (mm - _DIST_PEAK) / (_DIST_MAX - _DIST_PEAK); 
    float pwm = 0.0f + t * (255.0f - 0.0f);
    return (int)(pwm + 0.5f);
  }
}

void setup() {
  
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  R
  pinMode(PIN_ECHO, INPUT);   
  digitalWrite(PIN_TRIG, LOW);  
  
  Serial.begin(57600);

  last_sampling_time = millis(); 
}

void loop() { 
  float distance;

  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); 

  int pwm = distanceToPwm(distance);
  analogWrite(PIN_LED, pwm);


  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.print(",pwm:");       Serial.print(pwm);
  Serial.println("");

  last_sampling_time += INTERVAL;
}

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; 
}
