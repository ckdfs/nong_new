/*
 * @Author: ckdfs 2459317008@qq.com
 * @Date: 2024-05-06 20:40:47
 * @LastEditors: ckdfs 2459317008@qq.com
 * @LastEditTime: 2024-06-15 01:53:14
 * @FilePath: /esp32/esp32.ino
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <AHT20.h>
#include <BH1750.h>
#include <Adafruit_SGP30.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 定义RS485模块连接的TX和RX引脚
#define RX_PIN  6
#define TX_PIN  7

// 初始化软件串口
SoftwareSerial rs485(RX_PIN, TX_PIN);


// 定义一个结构体来存储传感器数据
struct SoilSensorData {
  float soilHumidity;
  float soilTemperature;
  float soilConductivity;
  float soilPH;
};

// WiFi相关配置信息
const char *wifi_ssid = "GKDHAJIMI";
const char *wifi_password = "gkdhajimi";

// MQTT相关配置信息
const char *mqtt_broker_addr = "49.235.106.143"; // 服务器地址
const uint16_t mqtt_broker_port = 1883; // 服务端口号            
const uint16_t mqtt_client_buff_size = 4096; // 客户端缓存大小（非必须）
String mqtt_client_id = "esp32_ywbveu"; // 客户端ID
const char *mqtt_topic_pub = "hello"; // 需要发布到的主题
const char *mqtt_topic_sub = "hello"; // 需要订阅的主题
const char *mqtt_topic_pub2 = "relay"; // 需要发布到的主题
const char *mqtt_topic_sub2 = "relay"; // 需要订阅的主题

WiFiClient tcpClient;
PubSubClient mqttClient;
AHT20 aht20;
BH1750 lightMeter;
Adafruit_SGP30 sgp;

// MQTT消息回调函数，该函数会在PubSubClient对象的loop方法中被调用
void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  Serial.printf("Message arrived in topic %s, length %d\n", topic, length);
  String message;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println("\n");

  if (strcmp(topic, "relay") == 0) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, message);
    String action = doc["action"];

    if (action == "ask") {
      // 获取并发送当前继电器状态
      StaticJsonDocument<200> ansDoc;
      ansDoc["action"] = "ans";
      ansDoc["0"] = digitalRead(0);
      ansDoc["1"] = digitalRead(1);
      ansDoc["3"] = digitalRead(3);
      ansDoc["10"] = digitalRead(10);
      String ansMessage;
      serializeJson(ansDoc, ansMessage);
      mqttClient.publish(mqtt_topic_pub2, ansMessage.c_str());
    } else if (action == "ctrl") {
      // 控制继电器状态
      digitalWrite(0, doc["0"].as<int>());
      digitalWrite(1, doc["1"].as<int>());
      digitalWrite(3, doc["3"].as<int>());
      digitalWrite(10, doc["10"].as<int>());

      // 发送改变后的继电器状态
      StaticJsonDocument<200> ansDoc;
      ansDoc["action"] = "ans";
      ansDoc["0"] = digitalRead(0);
      ansDoc["1"] = digitalRead(1);
      ansDoc["3"] = digitalRead(3);
      ansDoc["10"] = digitalRead(10);
      String ansMessage;
      serializeJson(ansDoc, ansMessage);
      mqttClient.publish(mqtt_topic_pub2, ansMessage.c_str());
    }
  }
}

void setup()
{
    Serial.begin(115200);
    rs485.begin(4800);
    Serial.println();

    // 连接网络
    Serial.printf("\nConnecting to %s", wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("ok.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // 设置MQTT客户端
    mqttClient.setClient(tcpClient);
    mqttClient.setServer(mqtt_broker_addr, mqtt_broker_port);
    mqttClient.setBufferSize(mqtt_client_buff_size);
    mqttClient.setCallback(mqtt_callback);

    // 初始化AHT20传感器
    Wire.begin(4, 5); // SDA: GPIO4, SCL: GPIO5
    aht20.begin();
    
    // 初始化BH1750传感器
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

    // 初始化SGP30传感器
    // if (!sgp.begin()){
    //     Serial.println("Sensor not found :(");
    //     while (1);
    // }

    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(10, OUTPUT);
    digitalWrite(0, LOW);
    digitalWrite(1, LOW);
    digitalWrite(3, LOW);
    digitalWrite(10, LOW);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // 无限循环
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.display();
}

unsigned long previousConnectMillis = 0; // 毫秒时间记录
const long intervalConnectMillis = 5000; // 时间间隔
unsigned long previousReadMillis = 0; // 毫秒时间记录
const long intervalReadMillis = 10000; // 时间间隔
unsigned long flag = 1;

void loop()
{
    unsigned long currentMillis = millis(); // 读取当前时间

    // 连接MQTT服务器
    if (!mqttClient.connected()) // 如果未连接
    {
        if (currentMillis - previousConnectMillis > intervalConnectMillis)
        {
            previousConnectMillis = currentMillis;
            mqtt_client_id += String(WiFi.macAddress()); // 每个客户端需要有唯一的ID，不然上线时会把其他相同ID的客户端踢下线
            if (mqttClient.connect(mqtt_client_id.c_str())) // 尝试连接服务器
            // if (mqttClient.connect(mqtt_client_id.c_str(), mqtt_username, mqtt_password))
            {
                mqttClient.subscribe(mqtt_topic_sub); // 连接成功后可以订阅主题
                mqttClient.subscribe(mqtt_topic_sub2);
            }
        }
    }

    // 读取传感器数据并发送
    if (currentMillis - previousReadMillis >= intervalReadMillis)
    {
        previousReadMillis = currentMillis;
        float temperature = aht20.getTemperature();
        float humidity = aht20.getHumidity();
        float lux = lightMeter.readLightLevel(); // 读取光照强度
        // if (! sgp.IAQmeasure()) {
        //     Serial.println("Measurement failed");
        //     return;
        // }
        // uint16_t co2 = sgp.eCO2; // 读取二氧化碳浓度

        // 发送问询帧
        sendQuery();
        SoilSensorData data;
        // 读取并解析应答帧
        data = parseResponse();
            
        // 使用返回的数据进行输出
        // Serial.print("soilHumidity: ");
        // Serial.println(data.soilHumidity);
        // Serial.print("soilTemperature: ");
        // Serial.println(data.soilTemperature);
        // Serial.print("soilConductivity: ");
        // Serial.println(data.soilConductivity);
        // Serial.print("soilPH: ");
        // Serial.println(data.soilPH);

        // 创建一个DynamicJsonDocument对象
        DynamicJsonDocument doc(1024);

        // 设置JSON对象的值
        doc["temperature"] = temperature;
        doc["humidity"] = humidity;
        doc["lightIntensity"] = lux;
        // doc["co2Concentration"] = co2;
        doc["co2Concentration"] = 0;
        doc["soilTemperature"] = data.soilTemperature;
        doc["soilHumidity"] = data.soilHumidity;
        doc["soilPH"] = data.soilPH;
        doc["soilConductivity"] = data.soilConductivity;

        // 创建一个字符数组来存储JSON字符串
        char json[256];

        // 将JSON对象序列化到字符串中
        serializeJson(doc, json);

        mqttClient.publish(mqtt_topic_pub, json);

        // 显示环境信息到OLED
        display.clearDisplay();
        display.setCursor(0,0);
        if (flag) {
            display.printf("Temp: %.0f'C\n", temperature);
            display.printf("Hum: %.0f%%\n", humidity);
            display.printf("Lux: %.0f\n", lux);
            // display.printf("CO2: %d ppm", co2);
            display.printf("CO2: %d ppm", 0);
        }
        else {
            display.printf("SoilTemp: %.0f'C\n", data.soilTemperature);
            display.printf("SoilHum: %.0f%%\n", data.soilHumidity);
            display.printf("SoilPH: %.0f\n", data.soilPH);
            display.printf("SoilCond: %d uS/cm", data.soilConductivity);
        }
        display.display();
        flag = !flag;
    }

    // 处理MQTT事务
    mqttClient.loop();
}

void sendQuery() {
  // 构造问询帧
  byte queryFrame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
  
  // 发送问询帧
  rs485.write(queryFrame, sizeof(queryFrame));
  
  // 在串口输出发送的问询帧内容
  Serial.print("发送问询帧: ");
  for (int i = 0; i < sizeof(queryFrame); i++) {
    Serial.print("0x");
    if (queryFrame[i] < 0x10) Serial.print("0");
    Serial.print(queryFrame[i], HEX);
    Serial.print(" ");
  }
  Serial.println(); // 换行
}

SoilSensorData parseResponse() {
  unsigned long startTime = millis(); // 记录开始等待的时间
  int retryCount = 0; // 重试计数器
  const int maxRetries = 3; // 最大重试次数

  while (true) {
    // 等待直到有足够的数据可读取或超时
    while (rs485.available() < 13) {
      if (millis() - startTime > 1000) { // 1秒超时
        if (retryCount < maxRetries) {
          Serial.println("应答超时，重新发送问询帧");
          sendQuery(); // 重新发送问询帧
          startTime = millis(); // 重置开始等待的时间
          retryCount++; // 增加重试计数
        } else {
          Serial.println("超过最大重试次数，放弃等待");
          return SoilSensorData(); // 返回空的数据结构体
        }
      }
    }

    // 读取应答帧
    byte response[13];
    for (int i = 0; i < 13; i++) {
      response[i] = rs485.read();
    }

    // 解析应答帧
    int moisture = (response[3] << 8) | response[4];
    int16_t temperature = (int16_t)((response[5] << 8) | response[6]);
    int conductivity = (response[7] << 8) | response[8];
    int pH = (response[9] << 8) | response[10];

    // 计算实际值并存储在结构体中
    SoilSensorData data;
    data.soilHumidity = moisture / 10.0;
    data.soilTemperature = temperature / 10.0;
    data.soilConductivity = conductivity; // 电导率单位为us/cm
    data.soilPH = pH / 10.0;

    return data;
  }
}