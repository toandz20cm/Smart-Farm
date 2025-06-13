#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "P519";
const char* password = "deocomatkhau";

WebServer server(80);

bool relayState = false;

unsigned long lastWebUpdateTime = 0;
const unsigned long webUpdateInterval = 5000;

// Dữ liệu mẫu thay vì nhận từ UART
float temperatureData[10] = {22.5, 24.0, 26.3, 28.1, 30.5, 31.2, 29.8, 25.5, 23.3, 27.0};  
float humidityData[10]    = {45.2, 50.3, 55.1, 60.6, 65.0, 70.5, 75.2, 80.0, 78.5, 72.1};
int soilData[10]          = {320, 340, 360, 380, 420, 460, 500, 600, 700, 750};
int lightData[10]         = {150, 200, 300, 400, 500, 600, 700, 800, 900, 950};
bool rainData[10]         = {0, 0, 1, 0, 1, 1, 0, 0, 0, 1};

int dataIndex = 0; // để duyệt qua các phần tử của mảng

// HTML hiển thị
String generateHTML(float temperature, float humidity, int soilMoisture, int rainPercent, int lightLevel) {
    String html = R"rawliteral(
    <html><head>
        <meta http-equiv='refresh' content='5'/>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <title>Smart Farm Dashboard</title>
        <style>
            body { background: #121212; color: #fff; font-family: Arial; text-align: center; padding: 20px; }
            .dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; }
            .card { background: #1e1e1e; border-radius: 15px; padding: 20px; }
            .value { font-size: 30px; font-weight: bold; margin-top: 10px; }
        </style>
    </head>
    <body>
        <h1>SMART FARM MONITOR</h1>
        <div class='dashboard'>
    )rawliteral";

    html += "<div class='card'><h3>Temperature</h3><div class='value'>" + String(temperature) + " &deg;C</div></div>";
    html += "<div class='card'><h3>Humidity</h3><div class='value'>" + String(humidity) + " %</div></div>";
    html += "<div class='card'><h3>Soil Moisture</h3><div class='value'>" + String(soilMoisture) + "</div></div>";
    html += "<div class='card'><h3>Light Level</h3><div class='value'>" + String(lightLevel) + "</div></div>";
    html += "<div class='card'><h3>Rain</h3><div class='value'>" + String(rainPercent) + " %</div></div>";

    html += R"rawliteral(
        </div>
        <div style="margin-top:30px;">
            <label style="font-size: 18px;">Water Pump:</label><br>
            <label class="switch">
                <input type="checkbox" onchange="toggleRelay(this)" )rawliteral";

    html += relayState ? "checked" : "";
    html += R"rawliteral(>
                <span class="slider"></span>
            </label>
        </div>
        <script>
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

void handleRoot() {
    float temp = temperatureData[dataIndex];
    float hum = humidityData[dataIndex];
    int soil = soilData[dataIndex];
    int light = lightData[dataIndex];
    int rain = rainData[dataIndex] ? 100 : 0;

    server.send(200, "text/html", generateHTML(temp, hum, soil, rain, light));
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Smart Farm with Array Data");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/relay", []() {
        if (server.hasArg("state")) {
            String state = server.arg("state");
            relayState = (state == "on");
        }
        server.send(200, "text/plain", relayState ? "ON" : "OFF");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();

    unsigned long currentTime = millis();
    if (currentTime - lastWebUpdateTime >= webUpdateInterval) {
        lastWebUpdateTime = currentTime;

        dataIndex++;
        if (dataIndex >= 10) dataIndex = 0;

        Serial.println("Updated to next sensor reading: Index " + String(dataIndex));
    }
}
