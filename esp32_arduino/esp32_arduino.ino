/*
 * @Author: ckdfs 2459317008@qq.com
 * @Date: 2024-05-06 20:40:47
 * @LastEditors: ckdfs 2459317008@qq.com
 * @LastEditTime: 2024-05-06 21:55:17
 * @FilePath: /esp32/esp32.ino
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <WiFi.h>
#include <PubSubClient.h>

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
}

unsigned long previousConnectMillis = 0; // 毫秒时间记录
const long intervalConnectMillis = 5000; // 时间间隔
unsigned long previousPublishMillis = 0; // 毫秒时间记录
const long intervalPublishMillis = 3600000; // 时间间隔

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

    // 处理MQTT事务
    mqttClient.loop();
}
