#define ENABLE_DATABASE

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// ---------------- WiFi ----------------
#define WIFI_SSID "Dhanaa"
#define WIFI_PASSWORD "88880204"

// ---------------- Firebase ----------------
#define DATABASE_URL "https://esp8266-project-c5fe2-default-rtdb.asia-southeast1.firebasedatabase.app/"

// ---------------- Pins ----------------
#define LDR_PIN A0
#define LED_PIN D5

void asyncCB(AsyncResult &aResult);

NoAuth no_auth;

FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;

unsigned long lastSendMillis = 0;
const unsigned long sendInterval = 5000;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // LED Output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // WiFi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected! IP Address : ");
  Serial.println(WiFi.localIP());

  ssl_client.setInsecure();

  Serial.println("Initializing Firebase...");

  initializeApp(aClient, app, getAuth(no_auth), asyncCB, "authTask");

  app.getApp<RealtimeDatabase>(Database);

  Database.url(DATABASE_URL);

  Serial.println("Firebase Ready");
}

void loop()
{
  app.loop();
  Database.loop();

  if (app.ready() &&
      (millis() - lastSendMillis >= sendInterval || lastSendMillis == 0))
  {
    lastSendMillis = millis();

    // Read LDR
    int ldrValue = analogRead(LDR_PIN);

    Serial.print("LDR Value : ");
    Serial.println(ldrValue);

    // LED Control
    if (ldrValue < 500)
    {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("Dark -> LED ON");
    }
    else
    {
      digitalWrite(LED_PIN, LOW);
      Serial.println("Bright -> LED OFF");
    }

    // Upload LDR Value to Firebase
    Database.set<int>(aClient,
                      "/sensor/LDR_Value",
                      ldrValue,
                      asyncCB,
                      "setTask");
  }
}

void asyncCB(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Serial.printf("Event: %s\n",
                  aResult.eventLog().message().c_str());
  }

  if (aResult.isError())
  {
    Serial.printf("Error: %s\n",
                  aResult.error().message().c_str());
  }

  if (aResult.available())
  {
    Serial.printf("Firebase Response: %s\n",
                  aResult.c_str());
  }
}