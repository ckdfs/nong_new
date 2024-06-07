/*
 * @Author: ckdfs 2459317008@qq.com
 * @Date: 2024-05-06 20:40:47
 * @LastEditors: ckdfs 2459317008@qq.com
 * @LastEditTime: 2024-06-08 01:32:38
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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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

WiFiClient tcpClient;
PubSubClient mqttClient;
AHT20 aht20;
BH1750 lightMeter;
Adafruit_SGP30 sgp;

// MQTT消息回调函数，该函数会在PubSubClient对象的loop方法中被调用
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    Serial.printf("Message arrived in topic %s, length %d\n", topic, length);
    Serial.print("Message:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println("\n----------------END----------------");
}

void setup()
{
    Serial.begin(115200);
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
unsigned long previousPublishMillis = 0; // 毫秒时间记录
const long intervalPublishMillis = 3600000; // 时间间隔
unsigned long previousReadMillis = 0; // 毫秒时间记录
const long intervalReadMillis = 10000; // 时间间隔

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
                mqttClient.publish(mqtt_topic_pub, "Connected! Hello mqtt!"); // 连接成功后可以发送消息
                mqttClient.subscribe(mqtt_topic_sub); // 连接成功后可以订阅主题
            }
        }
    }

    // 定期发送消息
    if (mqttClient.connected())
    {
        if (currentMillis - previousPublishMillis >= intervalPublishMillis) // 如果和前次时间大于等于时间间隔
        {
            previousPublishMillis = currentMillis;
            mqttClient.publish(mqtt_topic_pub, "naisu 233~~~");
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
        // 创建一个DynamicJsonDocument对象
        DynamicJsonDocument doc(1024);

        // 设置JSON对象的值
        doc["Temperature"] = temperature;
        doc["Humidity"] = humidity;
        doc["Lux"] = lux;
        // doc["CO2"] = co2;
        doc["CO2"] = 0;

        // 创建一个字符数组来存储JSON字符串
        char json[256];

        // 将JSON对象序列化到字符串中
        serializeJson(doc, json);

        mqttClient.publish(mqtt_topic_pub, json);

        // 显示环境信息到OLED
        display.clearDisplay();
        display.setCursor(0,0);
        display.printf("Temp: %.0f'C\n", temperature);
        display.printf("Hum: %.0f%%\n", humidity);
        display.printf("Lux: %.0f\n", lux);
        // display.printf("CO2: %d ppm", co2);
        display.printf("CO2: %d ppm", 0);
        display.display();
    }

    // 处理MQTT事务
    mqttClient.loop();
}