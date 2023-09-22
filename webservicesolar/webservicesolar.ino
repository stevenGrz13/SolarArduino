#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ACS712.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define FIREBASE_HOST "solar-smart-project.firebaseio.com"
#define FIREBASE_AUTH "CSEfOOgLf3qM1mOTcrGEOEepuXRCKSvOCNiuXawy"
#define API_KEY "AIzaSyAJO70Mv08WdVrhUulZOnSRWpPHZodTs7s"
#define DATABASE_URL "https://solar-smart-project-default-rtdb.firebaseio.com/"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define WIFI_SSID "NextA_Incubation"
#define WIFI_PASSWORD "1cuB3r!N3xt@2023"

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;

const int numSamples = 100; // Réduisez le nombre d'échantillons
int samples[numSamples];

// Supprimez les variables globales non utilisées
float VOLTAGE_SCALE = 7.617;
float CURRENT_SCALE = 1.5;
const int ACS712_PIN = 35;
const int voltagePin = 34;
const float ACS712_SENSITIVITY = 0.185;
const float pourcentage_last_value = 0.1;
float pourcentage_value = 0;
unsigned long current_time = 0;
float energy = 0;
float tempC = 0;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP:");
  Serial.println(WiFi.localIP());
  Serial.println();
  sensors.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Erreur lors de l'initialisation de l'écran OLED"));
    while (true);
  }

    firebaseConfig.api_key = API_KEY;
  firebaseConfig.database_url = DATABASE_URL;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, nullptr);
}

void loop() {
  int sensorValue = analogRead(ACS712_PIN);
  float milliVolts = (sensorValue / 4095.0) * 3.3;
  float current = ((-milliVolts + 1.65) / ACS712_SENSITIVITY) / 10;
  float Voltage = 0;
  for (int i = 0; i < numSamples; i++) {
    Voltage = analogRead(voltagePin);
  }
  Serial.println("voltage=");
  Serial.print(Voltage);
  sensors.requestTemperatures();
  tempC = sensors.getTempCByIndex(0);
  float sum = 0.0;
  for (int i = 0; i < numSamples; i++) {
    sum += samples[i];
  }
  Voltage = (sum / numSamples) * (3.3 / 4095.0) / 0.019;
  float puissance = current * Voltage;
  unsigned long last_time = current_time;
  current_time = millis();
  energy = energy + puissance * ((current_time - last_time) / 3600000.0);
  if (Voltage > 0.2) {
    Voltage += 10;
    pourcentage_value += pourcentage_last_value;
    Serial.print("Voltage: ");
    Serial.print(Voltage * pourcentage_value, 1);
    Serial.println(" V");
  }
  if (sensorValue == 0) {
    Serial.print("Current:");
    Serial.print(0);
    Serial.print(" A");
  } else {
    Serial.print("Current: ");
    Serial.print(current, 2);
    Serial.println(" A");
  }
  if (energy >= 1000) {
    Serial.print(energy / 1000, 3);
    Serial.println(" kWh");
  } else {
    Serial.print(energy, 1);
    Serial.println(" Wh");
  }

  // Appel de l'API HTTP
  HTTPClient http;
  http.begin("https://versionfinalwebservicesolar-production-aec0.up.railway.app/api/solar/insertdonnee/1234/4567/8910/1112"); // Remplacez par votre URL
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("Response: ");
    Serial.println(response);
  } else {
    Serial.print("HTTP Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

   srand(time(NULL));

    int lower_bound = 1;  // The lower bound of the range
    int upper_bound = 3; // The upper bound of the range

    // Generate a random number between lower_bound and upper_bound
    int zavatra = (rand() % (upper_bound - lower_bound + 1)) + lower_bound;

  if(zavatra==3){
    if(Firebase.RTDB.setFloat(&firebaseData, "Notification/Energie", zavatra)){
        Serial.println("tafiditra anaty firebase");
      }
      else{
      Serial.println("Erreur Firebase=");
      Serial.print(firebaseData.errorReason());
      }
  }
    delay(5000);
}