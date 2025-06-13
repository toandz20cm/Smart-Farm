#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

const char* ssid = "P519";
const char* password = "deocomatkhau";

WebServer server(80);

const char* thingSpeakAPIKey = "U08SDHAU7OJX8DLU";

bool relayState = false;

unsigned long lastWebUpdateTime = 0;
unsigned long lastThingSpeakSendTime = 0;
unsigned long lastDataReceivedTime = 0;
const unsigned long webUpdateInterval = 5000;
const unsigned long thingSpeakInterval = 15000;
const unsigned long dataTimeoutInterval = 10000; // 10 seconds timeout for STM32 data

uint8_t temperature = 0;
uint8_t humidity = 0;
uint8_t soilMoisture = 0;
uint16_t lightLevel = 0;
uint8_t rainPercent = 0;

// Enhanced UART communication variables
String receivedData = "";
bool dataReceived = false;
bool stm32Connected = false;
int parseErrorCount = 0;

// Enhanced UART data parsing with better error handling
void parseUARTData(String data) {
    data.trim(); // Remove whitespace and line endings
    
    // Skip debug messages and error messages
    if (data.startsWith("DEBUG:") || data.startsWith("DHT11_ERR:") || 
        data.startsWith("STM32") || data.length() < 5) {
        Serial.println("Skipping: " + data);
        return;
    }
    
    Serial.println("Parsing: '" + data + "'");
    
    // Count commas to validate format
    int commaCount = 0;
    for (int i = 0; i < data.length(); i++) {
        if (data.charAt(i) == ',') commaCount++;
    }
    
    if (commaCount != 4) {
        parseErrorCount++;
        Serial.println("Parse error: Expected 4 commas, found " + String(commaCount));
        if (parseErrorCount > 5) {
            Serial.println("Too many parse errors, resetting...");
            parseErrorCount = 0;
        }
        return;
    }
    
    // Parse data format: "moisture,temp,humidity,light,rain"
    int commaIndex1 = data.indexOf(',');
    int commaIndex2 = data.indexOf(',', commaIndex1 + 1);
    int commaIndex3 = data.indexOf(',', commaIndex2 + 1);
    int commaIndex4 = data.indexOf(',', commaIndex3 + 1);
    
    if (commaIndex1 != -1 && commaIndex2 != -1 && commaIndex3 != -1 && commaIndex4 != -1) {
        // Parse each value with error checking
        String moistureStr = data.substring(0, commaIndex1);
        String tempStr = data.substring(commaIndex1 + 1, commaIndex2);
        String humidityStr = data.substring(commaIndex2 + 1, commaIndex3);
        String lightStr = data.substring(commaIndex3 + 1, commaIndex4);
        String rainStr = data.substring(commaIndex4 + 1);
        
        // Convert and validate ranges
        int newMoisture = moistureStr.toInt();
        int newTemp = tempStr.toInt();
        int newHumidity = humidityStr.toInt();
        int newLight = lightStr.toInt();
        int newRain = rainStr.toInt();
        
        // Validate ranges (basic sanity check)
        if (newMoisture >= 0 && newMoisture <= 100 &&
            newTemp >= 0 && newTemp <= 60 &&
            newHumidity >= 0 && newHumidity <= 100 &&
            newLight >= 0 && newLight <= 7000 &&
            newRain >= 0 && newRain <= 100) {
            
            soilMoisture = newMoisture;
            temperature = newTemp;
            humidity = newHumidity;
            lightLevel = newLight;
            rainPercent = newRain;
            
            lastDataReceivedTime = millis();
            stm32Connected = true;
            parseErrorCount = 0;
            
            Serial.println("Successfully parsed data:");
            Serial.println("Soil: " + String(soilMoisture) + "%, Temp: " + String(temperature) + 
                         "C, Humidity: " + String(humidity) + "%, Light: " + String(lightLevel) + 
                         ", Rain: " + String(rainPercent) + "%");
        } else {
            Serial.println("Data validation failed - values out of range");
            parseErrorCount++;
        }
    } else {
        Serial.println("Failed to find all comma separators");
        parseErrorCount++;
    }
}

void readUARTData() {
    while (Serial2.available()) {
        char c = Serial2.read();
        
        if (c == '\n') {
            if (receivedData.length() > 0) {
                parseUARTData(receivedData);
                receivedData = "";
                dataReceived = true;
            }
        } else if (c != '\r') { // Ignore carriage return
            receivedData += c;
            
            // Prevent buffer overflow
            if (receivedData.length() > 200) {
                Serial.println("Buffer overflow, clearing...");
                receivedData = "";
            }
        }
    }
    
    // Check for STM32 connection timeout
    if (stm32Connected && (millis() - lastDataReceivedTime > dataTimeoutInterval)) {
        stm32Connected = false;
        Serial.println("STM32 connection timeout!");
    }
}

