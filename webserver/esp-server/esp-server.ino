#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "P519";
const char* password = "deocomatkhau";

WebServer server(80);

bool relayState = false;
bool autoMode = false; // New auto mode variable

unsigned long lastDataUpdateTime = 0;
const unsigned long dataUpdateInterval = 2000; // Update sensor data every 2 seconds

uint8_t temperature = 25;
uint8_t humidity = 60;
uint8_t soilMoisture = 45;
uint16_t lightLevel = 400;
uint8_t rainPercent = 10;

// Function to generate random sensor data
void updateSensorData() {
    // Random temperature (15-40°C)
    temperature = random(15, 41);
    
    // Random humidity (30-90%)
    humidity = random(30, 91);
    
    // Random soil moisture (10-90%)
    soilMoisture = random(10, 91);
    
    // Random light level (50-1000)
    lightLevel = random(50, 1001);
    
    // Random rain percentage (0-100%)
    rainPercent = random(0, 101);
    
    Serial.println("Updated sensor data:");
    Serial.println("Soil: " + String(soilMoisture) + "%, Temp: " + String(temperature) + 
                 "C, Humidity: " + String(humidity) + "%, Light: " + String(lightLevel) + 
                 ", Rain: " + String(rainPercent) + "%");
}

// Auto control function for water pump
void autoControlPump() {
    if (autoMode) {
        bool shouldTurnOn = (soilMoisture < 20);
        bool shouldTurnOff = (soilMoisture > 50);
        
        if (shouldTurnOn && !relayState) {
            relayState = true;
            Serial.println("Auto mode: Turning pump ON (soil moisture: " + String(soilMoisture) + "%)");
        } else if (shouldTurnOff && relayState) {
            relayState = false;
            Serial.println("Auto mode: Turning pump OFF (soil moisture: " + String(soilMoisture) + "%)");
        }
    }
}

// Enhanced HTML generation with auto mode button
String generateHTML(float temperature, float humidity, int soilMoisture, int rainPercent, int lightLevel) {
    String connectionStatus = "<div style='color: #4caf50;'>✓ System Connected</div>";
    
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
            .control-section { margin-top: 30px; }
            .switch { position: relative; display: inline-block; width: 100px; height: 50px; margin: 10px; }
            .switch input { display: none; }
            .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
                      background-color: red; transition: .4s; border-radius: 34px; }
            .slider:before { position: absolute; content: ''; height: 42px; width: 42px; left: 4px; bottom: 4px;
                           background-color: white; transition: .4s; border-radius: 50%; }
            input:checked + .slider { background-color: green; }
            input:checked + .slider:before { transform: translateX(50px); }
            .auto-slider { background-color: #2196F3; }
            input:checked + .auto-slider { background-color: #4CAF50; }
            .control-label { font-size: 18px; margin: 10px; display: block; }
            .mode-indicator { font-size: 14px; color: #aaa; margin-top: 10px; }
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
        <div class='control-section'>
            <label class="control-label">Auto Mode:</label>
            <label class="switch">
                <input type="checkbox" onchange="toggleAuto(this)" )rawliteral";

    html += autoMode ? "checked" : "";
    html += R"rawliteral(>
                <span class="slider auto-slider"></span>
            </label>
            <div class='mode-indicator'>)rawliteral";
    
    if (autoMode) {
        html += "Auto control active (ON &lt; 20%, OFF &gt; 50%)";
    } else {
        html += "Manual control";
    }
    
    html += R"rawliteral(</div>
            
            <label class="control-label">Water Pump:</label>
            <label class="switch">
                <input type="checkbox" onchange="toggleRelay(this)" )rawliteral";

    html += relayState ? "checked" : "";
    html += autoMode ? " disabled" : ""; // Disable manual control when auto mode is on
    html += R"rawliteral(>
                <span class="slider"></span>
            </label>
            <div class='mode-indicator'>)rawliteral";
    
    if (autoMode) {
        html += "Controlled by auto mode";
    } else {
        html += "Manual control enabled";
    }
    
    html += R"rawliteral(</div>
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
                if (!elem.disabled) {
                    var xhr = new XMLHttpRequest();
                    xhr.open("GET", "/relay?state=" + (elem.checked ? "on" : "off"), true);
                    xhr.send();
                }
            }
            
            function toggleAuto(elem) {
                var xhr = new XMLHttpRequest();
                xhr.open("GET", "/auto?state=" + (elem.checked ? "on" : "off"), true);
                xhr.send();
            }
        </script>
    </body></html>
    )rawliteral";

    return html;
}

void handleRoot() {
    server.send(200, "text/html", generateHTML(temperature, humidity, soilMoisture, rainPercent, lightLevel));
}

void setup() {
    Serial.begin(115200);
    
    // Initialize random seed
    randomSeed(analogRead(0));

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
    
    // Handle relay control
    server.on("/relay", []() {
        if (!autoMode && server.hasArg("state")) { // Only allow manual control when auto mode is off
            String state = server.arg("state");
            if (state == "on") {
                relayState = true;
                Serial.println("Manual: Water pump turned ON");
            } else if (state == "off") {
                relayState = false;
                Serial.println("Manual: Water pump turned OFF");
            }
        }
        server.send(200, "text/plain", relayState ? "ON" : "OFF");
    });
    
    // Handle auto mode control
    server.on("/auto", []() {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            if (state == "on") {
                autoMode = true;
                Serial.println("Auto mode enabled");
            } else if (state == "off") {
                autoMode = false;
                Serial.println("Auto mode disabled - switching to manual control");
            }
        }
        server.send(200, "text/plain", autoMode ? "AUTO_ON" : "AUTO_OFF");
    });

    server.begin();
    Serial.println("HTTP server started.");
    Serial.println("Smart Farm Dashboard ready!");
    
    // Generate initial sensor data
    updateSensorData();
}

void loop() {
    server.handleClient();
    
    unsigned long currentTime = millis();

    // Update sensor data every 2 seconds
    if (currentTime - lastDataUpdateTime >= dataUpdateInterval) {
        lastDataUpdateTime = currentTime;
        updateSensorData();
        
        // Check auto control after updating sensor data
        autoControlPump();
    }
}