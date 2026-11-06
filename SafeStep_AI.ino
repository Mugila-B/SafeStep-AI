#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "Joohi";
const char* WIFI_PASSWORD = "JoohiRm ";   


const char* SERVER_URL =
    "https://safestep-backend-7bwk.onrender.com/api/safestep";


#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOISTURE_PIN 34
#define VIBRATION_PIN 25



#define OBSTACLE_DISTANCE 100

#define MOISTURE_DRY 2800
#define MOISTURE_SMOOTH 2200
#define MOISTURE_SLIPPERY 1500



const unsigned long SEND_INTERVAL = 1000;
const unsigned long SENSOR_INTERVAL = 100;

unsigned long lastSensorTime = 0;

#define OBSTACLE_CLEAR_READS 2

#define VIBRATION_DEBUG_PULSES 1


#define SURFACE_DRY 0
#define SURFACE_SMOOTH 1
#define SURFACE_SLIPPERY 2
#define SURFACE_VERY_SLIPPERY 3

volatile bool sharedObstacle = false;
volatile int sharedSurfaceCode = SURFACE_DRY;
volatile int activePatternId = 0;

struct SensorSnapshot
{
  float distance;
  bool obstacle;
  int moistureRaw;
  int moisturePercent;
  char surface[16];
  int riskScore;
  char aiDecision[16];
  char hazard[32];
};

SensorSnapshot latestData;
bool snapshotValid = false;
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;


enum VibPattern
{
  VIB_NONE = 0,                  
  VIB_OBSTACLE = 1,              
  VIB_SLIPPERY = 2,              
  VIB_VERY_SLIPPERY = 3,         
  VIB_OBSTACLE_SLIPPERY = 4,     
  VIB_OBSTACLE_VERY_SLIPPERY = 5 
};


const uint16_t PAT_OBSTACLE[] = {200, 800};

const uint16_t PAT_SLIPPERY[] = {1000, 1000};

const uint16_t PAT_VERY_SLIPPERY[] = {1800, 500};

const uint16_t PAT_OBSTACLE_SLIPPERY[] = {200, 150, 1000, 1000};

const uint16_t PAT_OBSTACLE_VERY_SLIPPERY[] = {200, 150, 1800, 500};

bool motorState = false;

void setMotor(bool on)
{
  digitalWrite(VIBRATION_PIN, on ? HIGH : LOW);

  if (on != motorState)
  {
    motorState = on;

#if VIBRATION_DEBUG_PULSES
    Serial.println(on ? "VIBRATION: MOTOR ON" : "VIBRATION: MOTOR OFF");
#endif
  }
}

int selectPattern(bool obstacle, int surfaceCode)
{
  if (surfaceCode == SURFACE_VERY_SLIPPERY && obstacle)
  {
    return VIB_OBSTACLE_VERY_SLIPPERY;
  }

  if (surfaceCode == SURFACE_SLIPPERY && obstacle)
  {
    return VIB_OBSTACLE_SLIPPERY;
  }

  if (surfaceCode == SURFACE_VERY_SLIPPERY)
  {
    return VIB_VERY_SLIPPERY;
  }

  if (surfaceCode == SURFACE_SLIPPERY)
  {
    return VIB_SLIPPERY;
  }

  if (obstacle)
  {
    return VIB_OBSTACLE;
  }

  return VIB_NONE;
}

const uint16_t* getPatternSteps(int pattern, int &count)
{
  switch (pattern)
  {
    case VIB_OBSTACLE:
      count = sizeof(PAT_OBSTACLE) / sizeof(PAT_OBSTACLE[0]);
      return PAT_OBSTACLE;

    case VIB_SLIPPERY:
      count = sizeof(PAT_SLIPPERY) / sizeof(PAT_SLIPPERY[0]);
      return PAT_SLIPPERY;

    case VIB_VERY_SLIPPERY:
      count = sizeof(PAT_VERY_SLIPPERY) / sizeof(PAT_VERY_SLIPPERY[0]);
      return PAT_VERY_SLIPPERY;

    case VIB_OBSTACLE_SLIPPERY:
      count = sizeof(PAT_OBSTACLE_SLIPPERY) / sizeof(PAT_OBSTACLE_SLIPPERY[0]);
      return PAT_OBSTACLE_SLIPPERY;

    case VIB_OBSTACLE_VERY_SLIPPERY:
      count = sizeof(PAT_OBSTACLE_VERY_SLIPPERY) / sizeof(PAT_OBSTACLE_VERY_SLIPPERY[0]);
      return PAT_OBSTACLE_VERY_SLIPPERY;

    default:
      count = 0;
      return NULL;
  }
}

void printPatternName(int pattern)
{
  switch (pattern)
  {
    case VIB_OBSTACLE:
      Serial.println("VIBRATION: OBSTACLE PATTERN");
      break;
    case VIB_SLIPPERY:
      Serial.println("VIBRATION: SLIPPERY PATTERN");
      break;
    case VIB_VERY_SLIPPERY:
      Serial.println("VIBRATION: VERY SLIPPERY PATTERN");
      break;
    case VIB_OBSTACLE_SLIPPERY:
      Serial.println("VIBRATION: OBSTACLE + SLIPPERY PATTERN");
      break;
    case VIB_OBSTACLE_VERY_SLIPPERY:
      Serial.println("VIBRATION: OBSTACLE + VERY SLIPPERY PATTERN");
      break;
    default:
      Serial.println("VIBRATION: OFF");
      break;
  }
}


