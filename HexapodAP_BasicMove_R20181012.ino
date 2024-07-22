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
int SMov[32]={1440,1440,1440,0,1440,1440,1440,0,0,0,0,0,1440,1440,1440,0,1440,1440,1440,0,0,0,0,0,1440,1440,1440,0,1440,1440,1440,0};
int SAdj[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int StaBlink=0; int IntBlink=0;
int ClawPos=1500;
MDNSResponder mdns;
ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

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
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html><html><head><meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0"><title>PET-HQN Spider Hexapod</title><style>"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }"
#JD {text-align: center;}#JD {text-align: center;font-family: "Lucida Sans Unicode", "Lucida Grande", sans-serif;font-size: 24px;}.foot {text-align: center;font-family: "Comic Sans MS", cursive;font-size: 9px;color: #F00;}
.button {border: none;color: white;padding: 20px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;margin: 4px 2px;cursor: pointer;border-radius: 12px;width: 100%;}.red {background-color: #F00;}.green {background-color: #090;}.yellow {background-color:#F90;}.blue {background-color:#03C;}</style>
<script>var websock;function start() {websock = new WebSocket('ws://' + window.location.hostname + ':81/');websock.onopen = function(evt) { console.log('websock open'); };websock.onclose = function(evt) { console.log('websock close'); };websock.onerror = function(evt) { console.log(evt); }; 
websock.onmessage = function(evt) {console.log(evt);var e = document.getElementById('ledstatus');if (evt.data === 'ledon') { e.style.color = 'red';}else if (evt.data === 'ledoff') {e.style.color = 'black';} else {console.log('unknown event');}};} function buttonclick(e) {websock.send(e.id);}</script>
</head><body onload="javascript:start();">&nbsp;<table width="100%" border="1"><tr><td bgcolor="#FFFF33" id="JD">PET-HQN Controller</td></tr></table>
<table width="100" height="249" border="0" align="center">
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 20" type="button" onclick="buttonclick(this);" class="button red">Claw_Close</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 1 1" type="button" onclick="buttonclick(this);" class="button green">Forward</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 21"  type="button" onclick="buttonclick(this);" class="button red">Claw_Open</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 3 1" type="button" onclick="buttonclick(this);" class="button green">Turn_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 1"  type="button" onclick="buttonclick(this);" class="button red">Stop_all</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 4 1"  type="button" onclick="buttonclick(this);" class="button green">Turn_Right</button></label></form></td></tr>
<tr><td>&nbsp;</td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 1" type="button" onclick="buttonclick(this);" class="button green">Backward</button></label></form></td><td>&nbsp;</td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 5 3" type="button" onclick="buttonclick(this);" class="button yellow">Shake </button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 8 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_up</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 6 3"  type="button" onclick="buttonclick(this);" class="button yellow">Wave</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 16" type="button" onclick="buttonclick(this);" class="button blue">Twist_Left</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 9 5"  type="button" onclick="buttonclick(this);" class="button blue">Head_down</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 17"  type="button" onclick="buttonclick(this);" class="button blue">Twist_Right</button> </label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 11 5" type="button" onclick="buttonclick(this);" class="button blue">Body_left</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 13"  type="button" onclick="buttonclick(this);" class="button blue">Body_higher</button></label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 10 5"  type="button" onclick="buttonclick(this);" class="button blue">Body_right</button></label></form></td></tr>
<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 12" type="button" onclick="buttonclick(this);" class="button yellow">Service</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 14"  type="button" onclick="buttonclick(this);" class="button blue">Body_lower</button> </label></form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 15"  type="button" onclick="buttonclick(this);" class="button yellow">Reset_Pose</button> </label></form></td></tr>

<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td>     <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 4"  type="button" onclick="buttonclick(this);" class="button green">Normal</button> </label></form></td>         <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button yellow">Run</button> </label></form></td>           <td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 2 5"  type="button" onclick="buttonclick(this);" class="button green">Fast</button> </label></form></td></tr>


</table><p class="foot">this application requires Mwilmar Quadruped platform.</p></body></html>
)rawliteral";


////
//<tr><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 0 0" type="button" onclick="buttonclick(this);" class="button yellow">Walk</button> </label></form></td>     <td align="center" valign="middle"><form name="form1" method="post" action="">&nbsp;</form></td><td align="center" valign="middle"><form name="form1" method="post" action=""><label><button id="w 7 1"  type="button" onclick="buttonclick(this);" class="button yellow">Run</button> </label></form></td></tr>
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
  Pos_INT(); 

  #ifdef DEBUG
  Serial.println();  
  Serial.println("_________HTTP server started");  
  #endif
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

