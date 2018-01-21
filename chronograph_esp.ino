#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define RESULTS_NUM 128
#define CYCLETIME 12.5 // ns
//#define CYCLES_MICRO (1000 / CYCLETIME)

// wifi params of network to connect
const char* ssid     = "<ssid>";
const char* password = "<pass>";

const int led = 14;
const int gate1pin = 12;
const int gate2pin = 13;

ESP8266WebServer server(80);
String webString = "";   // String to display
unsigned int idx = 0;
float results[RESULTS_NUM];

unsigned long time1 = 0;
unsigned long time2 = 0;
float spd = 0;
float secs = 0;
int gate1state = 0;
int gate2state = 0;

void gate1(){
    time1 = ESP.getCycleCount();
    Serial.println("gate1");
    gate2state = 0;
    gate1state = 1;
}

void gate2(){
    time2 = ESP.getCycleCount();
    gate2state = 1;
}
void handle_root(){
  // returns all current results
  webString = "<html><head><title>Speed'o'meter by f0x</title></head><body>";
  webString += "<h3>chronograph esp v1</h3><br>";
  webString += "times in sec:<br>";
  for (int i = 0; i < idx; i++ ){
    webString += String(results[i], 3);
    webString += ", ";
  }
  webString += "<br>speed in m/s:<br>";
  for (int i = 0; i < idx; i++ ){
    webString += String(i);
    webString += " ";
    webString += String(0.2/results[i], 3);
    webString += "<br>";
  }
  webString += "<br><br><a href=/reset>Reset<br>";
  webString += "</body></html>";
  server.send(200, "text/html", webString);  
}
void handle_reset(){
  //memset(results, 0, RESULTS_NUM*sizeof(float));
  idx = 0;
  webString = "Resetted";
  server.send(200, "text/html", webString);
}

void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(gate1pin, INPUT);
  pinMode(gate2pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(gate1pin), gate1, FALLING);
  attachInterrupt(digitalPinToInterrupt(gate2pin), gate2, FALLING);
  Serial.begin(9600);
  Serial.println("Speed'o'meter by f0x");
  Serial.print("Frequency: ");
  Serial.println(ESP.getCpuFreqMHz());
  

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handle_root);
  server.on("/reset", handle_reset);
  server.begin();
  Serial.println("HTTP server started");
}



void loop() {
  server.handleClient();
  if ( gate1state == 1 && gate2state == 1) {
    //secs = (time2-time1)/1000000.00;
    secs = ((time2-time1) * CYCLETIME)/(1000 * 1000000.00);
    spd = 0.2/secs;
    
    Serial.print(spd);
    Serial.print(",");
    Serial.print(secs);
    Serial.print(",");
    Serial.print(time1);
    Serial.print(",");
    Serial.println(time2);

    results[idx] = secs;
    idx++;
    if (idx == RESULTS_NUM) idx = 0;
    //memset(results, 0, RESULTS_NUM*sizeof(float));
    
    time1 = 0;
    time2 = 0;
    spd = 0;
    gate1state = 0;
    gate2state = 0;
    
  }
}