void sendRelayCommand(int state) {
    Serial2.print(String(state));
    Serial2.flush(); // Ensure data is sent
    Serial.println("Sent relay command to STM32: " + String(state));
    
    // Wait a bit for response
    delay(100);
}

// Enhanced HTML generation with connection status
String generateHTML(float temperature, float humidity, int soilMoisture, int rainPercent, int lightLevel) {
    String connectionStatus = stm32Connected ? 
        "<div style='color: #4caf50;'>✓ STM32 Connected</div>" : 
        "<div style='color: #f44336;'>✗ STM32 Disconnected</div>";
    
    String tempStatus = (temperature > 30) ? "High" : (temperature < 20 ? "Low" : "Optimal");
    String tempColor = (temperature > 30) ? "status-bad" : (temperature < 20 ? "status-medium" : "status-good");

    String humStatus = (humidity > 80) ? "Too high" : (humidity < 40 ? "Too low" : "Optimal");
    String humColor = (humidity > 80) ? "status-bad" : (humidity < 40 ? "status-medium" : "status-good");

    String soilStatus = (soilMoisture < 20) ? "Too dry" : (soilMoisture > 80 ? "Too wet" : "Good");
    String soilColor = (soilMoisture < 40) ? "status-bad" : (soilMoisture > 60 ? "status-medium" : "status-good");

    String rainIcon = (rainPercent > 30) ? "fas fa-cloud-showers-heavy" : "fas fa-cloud-sun";
    String rainStatus = (rainPercent > 70) ? "Heavy rain" : (rainPercent > 30 ? "Light rain" : "No rain");
    String rainColor = (rainPercent > 70) ? "status-bad" : (rainPercent > 30 ? "status-medium" : "status-good");

    String lightStatus = (lightLevel < 200) ? "Low light" : (lightLevel > 800 ? "Bright" : "Ideal");
    String lightColor = (lightLevel < 200) ? "status-medium" : (lightLevel > 800 ? "status-medium" : "status-good");

    String html = R"rawliteral(
    <html><head>
        <meta http-equiv='refresh' content='5'/>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css'>
        <title>Smart Farm Dashboard</title>
        <style>
            body { background: #121212; color: #fff; font-family: Arial; text-align: center; padding: 20px; }
            .connection-status { margin-bottom: 20px; font-size: 16px; }
            .dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }
            .card { background: #1e1e1e; border-radius: 15px; padding: 20px; box-shadow: 0 10px 20px rgba(0,0,0,0.3); }
            .card-header { height: 6px; border-radius: 15px 15px 0 0; }
            .temperature .card-header { background: linear-gradient(90deg, #ff5252, #ff7878); }
            .humidity .card-header { background: linear-gradient(90deg, #64b5f6, #90caf9); }
            .soil .card-header { background: linear-gradient(90deg, #81c784, #a5d6a7); }
            .weather .card-header { background: linear-gradient(90deg, #4dd0e1, #80deea); }
            .light .card-header { background: linear-gradient(90deg, #ffd54f, #ffe082); }
            .icon { font-size: 40px; margin: 20px 0; }
            .value { font-size: 34px; font-weight: bold; }
            .label { font-size: 14px; color: #aaa; }
            .status { margin-top: 15px; display: flex; justify-content: center; align-items: center; gap: 8px; }
            .status-dot { width: 12px; height: 12px; border-radius: 50%; }
            .status-good { background-color: #4caf50; }
            .status-medium { background-color: #ff9800; }
            .status-bad { background-color: #f44336; }
            .last-update { margin-top: 20px; font-size: 14px; color: #aaa; }
            .switch { position: relative; display: inline-block; width: 100px; height: 50px; margin-top: 30px; }
            .switch input { display: none; }
            .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
                      background-color: red; transition: .4s; border-radius: 34px; }
            .slider:before { position: absolute; content: ''; height: 42px; width: 42px; left: 4px; bottom: 4px;
                           background-color: white; transition: .4s; border-radius: 50%; }
            input:checked + .slider { background-color: green; }
            input:checked + .slider:before { transform: translateX(50px); }
        </style>
    </head>
    <body>
        <h1>SMART FARM MONITOR</h1>
        <div class='connection-status'>)rawliteral";

    html += connectionStatus;
    html += R"rawliteral(</div>
        <div class='dashboard'>)rawliteral";

    html += "<div class='card temperature'><div class='card-header'></div><div class='icon'><i class='fas fa-thermometer-half'></i></div><div class='value'>" + String(temperature) + " &deg;C</div><div class='label'>Temperature</div><div class='status'><div class='status-dot " + tempColor + "'></div><div>" + tempStatus + "</div></div></div>";
    html += "<div class='card humidity'><div class='card-header'></div><div class='icon'><i class='fas fa-tint'></i></div><div class='value'>" + String(humidity) + "%</div><div class='label'>Humidity</div><div class='status'><div class='status-dot " + humColor + "'></div><div>" + humStatus + "</div></div></div>";
    html += "<div class='card soil'><div class='card-header'></div><div class='icon'><i class='fas fa-seedling'></i></div><div class='value'>" + String(soilMoisture) + "%</div><div class='label'>Soil Moisture</div><div class='status'><div class='status-dot " + soilColor + "'></div><div>" + soilStatus + "</div></div></div>";
    html += "<div class='card weather'><div class='card-header'></div><div class='icon'><i class='" + rainIcon + "'></i></div><div class='value'>" + String(rainPercent) + "%</div><div class='label'>Rain</div><div class='status'><div class='status-dot " + rainColor + "'></div><div>" + rainStatus + "</div></div></div>";
    html += "<div class='card light'><div class='card-header'></div><div class='icon'><i class='fas fa-sun'></i></div><div class='value'>" + String(lightLevel) + "</div><div class='label'>Light Level</div><div class='status'><div class='status-dot " + lightColor + "'></div><div>" + lightStatus + "</div></div></div>";

    html += R"rawliteral(
        </div>
        <div>
            <label style="font-size: 18px;">Water Pump:</label><br>
            <label class="switch">
                <input type="checkbox" onchange="toggleRelay(this)" )rawliteral";

    html += relayState ? "checked" : "";
    html += R"rawliteral(>
                <span class="slider"></span>
            </label>
        </div>
        <div class='last-update'>Last updated: <span id='timestamp'></span></div>
        <script>
            function updateTime() {
                const now = new Date();
                document.getElementById('timestamp').textContent = now.toLocaleTimeString();
            }
            updateTime();
            setInterval(updateTime, 1000);

            function toggleRelay(elem) {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/relay?state=" + (elem.checked ? "on" : "off"), true);
                xhr.send();
            }
        </script>
    </body></html>
    )rawliteral";

    return html;
}

void sendDataToThingSpeak() {
    if (WiFi.status() == WL_CONNECTED && stm32Connected) {
        HTTPClient http;
        String url = "http://api.thingspeak.com/update?api_key=" + String(thingSpeakAPIKey) +
                     "&field1=" + String(temperature) +
                     "&field2=" + String(humidity) +
                     "&field3=" + String(soilMoisture) +
                     "&field4=" + String(rainPercent) +
                     "&field5=" + String(lightLevel);

        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("Data sent to ThingSpeak. Response: " + String(httpResponseCode));
        } else {
            Serial.println("ThingSpeak error: " + String(httpResponseCode));
        }
        http.end();
    } else {
        Serial.println("Cannot send to ThingSpeak - WiFi or STM32 not connected");
    }
}

void handleRoot() {
    server.send(200, "text/html", generateHTML(temperature, humidity, soilMoisture, rainPercent, lightLevel));
}

void setup() {
    Serial.begin(115200);   // Higher baud for debugging
    Serial2.begin(9600, SERIAL_8N1, 16, 17);  // UART2 for STM32 communication

    Serial.println("ESP32 Smart Farm System Starting...");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/relay", []() {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (state == "on") {
                relayState = true;
                sendRelayCommand(1);
            } else if (state == "off") {
                relayState = false;
                sendRelayCommand(0);
            }
        }
        server.send(200, "text/plain", relayState ? "ON" : "OFF");
    });

    server.begin();
    Serial.println("HTTP server started.");
    Serial.println("Waiting for UART data from STM32...");
    
    // Send a test command to STM32
    delay(2000);
    Serial2.println("?"); // Query pump status
}

void loop() {
    server.handleClient();
    readUARTData();
    
    unsigned long currentTime = millis();

    // Send data to ThingSpeak every 15 seconds if we have valid data
    if (currentTime - lastThingSpeakSendTime >= thingSpeakInterval && dataReceived && stm32Connected) {
        lastThingSpeakSendTime = currentTime;
        sendDataToThingSpeak();
    }
    
    // Print connection status periodically
    static unsigned long lastStatusPrint = 0;
    if (currentTime - lastStatusPrint >= 10000) { // Every 10 seconds
        lastStatusPrint = currentTime;
        Serial.println("Status - STM32: " + String(stm32Connected ? "Connected" : "Disconnected") + 
                      ", Parse Errors: " + String(parseErrorCount));
    }
}