void wait_serial_return_ok()
{  
  int num=0; char c[16]; 
  while(1)
  { 
    webSocket.loop();server.handleClient();//NAD
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
//  IntBlink +=1;
//  if (IntBlink >=5000){
//    if (StaBlink ==0){
//      digitalWrite(LED_BUILTIN, HIGH); 
//      StaBlink=1;
//    } else {
//      digitalWrite(LED_BUILTIN, LOW); 
//      StaBlink=0;
//    }
//    IntBlink =0;
//  }
  
   webSocket.loop();
   server.handleClient();
   if (CommOut == "w 0 1"){
    Move_STP(); //~~~~~~~~ Stop
   }
   if (CommOut == "w 1 1") {
    Move_FWD(); //~~~~~~~~ Forward
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
    StepSpeed = 20;CommOut=lastComm;
   }   
   //~~~~~~~  claw close
   if (CommOut == "w 20") {
    ClwCls();CommOut=lastComm;
   }
   //~~~~~~~~~~~ claw open
   if (CommOut == "w 21") {
    ClwOpn();CommOut=lastComm;
   }
       
  }

//================================================================================= Servo Move =======================================================================
//~~~~~~~~ claw open
void ClwOpn(){
  ClawPos=1000;
  if (ClawPos <= 1000) ClawPos=1000;
  Serial.println("#9P" + String(ClawPos) + "T50D0");
  wait_serial_return_ok();
}
//~~~~~~~~~~claw close
void ClwCls(){
  ClawPos +=100;
  if (ClawPos >= 1600) ClawPos=1600;
  Serial.println("#9P" + String(ClawPos) + "T50D0");
  wait_serial_return_ok();
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
   SAdj[6]-=50;SAdj[7]+=25;SAdj[14]-=50;SAdj[15]+=25;SAdj[26]-=50;SAdj[27]+=25;SAdj[2]+=50;SAdj[3]-=25;SAdj[18]+=50;SAdj[19]-=25;SAdj[30]+=50;SAdj[31]-=25;
   Pos_SRV();
}
//~~~~~~~~ adjust body lower
void Adj_LW(){
  SAdj[6]+=50;SAdj[7]-=25;SAdj[14]+=50;SAdj[15]-=25;SAdj[26]+=50;SAdj[27]-=25;SAdj[2]-=50;SAdj[3]+=25;SAdj[18]-=50;SAdj[19]+=25;SAdj[30]-=50;SAdj[31]+=25;
  Pos_SRV();
}
//~~~~~~~~ adjust head up
void Adj_HU(){
   SAdj[6]-=50;SAdj[7]+=25;SAdj[26]+=50;SAdj[27]-=25;SAdj[2]+=50;SAdj[3]-=25;SAdj[30]-=50;SAdj[31]+=25;
   Pos_SRV();
}
//~~~~~~~~ adjust head down
void Adj_HD(){
   SAdj[6]+=50;SAdj[7]-=25;SAdj[26]-=50;SAdj[27]+=25;SAdj[2]-=50;SAdj[3]+=25;SAdj[30]+=50;SAdj[31]-=25;
   Pos_SRV();
}
//~~~~~~~~ adjust body left
void Adj_LF(){
  SAdj[6]+=50;SAdj[7]-=25;SAdj[14]+=50;SAdj[15]-=25;SAdj[26]+=50;SAdj[27]-=25;SAdj[2]+=50;SAdj[3]-=25;SAdj[18]+=50;SAdj[19]-=25;SAdj[30]+=50;SAdj[31]-=25;
  Pos_SRV();
}
//~~~~~~~~ adjust body right
void Adj_RG(){
  SAdj[6]-=50;SAdj[7]+=25;SAdj[14]-=50;SAdj[15]+=25;SAdj[26]-=50;SAdj[27]+=25;SAdj[2]-=50;SAdj[3]+=25;SAdj[18]-=50;SAdj[19]+=25;SAdj[30]-=50;SAdj[31]+=25;
  Pos_SRV();
}
//~~~~~~~~ adjust twist left
void Adj_TL(){
   SAdj[5]-=50;SAdj[13]-=50;SAdj[25]-=50;SAdj[1]-=50;SAdj[17]-=50;SAdj[29]-=50;
   Pos_SRV();
}
//~~~~~~~~ adjust twist right
void Adj_TR(){
   SAdj[5]+=50;SAdj[13]+=50;SAdj[25]+=50;SAdj[1]+=50;SAdj[17]+=50;SAdj[29]+=50;
   Pos_SRV();
}
//~~~~~~~~ move forward
void Move_FWD(){   
  SMov[29]=SAdj[29]+1490;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1625;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1480;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1830;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1415;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1690;SMov[30]=SAdj[30]+790;SMov[31]=SAdj[31]+1990;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1225;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1555;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1490;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1890;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1475;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1150;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+2090;SMov[15]=SAdj[15]+990;SMov[25]=SAdj[25]+1565;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1815;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1300;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1700;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1380;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1640;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1740;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1325;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+790;SMov[3]=SAdj[3]+1890;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1455;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1590;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1665;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1250;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1600;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1855;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1530;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1390;SMov[26]=SAdj[26]+2090;SMov[27]=SAdj[27]+890;Send_Comm();
  SMov[29]=SAdj[29]+1590;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1400;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1525;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1930;SMov[6]=SAdj[6]+1440;SMov[7]=SAdj[7]+1440;SMov[13]=SAdj[13]+1605;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1190;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1515;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1550;SMov[18]=SAdj[18]+790;SMov[19]=SAdj[19]+1890;SMov[1]=SAdj[1]+1450;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1780;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1680;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1265;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
  SMov[29]=SAdj[29]+1440;SMov[30]=SAdj[30]+1440;SMov[31]=SAdj[31]+1490;SMov[17]=SAdj[17]+1700;SMov[18]=SAdj[18]+1440;SMov[19]=SAdj[19]+1440;SMov[1]=SAdj[1]+1380;SMov[2]=SAdj[2]+1440;SMov[3]=SAdj[3]+1440;SMov[5]=SAdj[5]+1630;SMov[6]=SAdj[6]+2090;SMov[7]=SAdj[7]+990;SMov[13]=SAdj[13]+1755;SMov[14]=SAdj[14]+1440;SMov[15]=SAdj[15]+1440;SMov[25]=SAdj[25]+1340;SMov[26]=SAdj[26]+1440;SMov[27]=SAdj[27]+1390;Send_Comm();
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