void vibrationTask(void* parameter)
{
  int currentPattern = -1;        
  int stepIndex = 0;
  unsigned long stepStart = 0;

  for (;;)
  {
    bool obstacle = sharedObstacle;
    int surfaceCode = sharedSurfaceCode;

    int wanted = selectPattern(obstacle, surfaceCode);

    if (wanted != currentPattern)
    {
      currentPattern = wanted;
      activePatternId = wanted;
      stepIndex = 0;
      stepStart = millis();

      printPatternName(wanted);

      if (wanted == VIB_NONE)
      {
        setMotor(false);
      }
      else
      {
        setMotor(true);           
      }
    }
    else if (wanted == VIB_NONE)
    {
      setMotor(false);           
    }
    else
    {
      int count = 0;
      const uint16_t* steps = getPatternSteps(wanted, count);

      if (steps == NULL || count == 0)
      {
        setMotor(false);
      }
      else if (millis() - stepStart >= steps[stepIndex])
      {
        stepIndex = (stepIndex + 1) % count;
        stepStart = millis();

        setMotor(stepIndex % 2 == 0);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}


void connectWiFi()
{
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 40)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi: CONNECTED");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  else
  {
    Serial.println("WiFi: CONNECTION FAILED");
    Serial.print("WiFi Status Code: ");
    Serial.println(WiFi.status());
  }
}


float readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return 999.0;
  }

  float distance = duration * 0.0343 / 2.0;

  return distance;
}


int readMoisture()
{
  return analogRead(MOISTURE_PIN);
}

int calculateMoisturePercent(int moistureRaw)
{
  int percent = map(moistureRaw, 3500, 1000, 0, 100);

  percent = constrain(percent, 0, 100);

  return percent;
}

String classifySurface(int moistureRaw)
{
  if (moistureRaw >= MOISTURE_DRY)
  {
    return "DRY";
  }
  else if (moistureRaw >= MOISTURE_SMOOTH)
  {
    return "SMOOTH";
  }
  else if (moistureRaw >= MOISTURE_SLIPPERY)
  {
    return "SLIPPERY";
  }
  else
  {
    return "VERY_SLIPPERY";
  }
}

int surfaceToCode(String surface)
{
  if (surface == "SMOOTH")        return SURFACE_SMOOTH;
  if (surface == "SLIPPERY")      return SURFACE_SLIPPERY;
  if (surface == "VERY_SLIPPERY") return SURFACE_VERY_SLIPPERY;

  return SURFACE_DRY;
}


int calculateRiskScore(float distance, String surface)
{
  int risk = 0;

  if (distance <= 30)
  {
    risk += 60;
  }
  else if (distance <= 60)
  {
    risk += 40;
  }
  else if (distance <= 100)
  {
    risk += 25;
  }

  if (surface == "DRY")
  {
    risk += 0;
  }
  else if (surface == "SMOOTH")
  {
    risk += 10;
  }
  else if (surface == "SLIPPERY")
  {
    risk += 30;
  }
  else if (surface == "VERY_SLIPPERY")
  {
    risk += 50;
  }

  risk = constrain(risk, 0, 100);

  return risk;
}


String getAIDecision(int riskScore)
{
  if (riskScore >= 75)
  {
    return "HIGH_RISK";
  }
  else if (riskScore >= 40)
  {
    return "MEDIUM_RISK";
  }
  else
  {
    return "LOW_RISK";
  }
}

String getHazard(bool obstacle, String surface)
{
  bool slippery = surface == "SLIPPERY";
  bool verySlippery = surface == "VERY_SLIPPERY";

  if (obstacle && verySlippery)
  {
    return "OBSTACLE_AND_VERY_SLIPPERY";
  }

  if (obstacle && slippery)
  {
    return "OBSTACLE_AND_SLIPPERY";
  }

  if (obstacle)
  {
    return "OBSTACLE_DETECTED";
  }

  if (verySlippery)
  {
    return "VERY_SLIPPERY_SURFACE";
  }

  if (slippery)
  {
    return "SLIPPERY_SURFACE";
  }

  return "SAFE";
}

