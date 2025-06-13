#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int filterSize = 10; // Kích thước bộ lọc trung bình trượt
float tempReadings[filterSize];  // Mảng lưu nhiệt độ
float humReadings[filterSize];   // Mảng lưu độ ẩm
int currentIndex = 0;            // <<--- sửa tên biến này
bool bufferFilled = false;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT22 Filtered Readings"));

  dht.begin();

  for (int i = 0; i < filterSize; i++) {
    tempReadings[i] = 0.0;
    humReadings[i] = 0.0;
  }
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  tempReadings[currentIndex] = t;
  humReadings[currentIndex] = h;

  currentIndex++;
  if (currentIndex >= filterSize) {
    currentIndex = 0;
    bufferFilled = true;
  }

  int count = bufferFilled ? filterSize : currentIndex;
  float sumT = 0.0, sumH = 0.0;
  for (int i = 0; i < count; i++) {
    sumT += tempReadings[i];
    sumH += humReadings[i];
  }

  float filteredT = sumT / count;
  float filteredH = sumH / count;

  Serial.print(F("Raw Temp: "));
  Serial.print(t);
  Serial.print(F("°C -> Filtered Temp: "));
  Serial.print(filteredT);
  Serial.print(F("°C, Raw Hum: "));
  Serial.print(h);
  Serial.print(F("% -> Filtered Hum: "));
  Serial.print(filteredH);
  Serial.println(F("%"));
}
