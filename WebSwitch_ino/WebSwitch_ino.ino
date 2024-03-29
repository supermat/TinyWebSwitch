// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: December 2010
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//

#include <pins_arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Flash.h>
#include <SD.h>
#include <TinyWebServer.h>
#include <RCSwitch.h>
#include <Module.h>


/****************VALUES YOU CHANGE*************/
// The LED attached to PIN X on an Arduino board.
const int LEDPIN = 7;

// pin 4 is the SPI select pin for the SDcard
const int SD_CS = 4;

// pin 10 is the SPI select pin for the Ethernet
const int ETHER_CS = 10;

// Don't forget to modify the IP to an available one on your home network
byte ip[] = { 192, 168, 0, 20 };

/*********************************************/

RCSwitch mySwitch = RCSwitch();

/****************VALUES YOU CHANGE*************/
#define StartInter1 mySwitch.switchOn("10000", "10000");
#define StopInter1 mySwitch.switchOff("10000", "10000");
/*********************************************/

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// The initial state of the LED
int ledState = LOW;

void setLedEnabled(boolean state) {
  ledState = state;
  //digitalWrite(LEDPIN, ledState);
  Serial << F("Switch");
  if(state)
  {
    StartInter1;
  }
  else
  {
    StopInter1;
  }
}

inline boolean getLedState() { return ledState; }

boolean file_handler(TinyWebServer& web_server);
boolean blink_led_handler(TinyWebServer& web_server);
boolean led_status_handler(TinyWebServer& web_server);
boolean index_handler(TinyWebServer& web_server);

TinyWebServer::PathHandler handlers[] = {
  // Work around Arduino's IDE preprocessor bug in handling /* inside
  // strings.
  //
  // `put_handler' is defined in TinyWebServer
  {"/", TinyWebServer::GET, &index_handler },
  {"/upload/" "*", TinyWebServer::PUT, &TinyWebPutHandler::put_handler },
  {"/blinkled", TinyWebServer::GET, &blink_led_handler },
  {"/action" "*", TinyWebServer::GET, &action_handler },
  {"/ledstatus" "*", TinyWebServer::GET, &led_status_handler },
  {"/" "*", TinyWebServer::GET, &file_handler },
  {NULL},
};

const char* headers[] = {
  "Content-Length",
  NULL
};

TinyWebServer web = TinyWebServer(handlers, headers);

boolean has_filesystem = true;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

void send_file_name(TinyWebServer& web_server, const char* filename) {
  if(!has_filesystem)
  {
    web_server.send_error_code(200);
    web_server.send_content_type("text/plain");
    web_server.end_headers();
    web_server << F("No File System install (No SD Card)");
  }
  else
  {
    if (!filename) {
      web_server.send_error_code(404);
      web_server << F("Could not parse URL");
    } else {
      TinyWebServer::MimeType mime_type
        = TinyWebServer::get_mime_type_from_filename(filename);
      web_server.send_error_code(200);
      web_server.send_content_type(mime_type);
      web_server.end_headers();
      if (file.open(&root, filename, O_READ)) {
        Serial << F("Read file "); Serial.println(filename);
        web_server.send_file(file);
        file.close();
      } else {
        web_server << F("Could not find file: ") << filename << "\n";
      }
    }
  }
}

boolean file_handler(TinyWebServer& web_server) {
  char* filename = TinyWebServer::get_file_from_path(web_server.get_path());
  send_file_name(web_server, filename);
  free(filename);
  return true;
}

/*******/
#define CMDBUF 40
char* ParseUrl(const char *str)
{
  int8_t r=-1;
  int8_t i = 0;
  int index = 4;
  int index1 = 0;
  int plen = 0;
  char ch = str[index];
  char clientline[CMDBUF];
  while( ch != ' ' && index < CMDBUF)
  {
    clientline[index1++] = ch;
    ch = str[++index];
  }
  clientline[index1] = '\0';
 
  // convert clientline into a proper
  // string for further processing
  String urlString = String(clientline);
 
  // extract the operation
  String op = urlString.substring(0,urlString.indexOf(' '));
 
  // we're only interested in the first part...
  urlString = urlString.substring(urlString.indexOf('/'), urlString.indexOf(' ', urlString.indexOf('/')));
 
  // put what's left of the URL back in client line
  urlString.toCharArray(clientline, CMDBUF);
 
  return clientline;
}

void showUrlArg(const char* arg1)
{
  static char clientline[CMDBUF];
  
    sprintf(clientline,"%s",ParseUrl(arg1));
    arg1 = strtok(clientline,"/");
    
    int i = 0;
    while(arg1 != NULL)
    {
        Serial << "Argument " << ++i <<": " << arg1;
        arg1 = strtok(NULL,"/");
    }
 }

