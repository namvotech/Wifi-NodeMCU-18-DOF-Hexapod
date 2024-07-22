/* -----------------------------------------------------------------------------
  - Original Project;Remote control Crawling hexapod robot
  - Author;mwilmars
  - Date;2018/10/07
  
   -----------------------------------------------------------------------------
  - Overview
  - This is web sever controller for hexapod spider robot
  - Just a little test message.  Go to http://192.168.4.1 in a web browser connected to this access point to see it.
/* Includes ------------------------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266mDNS.h>
/*Set for Wifi PA config*/
#define SET_IP_PA
#ifdef SET_IP_PA 
IPAddress local_IP(192,168,2,4);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);
#endif
/* Set these to your desired credentials. */
const char *ssid = "PET-HQN"; const char *password = "13572468hqn";
String CommOut=""; String CommIn=""; String lastComm = "";
int StepSpeed = 50; int lastSpeed = 50;
int SMov[32]={1440,1440,1440,0, 1440,1440,1440,0,   0,0,0,0, 1440,1440,1440,0, 1440,1440,1440,0, 0,0,0,0,  1440,1440,1440,0, 1440,1440,1440,0};
int SAdj[32]={0,0,0,0, 0,0,0,0,  1000,0,0,0, 0,0,0,0, 0,0,0,0, 1440,900,2100,900,  0,0,0,0, 0,0,0,0};
int StaBlink=0; int IntBlink=0;

MDNSResponder mdns;
ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//FIXNAM
int ClawPos=500; //Pin 9
int xoay_pos = 1440; //Pin 21
int post_arm_1=900; // Pin 22
int post_arm_2=2100; //Pin 23
int post_arm_3=900; //Pin 24
int val_step =200;

//LED
int ledState = LOW;
unsigned long previousMillis = 0;
bool kt=false;
//#define DEBUG
//================================================================================= HTML =======================================================================
/*
 *ESP8266 Spider Hexapod = PET-HQN Spider Hexapod
 *Quadruped Controller = PET-HQN Controller
*/
//static const char PROGMEM INDEX_HTML[] = R"rawliteral(
//<!DOCTYPE html><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>PET-HQN Spider Hexapod</title><style>"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
//#JD {text-align: center;}#JD {text-align: center;font-family: "Lucida Sans Unicode", "Lucida Grande", sans-serif;font-size: 24px;}.foot {text-align: center;font-family: "Comic Sans MS", cursive;font-size: 9px;color: #F00;}
//.button {border: none;color: white;padding: 20px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;margin: 4px 2px;cursor: pointer;border-radius: 12px;width: 100%;}.red {background-color: #F00;}.green {background-color: #090;}.yellow {background-color:#F90;}.blue {background-color:#03C;}</style>
//<script>var websock;function start() {websock = new WebSocket('ws://' + window.location.hostname + ':81/');websock.onopen = function(evt) { console.log('websock open'); };websock.onclose = function(evt) { console.log('websock close'); };websock.onerror = function(evt) { console.log(evt); }; 
//websock.onmessage = function(evt) {console.log(evt);var e = document.getElementById('ledstatus');if (evt.data === 'ledon') { e.style.color = 'red';}else if (evt.data === 'ledoff') {e.style.color = 'black';} else {console.log('unknown event');}};} function buttonclick(e) {websock.send(e.id);}</script>
//</head><body onload="javascript:start();">&nbsp;<table width="100%" border="1"><tr><td bgcolor="#FFFF33" id="JD">PET-HQN Controller</td></tr></table>
//<table width="100" height="249" border="0" align="center">
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 2" type="button" onclick="buttonclick(this);" class="button red">Forward_Stop_Left</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 1" type="button" onclick="buttonclick(this);" class="button green">Forward</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 3"  type="button" onclick="buttonclick(this);" class="button red">Forward_Stop_Right</button> </label></form></td></tr>
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 3 1" type="button" onclick="buttonclick(this);" class="button green">Turn_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 1"  type="button" onclick="buttonclick(this);" class="button red">Stop_all</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 4 1"  type="button" onclick="buttonclick(this);" class="button green">Turn_Right</button></label></form></td></tr>
//<tr><td>&nbsp;</td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 1" type="button" onclick="buttonclick(this);" class="button green">Backward</button></label></form></td><td>&nbsp;</td></tr>
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 5 3" type="button" onclick="buttonclick(this);" class="button yellow">Shake </button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 8 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_up</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 6 3"  type="button" onclick="buttonclick(this);" class="button yellow">Wave</button></label></form></td></tr>
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 16" type="button" onclick="buttonclick(this);" class="button blue">Twist_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 9 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_down</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 17"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Right</button> </label></form></td></tr>
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 11 5" type="button" onclick="buttonclick(this);" class="button blue">Body_left</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 13"  type="button" onclick="buttonclick(this);" class="button blue">Body_higher</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 10 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_right</button></label></form></td></tr>
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 12" type="button" onclick="buttonclick(this);" class="button yellow">Service</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 14"  type="button" onclick="buttonclick(this);" class="button blue">Body_lower</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 15"  type="button" onclick="buttonclick(this);" class="button yellow">Reset_Pose</button> </label></form></td></tr>
//
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td>     <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 4"  type="button" onclick="buttonclick(this);" class="button green">Normal</button> </label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button yellow">Run</button> </label></form></td>           <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 5"  type="button" onclick="buttonclick(this);" class="button green">Fast</button> </label></form></td></tr>
//
//
//</table><p class="foot">this application requires Mwilmar Quadruped platform.</p></body></html>
//)rawliteral";

