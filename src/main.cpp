#include "MainHeader.h"
#ifdef LOGGING_BT
BluetoothSerial SerialBT;
#endif
// Only one file is allowed to include TaskScheduler
#define _TASK_STATUS_REQUEST
#include "TaskScheduler.h"

#include "ClientStorage.h"
#include "Task.h"

#define PASSWORD_LEN 4
#define BEACON_INTERVAL (1000 * TASK_MILLISECOND)

void onRecvFromClient(const uint8_t *mac_addr, const uint8_t *data, int data_len);
uint32_t generatePassword(uint8_t len);

esp_now_peer_info_t broadcastPeer{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
Scheduler scheduler;
BeaconPacket broadcastData(generatePassword(PASSWORD_LEN));

void setup()
{
  Serial.begin(115200);
#ifdef LOGGING_BT
  SerialBT.begin("ESP32-LOG");
#endif
  // Call this first to set log level
  LOG_INIT(LOG_LEVEL_VERBOSE);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK)
  {
    LOG_FATAL("[ERROR] Initialize ESP-NOW failed");
    delay(500);
    ESP.restart();
  }
  LOG_VERBOSE("MAC: %s", WiFi.macAddress().c_str());
  LOG_VERBOSE("Beacon password: %u", broadcastData.password);
  esp_now_register_recv_cb(&onRecvFromClient);

  broadcastPeer.channel = CHANNEL;
  broadcastPeer.encrypt = false;

  client_storage::init();

  beacon::init(&scheduler);
  beacon::setPeer(&broadcastPeer);
  beacon::setData((uint8_t *)&broadcastData, sizeof(broadcastData));
  beacon::task.setInterval(BEACON_INTERVAL);

  beacon::task.enable();
}

void loop()
{
  scheduler.execute();
}

void onRecvFromClient(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  // Unpack data
  switch (((BasePacket *)data)->type)
  {
  case packet_t::RECV_ANSW:
    // Receive client's answer to the question
    break;
  case packet_t::RESPOND_BEACON:
    // Receive client's respond to register client
    break;
  case packet_t::REQ_BEACON:
    // client request to know the server address
    // (in case the client can't capture BeaconPacket)
    // May not need to implement
    break;
  default:
    break;
  }
}

// @param len maximum is 10
uint32_t generatePassword(uint8_t len)
{
  if (len > 10)
    len = 10;
  randomSeed(micros());
  uint32_t password = 0;
  for (int i = 0; i < len; i++)
  {
    password += random(1, 5) * pow(10, i);
  }
  return password;
}