void getModule(const char* arg1)
{
  static char clientline[CMDBUF];
  
    sprintf(clientline,"%s",ParseUrl(arg1));
    
    Module mod = Module();
    mod.m_type = atoi(strtok(clientline,"/"));
    mod.m_state = atoi(strtok(NULL,"/"));
 }

boolean action_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.send_content_type("text/plain");
  web_server.end_headers();
  
  showUrlArg(web_server.get_path());
  
  return true;
}
/******/

boolean blink_led_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.send_content_type("text/plain");
  web_server.end_headers();
  // Reverse the state of the LED.
  setLedEnabled(!getLedState());
  Client& client = web_server.get_client();
  if (client.available()) {
    char ch = (char)client.read();
    if (ch == '0') {
      setLedEnabled(false);
    } else if (ch == '1') {
      setLedEnabled(true);
    }
  }
  return true;
}

boolean led_status_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.send_content_type("text/plain");
  web_server.end_headers();
  Client& client = web_server.get_client();
  client.println(getLedState(), DEC);
  return true;
}

boolean index_handler(TinyWebServer& web_server) {
  send_file_name(web_server, "INDEX.HTM");
  return true;
}

void file_uploader_handler(TinyWebServer& web_server,
			   TinyWebPutHandler::PutAction action,
			   char* buffer, int size) {
  static uint32_t start_time;
  static uint32_t total_size;

  switch (action) {
  case TinyWebPutHandler::START:
    start_time = millis();
    total_size = 0;
    if (!file.isOpen()) {
      // File is not opened, create it. First obtain the desired name
      // from the request path.
      char* fname = web_server.get_file_from_path(web_server.get_path());
      if (fname) {
	Serial << F("Creating ") << fname << "\n";
	file.open(&root, fname, O_CREAT | O_WRITE | O_TRUNC);
	free(fname);
      }
    }
    break;

  case TinyWebPutHandler::WRITE:
    if (file.isOpen()) {
      file.write(buffer, size);
      total_size += size;
    }
    break;

  case TinyWebPutHandler::END:
    file.sync();
    Serial << F("Wrote ") << file.fileSize() << F(" bytes in ")
	   << millis() - start_time << F(" millis (received ")
           << total_size << F(" bytes)\n");
    file.close();
  }
}

void setup() {
  Serial.begin(115200);
  Serial << F("Free RAM: ") << FreeRam() << "\n";

  pinMode(LEDPIN, OUTPUT);
  setLedEnabled(false);

  pinMode(SS_PIN, OUTPUT);	// set the SS pin as an output
                                // (necessary to keep the board as
                                // master and not SPI slave)
  digitalWrite(SS_PIN, HIGH);	// and ensure SS is high

  // Ensure we are in a consistent state after power-up or a reset
  // button These pins are standard for the Arduino w5100 Rev 3
  // ethernet board They may need to be re-jigged for different boards
  pinMode(ETHER_CS, OUTPUT); 	// Set the CS pin as an output
  digitalWrite(ETHER_CS, HIGH); // Turn off the W5100 chip! (wait for
                                // configuration)
  pinMode(SD_CS, OUTPUT);       // Set the SDcard CS pin as an output
  digitalWrite(SD_CS, HIGH); 	// Turn off the SD card! (wait for
                                // configuration)

  // initialize the SD card.
  Serial << F("Setting up SD card...\n");
  // Pass over the speed and Chip select for the SD card
  if (!card.init(SPI_FULL_SPEED, SD_CS)) {
    Serial << F("card failed\n");
    has_filesystem = false;
  }
  // initialize a FAT volume.
  if (!volume.init(&card)) {
    Serial << F("vol.init failed!\n");
    has_filesystem = false;
  }
  if (!root.openRoot(&volume)) {
    Serial << F("openRoot failed");
    has_filesystem = false;
  }

  if (has_filesystem) {
    // Assign our function to `upload_handler_fn'.
    TinyWebPutHandler::put_handler_fn = file_uploader_handler;
  }

  // Initialize the Ethernet.
  Serial << F("Setting up the Ethernet card...\n");
  Ethernet.begin(mac, ip);

  // Start the web server.
  Serial << F("Web server starting...\n");
  web.begin();

  Serial << F("Ready to accept HTTP requests.\n");
  
  // Transmitter is connected to Arduino Pin #11
  mySwitch.enableTransmit(11);
  Serial << F("RFSwitch ready.\n");
  
  Serial << F("Free RAM: ") << FreeRam() << "\n";
}

void loop() {
  //if (has_filesystem) 
  {
    web.process();
  }
}