////
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td>     <td align="center" valign="middle"><form name="form1" method="post" action="">&nbsp;</form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button yellow">Run</button> </label></form></td></tr>

/////////////////////////////////////////////////////////////
//FIXNAM
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>PET-HQN Spider Hexapod</title><style>"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
#JD {text-align: center;}#JD {text-align: center;font-family: "Lucida Sans Unicode", "Lucida Grande", sans-serif;font-size: 24px;}.foot {text-align: center;font-family: "Comic Sans MS", cursive;font-size: 9px;color: #F00;}
.button {border: none;color: white;padding: 20px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;margin: 4px 2px;cursor: pointer;border-radius: 12px;width: 100%;}.red {background-color: #F00;}.green {background-color: #090;}.yellow {background-color:#F90;}.blue {background-color:#03C;} .pink{background-color:#C600D8;} .cyan{background-color:#58F6FB;} </style>
<script>var websock;function start() {websock = new WebSocket('ws://' + window.location.hostname + ':81/');websock.onopen = function(evt) { console.log('websock open'); };websock.onclose = function(evt) { console.log('websock close'); };websock.onerror = function(evt) { console.log(evt); }; 
websock.onmessage = function(evt) {console.log(evt);var e = document.getElementById('ledstatus');if (evt.data === 'ledon') { e.style.color = 'red';}else if (evt.data === 'ledoff') {e.style.color = 'black';} else {console.log('unknown event');}};} function buttonclick(e) {websock.send(e.id);}</script></head>
<body onload="javascript:start();">&nbsp;

<table width="100%" height="100%" border="0" align="center">
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 5 3" type="button" onclick="buttonclick(this);" class="button pink">Shake </button></label></form></td>          <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 8 5"  type="button" onclick="buttonclick(this);" class="button blue">Head Up</button></label></form></td>            <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 6 3"  type="button" onclick="buttonclick(this);" class="button pink">Wave</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 14"  type="button" onclick="buttonclick(this);" class="button blue">Bod Low</button> </label></form></td>       <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 9 5"  type="button" onclick="buttonclick(this);" class="button blue">Head Dn</button> </label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 13"  type="button" onclick="buttonclick(this);" class="button blue">Bod High</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 12"  type="button" onclick="buttonclick(this);" class="button yellow">Service</button> </label></form></td>        <td>&nbsp;</td>                                                   <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 15"  type="button" onclick="buttonclick(this);" class="button yellow">RESET</button> </label></form></td></tr>

<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 2" type="button" onclick="buttonclick(this);" class="button red">4 chan</button> </label></form></td>               <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 1" type="button" onclick="buttonclick(this);" class="button green">Forward</button></label></form></td>          <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 3"  type="button" onclick="buttonclick(this);" class="button red">2 chan</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 3 1" type="button" onclick="buttonclick(this);" class="button green">Turn Left</button></label></form></td>        <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 1"  type="button" onclick="buttonclick(this);" class="button red">STOP ALL</button> </label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 4 1"  type="button" onclick="buttonclick(this);" class="button green">Turn Righ</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 20"  type="button" onclick="buttonclick(this);" class="button pink">Close</button> </label></form></td>             <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 1" type="button" onclick="buttonclick(this);" class="button green">Backward</button></label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 21"   type="button" onclick="buttonclick(this);" class="button pink">Open</button> </label></form></td></tr>

<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 35" type="button" onclick="buttonclick(this);" class="button yellow">Arm Low </button></label></form></td>          <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 34" type="button" onclick="buttonclick(this);" class="button yellow">Arm Nor </button></label></form></td>       <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 33" type="button" onclick="buttonclick(this);" class="button yellow">Arm End </button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 23" type="button" onclick="buttonclick(this);" class="button blue">X Trai </button></label></form></td>          <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 36" type="button" onclick="buttonclick(this);" class="button yellow">Arm Long </button></label></form></td>     <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 22"  type="button" onclick="buttonclick(this);" class="button blue">X Phai</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 24" type="button" onclick="buttonclick(this);" class="button blue">Arm 1 Up </button></label></form></td>        <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 26" type="button" onclick="buttonclick(this);" class="button blue">Arm 2 Up </button></label></form></td>       <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 28"  type="button" onclick="buttonclick(this);" class="button blue">Arm 3 Up</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 25" type="button" onclick="buttonclick(this);" class="button blue">Arm 1 Dn </button></label></form></td>        <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 27" type="button" onclick="buttonclick(this);" class="button blue">Arm 2 Dn </button></label></form></td>       <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 29"  type="button" onclick="buttonclick(this);" class="button blue">Arm 3 Dn</button></label></form></td></tr>

<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 4"  type="button" onclick="buttonclick(this);" class="button green">Normal</button> </label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button green">Run</button> </label></form></td>           <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 5"  type="button" onclick="buttonclick(this);" class="button green">Fast</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 30" type="button" onclick="buttonclick(this);" class="button yellow">Val 50 </button></label></form></td>          <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 31" type="button" onclick="buttonclick(this);" class="button yellow">Val 100 </button></label></form></td>       <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 32" type="button" onclick="buttonclick(this);" class="button yellow">Val 200 </button></label></form></td></tr>

</table>

<table width="100%" border="1"><tr><td bgcolor="#FFFF33" id="JD">PET-HQN Controller</td></tr></table>
<p class="foot">this application requires Mwilmar Quadruped platform.</p></body></html>

)rawliteral";

