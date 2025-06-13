#include <SimpleDHT.h>
#include "Kalman_Filter.h"

// KF filter with temperature and humidity sensor DHT11 for GUI in Qt

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

  byte temperature = 0;
  byte humidity = 0;

  if (DHT_sensor.read(DHT_pin, &temperature, &humidity, NULL))
  {
    Serial.println("Failed reading sensor...");
    return;
  }
  // KF with temperature value
  Kalman_result_temp = KF_temp.update(temperature);

  Serial.print((float)temperature);
  Serial.print("/");
  
  Serial.print(Kalman_result_temp);
  Serial.print("/");

  // KF with humidity value
  Kalman_result_hum = KF_hum.update(humidity);
  Serial.print((float)humidity);
  Serial.print("/");

  Serial.print(Kalman_result_hum);
  Serial.println("/");
  Serial.flush();

  // Frequency of generate info 1.5 Hz
  delay(1500); // delay of program
}
