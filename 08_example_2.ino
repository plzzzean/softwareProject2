// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // ★ 변경: sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)
#define _DIST_PEAK 200.0  // ★ 추가: 최대 밝기 지점 (mm)

// TIMEOUT은 인터벌의 절반 정도면 충분
#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL) // coefficent to convert duration to distance (mm/us)

unsigned long last_sampling_time;   // unit: msec

// ★ 추가: 거리 -> PWM(0~255) 삼각형 매핑 (active-low: 0=최대밝기, 255=꺼짐)
static inline int distanceToPwm(float mm) {
  if (mm <= 0.0f) return 255;                       // 타임아웃 등
  if (mm < _DIST_MIN || mm > _DIST_MAX) return 255; // 범위 밖 = 최소 밝기(꺼짐)

  if (mm <= _DIST_PEAK) {
    // 100~200mm : 255 → 0 (선형)
    float t = (mm - _DIST_MIN) / (_DIST_PEAK - _DIST_MIN); // 0~1
    float pwm = 255.0f + t * (0.0f - 255.0f);
    return (int)(pwm + 0.5f);
  } else {
    // 200~300mm : 0 → 255 (선형)
    float t = (mm - _DIST_PEAK) / (_DIST_MAX - _DIST_PEAK); // 0~1
    float pwm = 0.0f + t * (255.0f - 0.0f);
    return (int)(pwm + 0.5f);
  }
}

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar 
  
  // initialize serial port
  Serial.begin(57600);

  last_sampling_time = millis(); // ★ 초기화
}

void loop() { 
  float distance;

  // wait until next sampling time. // polling
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

  // ★ 변경: ON/OFF 대신 PWM 밝기 제어
  int pwm = distanceToPwm(distance);
  analogWrite(PIN_LED, pwm);

  // output the distance to the serial port
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.print(",pwm:");       Serial.print(pwm);
  Serial.println("");

  // ★ delay(50) 삭제 (논블로킹 샘플링)
  
  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm

  // Pulse duration to distance conversion example (target distance = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout) returns microseconds (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 하려면 * 1,000이 필요 ==>  SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예, pusseIn()이 100,000 이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}