//************************************************************
// this is a simple example that uses the easyMesh library
//
// 1. blinks led once for every node on the mesh
// 2. blink cycle repeats every BLINK_PERIOD
// 3. sends a silly message to every node on the mesh at a random time betweew 1 and 5 seconds
// 4. prints anything it recieves to Serial.print
// 
//
//************************************************************
#include <easyMesh.h>
#include "easyWebServer.h"
#include <ESP8266WebServer.h>
#include <FS.h>

// some gpio pin that is connected to an LED... 
// on my rig, this is 5, change to the right number of your LED.
#define   LED             5       // GPIO number of connected LED

#define   BLINK_PERIOD    1000000 // microseconds until cycle repeat
#define   BLINK_DURATION  100000  // microseconds LED is on for

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneeky"
#define   MESH_PORT       5555

easyMesh  mesh;
os_timer_t  yerpTimer;
String webPage = "";

void setup() {
  Serial.begin(115200);
  pinMode( LED, OUTPUT );

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setReceiveCallback( &receivedCallback );
  mesh.setNewConnectionCallback( &newConnectionCallback );
  webServerInit();
  os_timer_setfn( &yerpTimer, yerpCb, NULL );
  os_timer_arm( &yerpTimer, 1000, 1 );
  mesh.debugMsg( STARTUP, "\nIn setup() my chipId=%d\n", mesh.getChipId());
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(MESH_PREFIX);

  

}

void loop() {
  mesh.update();
}

void yerpCb( void *arg ) {
  static int yerpCount;
  int connCount = 0;

  String msg = "Yerp=";
  msg += yerpCount++;

  mesh.debugMsg( APPLICATION, "msg-->%s<-- stationStatus=%u numConnections=%u\n", msg.c_str(), wifi_station_get_connect_status(), mesh.connectionCount( NULL ) );

  SimpleList<meshConnectionType>::iterator connection = mesh._connections.begin();
  while ( connection != mesh._connections.end() ) {
    mesh.debugMsg( APPLICATION, "\tconn#%d, chipId=%d subs=%s\n", connCount++, connection->chipId, connection->subConnections.c_str() );
    connection++;
  }

  // send ping to webSockets
  String ping("ping");
  //sendWsControl();
}

void receivedCallback( uint32_t from, String &msg ) {
  File submit = SPIFFS.open("/Submit.txt", "w");
  if (!submit) {
      Serial.println("file open failed on mesh recived cb");
  }
  submit.print(msg);
  submit.close();
  
  Serial.printf("startHere: Received from %d msg=%s\n", from, msg.c_str());

}

void newConnectionCallback( bool adopt ) {
  Serial.printf("startHere: New Connection, adopt=%d\n", adopt);
}