//Twist_Left//  <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 16" type="button" onclick="buttonclick(this);" class="button blue">Twist_Left</button></label></form></td>
//Twist_Right// <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 17"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Right</button> </label></form></td>      
//<td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 11 5" type="button" onclick="buttonclick(this);" class="button blue">Body_left</button> </label></form></td>
//<td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 10 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_right</button></label></form></td>
//<td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td>

//================================================================================= HTML END =======================================================================

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
      }
      break;
    case WStype_TEXT:
      lastComm = CommOut; CommOut = "";
      for(int i = 0;i < length;i++) CommOut += ((char) payload[i]);
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;default:break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI;"; message += server.uri();  message += "\nMethod;";  message += (server.method() == HTTP_GET)?"GET":"POST"; message += "\nArguments;"; message += server.args(); message += "\n";
  for (uint8_t i=0;i<server.args();i++){
    message += " " +server.argName(i) +";" +server.arg(i) +"\n";
  }
  server.send(404, "text/plain", message);
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT); 

  Serial.begin(115200);//115200
  for(uint8_t t = 4;t > 0;t--) {
    Serial.flush();
    delay(1000);
  }
  delay(1000);
  Serial.begin(115200); 
  /* You can remove the password parameter if you want the AP to be open. */ 
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(200);
/*Set for Wifi PA config*/
#ifdef SET_IP_PA
  kt=WiFi.softAPConfig(local_IP, gateway, subnet);
  #ifdef DEBUG
  Serial.println();
  Serial.print("_________Setting soft-AP configuration ... ");
  Serial.println(kt? "_________Ready" : "_________Failed!");
  #endif
#endif 
 
  IPAddress myIP = WiFi.softAPIP();
  if (mdns.begin("espWebSock", WiFi.localIP())) {
    mdns.addService("http", "tcp", 80); mdns.addService("ws", "tcp", 81);
  }

  #ifdef DEBUG
  Serial.println();
  Serial.print("_________AP IP address: ");
  Serial.println(myIP);
  #endif
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  #ifdef DEBUG
  Serial.println();  
  Serial.println("_________HTTP server started");  
  #endif  
  
  Pos_INT(); 
  Arm_Pos_INT(); 
}
//=========================================
void Send_Comm() {
  String SendString;int i;
  for (i = 1;i < 32;i += 1) {
    if (SMov[i]>=600 and SMov[i]<=2280){
      SendString = SendString +"#" +i +"P" +String(SMov[i]);
    }
  }
  Serial.print (SendString +"T"+String(StepSpeed)+"D0\r\n");wait_serial_return_ok();
}

