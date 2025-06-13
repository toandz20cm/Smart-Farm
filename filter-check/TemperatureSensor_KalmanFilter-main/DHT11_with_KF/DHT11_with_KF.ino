#include <SimpleDHT.h>
#include "Kalman_Filter.h"

// KF filter with temperature and humidity sensor DHT11

//
int DHT_pin = A2;
double Kalman_result_temp = 0.;
double Kalman_result_hum = 0.;

SimpleDHT11 DHT_sensor;
KalmanFilter KF_temp;
KalmanFilter KF_hum;


void setup()
{
  Serial.begin(115200);
}


void loop()
{
  for (int i = 0; i < 80; i++)
    Serial.print("*");
  Serial.println("");
  Serial.println("Start sampling sensor...");
  Serial.println("");
  byte temperature = 0;
  byte humidity = 0;

  if (DHT_sensor.read(DHT_pin, &temperature, &humidity, NULL))
  {
    Serial.println("Failed reading sensor...");
    return;
  }
  // KF with temperature value
  Kalman_result_temp = KF_temp.update(temperature);
  Serial.print("Temperature read by sensor: ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Value of temperature determined by the Kalman Filter: ");
  Serial.print(Kalman_result_temp);
  Serial.println(":");
  
  Serial.println("");
  // KF with humidity value
  Serial.print("Humidity read by sensor: ");
  Kalman_result_hum = KF_hum.update(humidity);
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Value of humidity determined by the Kalman Filter: ");
  Serial.print(Kalman_result_hum);
  Serial.println(":");
  Serial.println("");
  
  // Frequency of generate info 1.5 Hz
  delay(1500); // delay of program
}