void sendToServer(
    float distance,
    bool obstacle,
    int moistureRaw,
    int moisturePercent,
    String surface,
    int riskScore,
    String aiDecision,
    String hazard)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Website update skipped - WiFi disconnected.");
    return;
  }

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  Serial.println();
  Serial.println("Connecting to SafeStep backend...");

  if (!http.begin(client, SERVER_URL))
  {
    Serial.println("HTTP connection initialization failed.");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"device\":\"SafeStep\",";
  json += "\"distance_cm\":" + String(distance, 2) + ",";
  json += "\"obstacle\":" + String(obstacle ? "true" : "false") + ",";
  json += "\"moisture_raw\":" + String(moistureRaw) + ",";
  json += "\"moisture_percent\":" + String(moisturePercent) + ",";
  json += "\"surface\":\"" + surface + "\",";
  json += "\"risk_score\":" + String(riskScore) + ",";
  json += "\"ai_decision\":\"" + aiDecision + "\",";
  json += "\"hazard\":\"" + hazard + "\",";
  json += "\"wifi\":\"CONNECTED\"";
  json += "}";

  Serial.println("Sending JSON:");
  Serial.println(json);

  int httpResponseCode = http.POST(json);

  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0)
  {
    String response = http.getString();

    Serial.println("Server Response:");
    Serial.println(response);
  }
  else
  {
    Serial.print("HTTP Error: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
}


void networkTask(void* parameter)
{
  connectWiFi();

  for (;;)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println();
      Serial.println("WiFi disconnected. Reconnecting...");

      connectWiFi();

      if (WiFi.status() != WL_CONNECTED)
      {
        vTaskDelay(pdMS_TO_TICKS(5000));
        continue;
      }
    }

    SensorSnapshot data;
    bool valid = false;

    portENTER_CRITICAL(&dataMux);
    valid = snapshotValid;
    if (valid)
    {
      data = latestData;
    }
    portEXIT_CRITICAL(&dataMux);

    if (valid)
    {
      sendToServer(
          data.distance,
          data.obstacle,
          data.moistureRaw,
          data.moisturePercent,
          String(data.surface),
          data.riskScore,
          String(data.aiDecision),
          String(data.hazard));
    }

    vTaskDelay(pdMS_TO_TICKS(SEND_INTERVAL));
  }
}

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("====================================");
  Serial.println("        SAFESTEP SMART CANE");
  Serial.println("====================================");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);

  analogReadResolution(12);

  Serial.println("Sensors initialized.");

  xTaskCreatePinnedToCore(
      vibrationTask,
      "VibrationTask",
      4096,
      NULL,
      2,          
      NULL,
      1);

  xTaskCreatePinnedToCore(
      networkTask,
      "NetworkTask",
      12288,
      NULL,
      1,
      NULL,
      0);

  Serial.println();
  Serial.println("SafeStep system ready.");
  Serial.println("====================================");
}


void loop()
{
  static int obstacleClearCount = 0;

  if (millis() - lastSensorTime >= SENSOR_INTERVAL)
  {
    lastSensorTime = millis();


    float distance = readDistance();

    int moistureRaw = readMoisture();

    bool obstacle = distance <= OBSTACLE_DISTANCE;

    int moisturePercent =
        calculateMoisturePercent(moistureRaw);

    String surface =
        classifySurface(moistureRaw);


    int riskScore =
        calculateRiskScore(distance, surface);

    String aiDecision =
        getAIDecision(riskScore);

    String hazard =
        getHazard(obstacle, surface);

    if (obstacle)
    {
      obstacleClearCount = 0;
      sharedObstacle = true;
    }
    else
    {
      if (obstacleClearCount < OBSTACLE_CLEAR_READS)
      {
        obstacleClearCount++;
      }

      if (obstacleClearCount >= OBSTACLE_CLEAR_READS)
      {
        sharedObstacle = false;
      }
    }

    sharedSurfaceCode = surfaceToCode(surface);


    portENTER_CRITICAL(&dataMux);

    latestData.distance = distance;
    latestData.obstacle = obstacle;
    latestData.moistureRaw = moistureRaw;
    latestData.moisturePercent = moisturePercent;
    latestData.riskScore = riskScore;

    strncpy(latestData.surface, surface.c_str(), sizeof(latestData.surface) - 1);
    latestData.surface[sizeof(latestData.surface) - 1] = '\0';

    strncpy(latestData.aiDecision, aiDecision.c_str(), sizeof(latestData.aiDecision) - 1);
    latestData.aiDecision[sizeof(latestData.aiDecision) - 1] = '\0';

    strncpy(latestData.hazard, hazard.c_str(), sizeof(latestData.hazard) - 1);
    latestData.hazard[sizeof(latestData.hazard) - 1] = '\0';

    snapshotValid = true;

    portEXIT_CRITICAL(&dataMux);


    Serial.println();
    Serial.println("------------------------------------");

    Serial.print("Distance: ");
    Serial.print(distance, 2);
    Serial.println(" cm");

    Serial.print("Obstacle: ");

    if (obstacle)
    {
      Serial.println("YES");
    }
    else
    {
      Serial.println("NO");
    }

    Serial.print("Moisture Raw: ");
    Serial.println(moistureRaw);

    Serial.print("Moisture Percent: ");
    Serial.print(moisturePercent);
    Serial.println("%");

    Serial.print("Surface: ");
    Serial.println(surface);

    Serial.print("Risk Score: ");
    Serial.println(riskScore);

    Serial.print("AI Decision: ");
    Serial.println(aiDecision);

    Serial.print("Hazard: ");
    Serial.println(hazard);

    Serial.print("Vibration Pattern ID: ");
    Serial.println(activePatternId);

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi: CONNECTED");

      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("WiFi: DISCONNECTED");
    }
  }

  delay(5);
}