//=========================================
void Arm_Send_Comm() {
  String SendString;int i;
  for (i = 1;i < 32;i += 1) {
    if (SMov[i]>=900 and SMov[i]<=2100){
      SendString = SendString +"#" +i +"P" +String(SMov[i]);
    }
  }
  Serial.print (SendString +"T"+String(StepSpeed)+"D0\r\n");wait_serial_return_ok();
}

void wait_serial_return_ok()
{  
  int num=0; char c[16]; 
  while(1)
  { 
    //webSocket.loop();server.handleClient();//NAD
    led_blink(1000);
    while(Serial.available() > 0)
    {
      webSocket.loop();server.handleClient();
      c[num]=Serial.read();num++;
      if(num>=15) num=0;
    }
    if(c[num-2]=='O'&&c[num-1]=='K') break;
  }
}

void loop() {
  
   webSocket.loop();
   server.handleClient();
   if (CommOut == "w 0 1"){
    Move_STP(); //~~~~~~~~ Stop
   }
   if (CommOut == "w 1 1") {
    Move_FWD(); //~~~~~~~~ Forward
   }
   if (CommOut == "w 1 2") {
    Move_FWD_StopLeft(); //~~~~~~~~ Forward L //4 chan
   }
   if (CommOut == "w 1 3") {
    Move_FWD_StopRight(); //~~~~~~~~ Forward R
   }
   if (CommOut == "w 2 1") {
    Move_BWD(); //~~~~~~~~ backward
   }
   if (CommOut == "w 3 1") {
    Move_LFT(); //~~~~~~~~ turn left
   }
   if (CommOut == "w 4 1") {
    Move_RGT(); //~~~~~~~~ turn right
   }
   //~~~~~~~~ shake hand
   if (CommOut == "w 5 3"){
     lastSpeed = StepSpeed;StepSpeed = 300;
     Move_SHK();StepSpeed = lastSpeed;CommOut=lastComm;
   }
   //~~~~~~~~ waving
   if (CommOut == "w 6 3"){
     lastSpeed = StepSpeed;StepSpeed = 300;
     Move_WAV();StepSpeed = lastSpeed;CommOut=lastComm;
   }
   //~~~~~~~~ pos initial
   if (CommOut == "w 15") {
    Pos_INT();CommOut=lastComm;
   }
   //~~~~~~~~ pos service
   if (CommOut == "w 12") {
    Pos_SRV();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body left
   if (CommOut == "w 11 5") {
    Adj_LF();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body right
   if (CommOut == "w 10 5") {
    Adj_RG();CommOut=lastComm;
   }
   //~~~~~~~~ adjust head up
   if (CommOut == "w 8 5") {
    Adj_HU();CommOut=lastComm;
   }
   //~~~~~~~~ adjust head down
   if (CommOut == "w 9 5") {
    Adj_HD();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body higher
   if (CommOut == "w 13") {
    Adj_HG();CommOut=lastComm;
   }
   //~~~~~~~~ adjust body lower
   if (CommOut == "w 14") {
    Adj_LW();CommOut=lastComm;
   }
   //~~~~~~~~ adjust twist left
   if (CommOut == "w 16") {
    Adj_TL();CommOut=lastComm;
   }
   //~~~~~~~~ adjust twist right
   if (CommOut == "w 17") {
    Adj_TR();CommOut=lastComm;
   }
   //~~~~~~~~ adjust speed walk
   if (CommOut == "w 0 0") {
    StepSpeed = 300;CommOut=lastComm;
   }
   //~~~~~~~~ adjust speed normal
   if (CommOut == "w 2 4") {
    StepSpeed = 100;CommOut=lastComm;
   }   
   //~~~~~~~~ adjust speed run
   if (CommOut == "w 7 1") {
    StepSpeed = 50;CommOut=lastComm;
   }
   //~~~~~~~~ adjust speed run fast
   if (CommOut == "w 2 5") {
    StepSpeed = 40;CommOut=lastComm;
   }   
   //~~~~~~~  claw close
   if (CommOut == "w 20") {
    ClwCls();CommOut=lastComm;
   }
   //~~~~~~~~~~~ claw open
   if (CommOut == "w 21") {
    ClwOpn();CommOut=lastComm;
   }
   //FIXNAM
   //~~~~~~~~~~~ xoay phải
   if (CommOut == "w 22") {
    Xoay_Phai_Step();CommOut=lastComm;
   }
   //~~~~~~~~~~~ xoay trái
   if (CommOut == "w 23") {
    Xoay_Trai_Step();CommOut=lastComm;
   }   
   //~~~~~~~~~~~ arm 1 up
   if (CommOut == "w 24") {
    Arm_1_Up();CommOut=lastComm;
   }  
   //~~~~~~~~~~~ arm 1 dn
   if (CommOut == "w 25") {
    Arm_1_Down();CommOut=lastComm;
   } 
   
   //~~~~~~~~~~~ arm 2 up
   if (CommOut == "w 26") {
    Arm_2_Up();CommOut=lastComm;
   }  
   //~~~~~~~~~~~ arm 2 dn
   if (CommOut == "w 27") {
    Arm_2_Down();CommOut=lastComm;
   }    

   //~~~~~~~~~~~ arm 3 up
   if (CommOut == "w 28") {
    Arm_3_Up();CommOut=lastComm;
   }  
   //~~~~~~~~~~~ arm 3 dn
   if (CommOut == "w 29") {
    Arm_3_Down();CommOut=lastComm;
   } 
   //~~~~~~~~~~~ val 50
   if (CommOut == "w 30") {
    val_step=50;CommOut=lastComm;
   }
   if (CommOut == "w 31") {  
    val_step=100;CommOut=lastComm;
   }
   if (CommOut == "w 32") {  
    val_step=200;CommOut=lastComm;
   }
   if (CommOut == "w 33") {  
    Arm_End();CommOut=lastComm;
   } 
   if (CommOut == "w 34") {  
    Arm_Normal();CommOut=lastComm;
   }
   if (CommOut == "w 35") {  
    Arm_Low();CommOut=lastComm;
   }
   if (CommOut == "w 36") {  
    Arm_Long();CommOut=lastComm;
   }           
  }

//================================================================================= Servo Move =======================================================================
//FIXNAM

//~~~~~~~~ ARM end
void Arm_End(){
  lastSpeed = StepSpeed;StepSpeed = 1000;
  //SMov[22]=SAdj[22]+900;SMov[23]=SAdj[23]+2100;SMov[24]=SAdj[24]+900;Arm_Send_Comm();
  SAdj[22]=900;SAdj[23]=2100;SAdj[24]=900;
  SMov[22]=SAdj[22];SMov[23]=SAdj[23];SMov[24]=SAdj[24];
  Arm_Send_Comm();
  StepSpeed=lastSpeed;
}

void Arm_Normal(){
  lastSpeed = StepSpeed;StepSpeed = 1000;  
  //SMov[22]=SAdj[22]+1300;SMov[23]=SAdj[23]+2100;SMov[24]=SAdj[24]+2000;Arm_Send_Comm();
  SAdj[22]=1300;SAdj[23]=2100;SAdj[24]=2100;
  SMov[22]=SAdj[22];SMov[23]=SAdj[23];SMov[24]=SAdj[24];
  Arm_Send_Comm();
  StepSpeed=lastSpeed;
}

void Arm_Low(){
  lastSpeed = StepSpeed;StepSpeed = 1000;  
  //SMov[22]=SAdj[22]+1600;SMov[23]=SAdj[23]+2100;SMov[24]=SAdj[24]+1800;Arm_Send_Comm();
  SAdj[22]=1750;SAdj[23]=2100;SAdj[24]=2100;
  SMov[22]=SAdj[22];SMov[23]=SAdj[23];SMov[24]=SAdj[24];
  Arm_Send_Comm();  
  StepSpeed=lastSpeed;
}

void Arm_Long(){
  lastSpeed = StepSpeed;StepSpeed = 1000;  
  //SMov[22]=SAdj[22]+2100;SMov[23]=SAdj[23]+900;SMov[24]=SAdj[24]+2100;Arm_Send_Comm();
  SAdj[22]=2100;SAdj[23]=900;SAdj[24]=2100;
  SMov[22]=SAdj[22];SMov[23]=SAdj[23];SMov[24]=SAdj[24];
  Arm_Send_Comm();  
  StepSpeed=lastSpeed;
}
  
//~~~~~~~~ xoay phải từng bước
void Xoay_Phai_Step(){ 
    lastSpeed = StepSpeed;StepSpeed = 500;
    SAdj[21]=SAdj[21]<=500?500:SAdj[21]-=val_step;
    SMov[21]=SAdj[21];
    Send_Comm();
    StepSpeed=lastSpeed;
    
//  xoay_pos-=val_step;
//  if (xoay_pos <=500) xoay_pos=500;
//  Serial.println("#21P" + String(xoay_pos) + "T500D0");
//  wait_serial_return_ok();
}

//~~~~~~~~ xoay trái từng bước
void Xoay_Trai_Step(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[21]=SAdj[21]>=2500?2500:SAdj[21]+=val_step;
    SMov[21]=SAdj[21];
    Send_Comm();
    StepSpeed=lastSpeed;    
  
//  xoay_pos+=val_step;
//  if (xoay_pos >=2500) xoay_pos=2500;
//  Serial.println("#21P" + String(xoay_pos) + "T500D0");
//  wait_serial_return_ok();
}
//
//~~~~~~~~ ARM 1 up
void Arm_1_Up(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[22]=SAdj[22]<900?900:SAdj[22]-=val_step;
    SMov[22]=SAdj[22];
    Arm_Send_Comm();
    StepSpeed=lastSpeed;
     
//  post_arm_1-=val_step;
//  if (post_arm_1 <= 500) post_arm_1=500;
//  Serial.println("#22P" + String(post_arm_1) + "T500D0");
//  wait_serial_return_ok();
} 
//~~~~~~~~ ARM 1 down
void Arm_1_Down(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[22]=SAdj[22]>=2100?2100:SAdj[22]+=val_step;
    SMov[22]=SAdj[22];
    Arm_Send_Comm();
    StepSpeed=lastSpeed;
  
//  post_arm_1+=val_step;
//  if (post_arm_1 >= 2500) post_arm_1=2500;
//  Serial.println("#22P" + String(post_arm_1) + "T500D0");
//  wait_serial_return_ok();
} 
//~~~~~~~~ ARM 2 up
void Arm_2_Up(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[23]=SAdj[23]<=900?900:SAdj[23]-=val_step;
    SMov[23]=SAdj[23];
    Arm_Send_Comm();
    StepSpeed=lastSpeed;
           
//  post_arm_2-=val_step;
//  if (post_arm_2 <= 900) post_arm_2=900;
//  Serial.println("#23P" + String(post_arm_2) + "T500D0");
//  wait_serial_return_ok();
} 
//~~~~~~~~ ARM 2 down
void Arm_2_Down(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[23]=SAdj[23]>=2100?2100:SAdj[23]+=val_step;
    SMov[23]=SAdj[23];
    Arm_Send_Comm(); 
    StepSpeed=lastSpeed;
//  post_arm_2+=val_step;
//  if (post_arm_2 >= 2100) post_arm_2=2100;
//  Serial.println("#23P" + String(post_arm_2) + "T500D0");
//  wait_serial_return_ok();
} 

//~~~~~~~~ ARM 3 up
void Arm_3_Up(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[24]=SAdj[24]>=2100?2100:SAdj[24]+=val_step;
    SMov[24]=SAdj[24];
    Arm_Send_Comm();
    StepSpeed=lastSpeed;
  
//  post_arm_3+=val_step;
//  if (post_arm_3 >= 2500) post_arm_3=2500;
//  Serial.println("#24P" + String(post_arm_3) + "T500D0");
//  wait_serial_return_ok();
} 
//~~~~~~~~ ARM 3 down
void Arm_3_Down(){
    lastSpeed = StepSpeed;StepSpeed = 500;  
    SAdj[24]=SAdj[24]<=900?900:SAdj[24]-=val_step;
    SMov[24]=SAdj[24];
    Arm_Send_Comm();
    StepSpeed=lastSpeed;
    
//  post_arm_3-=val_step;
//  if (post_arm_3 <= 500) post_arm_3=500;
//  Serial.println("#24P" + String(post_arm_3) + "T500D0");
//  wait_serial_return_ok();
} 

//~~~~~~~~ claw open
void ClwOpn(){
    SAdj[9]=2000;
    SMov[9]=SAdj[9];
    Send_Comm();
  
//  ClawPos=2000;
//  if (ClawPos >= 2000) ClawPos=2000;
//  Serial.println("#9P" + String(ClawPos) + "T50D0");
//  wait_serial_return_ok();
}
//~~~~~~~~~~claw close
void ClwCls(){
    SAdj[9]=SAdj[9]<=500?500:SAdj[9]-=val_step;
    SMov[9]=SAdj[9];
    Send_Comm();
      
//  ClawPos -=val_step;
//  if (ClawPos <= 500) ClawPos=500;
//  Serial.println("#9P" + String(ClawPos) + "T50D0");
//  wait_serial_return_ok();
}
//~~~~~~~~ 7
//void Arm_Pos_SRV(){
//  lastSpeed = StepSpeed;StepSpeed = 1000;
//  SMov[9]=SAdj[9];SMov[21]=SAdj[21];Send_Comm();
//  SMov[22]=SAdj[22];SMov[23]=SAdj[23];SMov[24]=SAdj[24];Arm_Send_Comm();
//  StepSpeed = lastSpeed;
//}
//~~~~~~~~ Initial position (adjust all init servo here)
void Arm_Pos_INT(){
   lastSpeed = StepSpeed;StepSpeed = 1000;
   SMov[9]=1000;Send_Comm();
   SMov[21]=1440;Send_Comm();
   SMov[22]=900;Arm_Send_Comm();
   SMov[23]=2100;SMov[24]=900;Arm_Send_Comm();  
   StepSpeed = lastSpeed;

//  SAdj[9]=1000;SAdj[21]=1440;Send_Comm();
//  SAdj[22]=900;Arm_Send_Comm();
//  SAdj[23]=2100;SAdj[24]=900;Arm_Send_Comm();
//  Arm_Pos_SRV();
}

//~~~~~~~~ 7
void Pos_SRV(){
  lastSpeed = StepSpeed;StepSpeed = 50;
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1440;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  StepSpeed = lastSpeed;
}
//~~~~~~~~ Initial position (adjust all init servo here)
void Pos_INT(){
  SAdj[29]=0;SAdj[30]=0;SAdj[31]=0;SAdj[17]=0;SAdj[18]=0;SAdj[19]=0;SAdj[1]=0;SAdj[2]=0;SAdj[3]=0;SAdj[5]=0;SAdj[6]=0;SAdj[7]=0;SAdj[13]=0;SAdj[14]=0;SAdj[15]=0;SAdj[25]=0;SAdj[26]=0;SAdj[27]=0;Send_Comm();
  Pos_SRV();
}
//~~~~~~~~ Stop motion
void Move_STP(){
  SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;Send_Comm();
}
//~~~~~~~~ Shake hand
void Move_SHK(){
  SMov[5]=SAdj[5]+1117;SMov[6]=SAdj[6]+2218;SMov[7]=SAdj[7]+1828;Send_Comm();
  SMov[7]=SAdj[7]+1246;Send_Comm();
  SMov[7]=SAdj[7]+1795;Send_Comm();
  SMov[7]=SAdj[7]+1182;Send_Comm();
  SMov[7]=SAdj[7]+1763;Send_Comm();
  SMov[7]=SAdj[7]+1117;Send_Comm();
  SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;Send_Comm();
}
//~~~~~~~~ Waving hand
void Move_WAV(){
  SMov[5]=SAdj[5]+1058;SMov[6]=SAdj[6]+1975;SMov[7]=SAdj[7]+2280;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1096;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1096;Send_Comm();
  SMov[5]=SAdj[5]+1478;Send_Comm();
  SMov[5]=SAdj[5]+1440;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;Send_Comm();
}
//~~~~~~~~ adjust body higher
void Adj_HG(){
   SAdj[6]-=val_step;SAdj[7]+=val_step/2;SAdj[14]-=val_step;SAdj[15]+=val_step/2;SAdj[26]-=val_step;SAdj[27]+=val_step/2;SAdj[2]+=val_step;SAdj[3]-=val_step/2;SAdj[18]+=val_step;SAdj[19]-=val_step/2;SAdj[30]+=val_step;SAdj[31]-=val_step/2;
   Pos_SRV();
}
//~~~~~~~~ adjust body lower
void Adj_LW(){
  SAdj[6]+=val_step;SAdj[7]-=val_step/2;SAdj[14]+=val_step;SAdj[15]-=val_step/2;SAdj[26]+=val_step;SAdj[27]-=val_step/2;SAdj[2]-=val_step;SAdj[3]+=val_step/2;SAdj[18]-=val_step;SAdj[19]+=val_step/2;SAdj[30]-=val_step;SAdj[31]+=val_step/2;
  Pos_SRV();
}
//~~~~~~~~ adjust head up
void Adj_HU(){
   SAdj[6]-=val_step;SAdj[7]+=val_step/2;SAdj[26]+=val_step;SAdj[27]-=val_step/2;SAdj[2]+=val_step;SAdj[3]-=val_step/2;SAdj[30]-=val_step;SAdj[31]+=val_step/2;
   Pos_SRV();
}
//~~~~~~~~ adjust head down
void Adj_HD(){
   SAdj[6]+=val_step;SAdj[7]-=val_step/2;SAdj[26]-=val_step;SAdj[27]+=val_step/2;SAdj[2]-=val_step;SAdj[3]+=val_step/2;SAdj[30]+=val_step;SAdj[31]-=val_step/2;
   Pos_SRV();
}
//~~~~~~~~ adjust body left
void Adj_LF(){
  SAdj[6]+=val_step;SAdj[7]-=val_step/2;SAdj[14]+=val_step;SAdj[15]-=val_step/2;SAdj[26]+=val_step;SAdj[27]-=val_step/2;SAdj[2]+=val_step;SAdj[3]-=val_step/2;SAdj[18]+=val_step;SAdj[19]-=val_step/2;SAdj[30]+=val_step;SAdj[31]-=val_step/2;
  Pos_SRV();
}
//~~~~~~~~ adjust body right
void Adj_RG(){
  SAdj[6]-=val_step;SAdj[7]+=val_step/2;SAdj[14]-=val_step;SAdj[15]+=val_step/2;SAdj[26]-=val_step;SAdj[27]+=val_step/2;SAdj[2]-=val_step;SAdj[3]+=val_step/2;SAdj[18]-=val_step;SAdj[19]+=val_step/2;SAdj[30]-=val_step;SAdj[31]+=val_step/2;
  Pos_SRV();
}
//~~~~~~~~ adjust twist left
void Adj_TL(){
   SAdj[5]-=val_step;SAdj[13]-=val_step;SAdj[25]-=val_step;SAdj[1]-=val_step;SAdj[17]-=val_step;SAdj[29]-=val_step;
   Pos_SRV();
}
//~~~~~~~~ adjust twist right
void Adj_TR(){
   SAdj[5]+=val_step;SAdj[13]+=val_step;SAdj[25]+=val_step;SAdj[1]+=val_step;SAdj[17]+=val_step;SAdj[29]+=val_step;
   Pos_SRV();
}
//~~~~~~~~ move forward
void Move_FWD(){   
  SMov[29]=SAdj[29]+1565;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1375;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1420;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1770;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1355;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1765;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1495;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1620;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1430;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1965;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1570;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1470;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1505;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1375;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1640;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1320;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1580;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1720;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1395;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1530;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1675;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1795;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1470;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1330;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1870;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1545;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1130;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1720;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1620;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1205;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1775;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1455;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1570;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1695;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1280;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
}
//FIXNAM
//2 chan
void Move_FWD_StopRight(){   
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1350;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1405;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1275;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1480;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1200;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1350;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1625;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1500;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1705;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1650;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1575;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1855;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1500;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1705;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1440;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1430;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1440;Send_Comm();
}
//4 chan
void Move_FWD_StopLeft(){   
  SMov[29]=SAdj[29]+1540;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1350;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1430;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1365;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1275;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1505;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1440;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1940;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1200;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1515;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1865;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1350;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1650;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1590;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1790;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1500;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1540;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1715;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1650;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1805;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1340;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1640;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1575;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1880;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1140;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1565;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1500;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1730;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1215;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1440;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1430;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1580;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1440;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1290;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
}

//~~~~~~~~ move backward
void Move_BWD(){
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1755;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1340;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1265;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1930;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1605;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1855;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1455;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1590;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1700;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1380;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1640;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1565;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1490;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1480;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1830;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1415;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
}
//~~~~~~~~ turn left
void Move_LFT(){
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1755;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1340;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1265;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1930;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1605;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1855;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1455;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1590;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1700;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1380;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1640;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1565;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1490;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1480;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1830;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1415;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  }
//~~~~~~~~ turn right
void Move_RGT(){
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1480;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1830;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1415;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1490;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1565;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1700;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1380;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1640;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1455;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1590;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1855;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1930;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1605;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1265;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1755;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1340;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
}

//////
void led_blink(unsigned int blink_delay){
  unsigned long currentMillis = millis();
  if ((currentMillis - previousMillis >= blink_delay/100)&& (currentMillis - previousMillis < blink_delay)) {
//    if (ledState == LOW) {
//      ledState = HIGH;  // Note that this switches the LED *off*
//    } else {
//      ledState = LOW;  // Note that this switches the LED *on*
//    }
//    previousMillis = currentMillis;
      ledState = HIGH;
  }
  else if(currentMillis - previousMillis >= blink_delay){
     previousMillis = currentMillis;
     ledState = LOW;        
  }
  digitalWrite(LED_BUILTIN, ledState);
  
}

//================================================================================= Servo Move end =======================================================================
