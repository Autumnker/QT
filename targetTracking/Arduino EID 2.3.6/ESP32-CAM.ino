#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

// 增加摄像头任务栈大小
#define CONFIG_CAMERA_TASK_STACK_SIZE 8192

// 选择摄像头模型
#include "board_config.h"

// WiFi 配置
const char *ap_ssid = "ESP32-CAM-AP";
const char *ap_password = "12345678";

// 舵机控制引脚配置
#define SERVO_X_PIN 12  // 控制X方向的舵机引脚
#define SERVO_Y_PIN 13  // 控制Y方向的舵机引脚

// PWM配置
#define PWM_FREQUENCY 50    // 50Hz (SG90舵机标准)
#define PWM_RESOLUTION 12   // 12位分辨率 (降低分辨率以节省内存)
#define PWM_MIN_PULSE 500   // 0度对应的脉冲宽度(微秒)
#define PWM_MAX_PULSE 2500  // 180度对应的脉冲宽度(微秒)

// 舵机通道配置
#define SERVO_X_CHANNEL 1
#define SERVO_Y_CHANNEL 2

// 舵机旋转角度限制
int Y_MIN = -10; int Y_MAX = 80;
int X_MIN = -90; int X_MAX = 90;

// 角度映射区间
int LEFT_RANGE = -90; int RIGHT_RANGE = 90;

// 端口配置
#define SERVER_PORT 8888

// 调试时开启LED
// #define DEBUG_LED_ON

// 全局变量
WiFiServer tcpServer(SERVER_PORT);
WiFiClient tcpClient;
bool clientConnected = false;
#if defined(LED_GPIO_NUM)
unsigned long lastLedToggle = 0;
bool ledState = false;
#endif
int failCount = 0;
unsigned long lastCaptureTime = 0;
bool cameraInitialized = false;

// 舵机控制参数
String origin_X_Y = "0,0";

// 函数声明
void setServoAngle(int channel, int angle);
void processControlCommand(String command);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // 先初始化 WiFi，减少对摄像头初始化的干扰
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  WiFi.setSleep(false);

  Serial.println("");
  Serial.print("AP Started. SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // 摄像头配置(硬件接口配置)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;  // 降低时钟频率
  config.pixel_format = PIXFORMAT_JPEG;

  // 使用更低分辨率配置以节省内存
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  // 尝试初始化摄像头，最多尝试5次
  for (int attempt = 0; attempt < 5; attempt++) {
    esp_err_t err = esp_camera_init(&config);
    if (err == ESP_OK) {
      cameraInitialized = true;
      Serial.println("Camera initialized successfully");
      break;
    }

    Serial.printf("Camera init attempt %d failed with error 0x%x\n", attempt + 1, err);
    delay(2000);  // 增加延迟时间

    // 如果是I2C错误，尝试重置I2C总线
    if (err == 0x105 || err == 0x106) {  // I2C相关错误码
      // 重置I2C总线
      pinMode(config.pin_sccb_sda, OUTPUT);
      pinMode(config.pin_sccb_scl, OUTPUT);
      digitalWrite(config.pin_sccb_sda, HIGH);
      digitalWrite(config.pin_sccb_scl, HIGH);
      delay(100);

      // 发送I2C复位序列
      for (int i = 0; i < 10; i++) {
        digitalWrite(config.pin_sccb_scl, LOW);
        delayMicroseconds(10);
        digitalWrite(config.pin_sccb_scl, HIGH);
        delayMicroseconds(10);
      }

      // 恢复引脚功能
      pinMode(config.pin_sccb_sda, INPUT);
      pinMode(config.pin_sccb_scl, INPUT);
      delay(100);
    }
  }

  if (!cameraInitialized) {
    Serial.println("Camera initialization failed after multiple attempts");
    Serial.println("Please check hardware connections and power supply");
    // 即使摄像头初始化失败，也继续初始化其他部分
  }

  // 配置摄像头传感器(传感器配置)
  if (cameraInitialized) {
    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV2640_PID) {
      s->set_vflip(s, 1);
      s->set_brightness(s, 1);
      s->set_saturation(s, -2);
    }
  }
  // 初始化舵机PWM (使用Arduino 3.0的新API)
  ledcAttach(SERVO_X_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(SERVO_Y_PIN, PWM_FREQUENCY, PWM_RESOLUTION);

  // 设置舵机初始位置
  processControlCommand(origin_X_Y);

  Serial.println("Servos initialized to center position (90°)");

  // 启动TCP服务器
  tcpServer.begin();
  Serial.println("TCP Server started on port " + SERVER_PORT);

#if defined(LED_GPIO_NUM)
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, LOW);
#endif

  Serial.println("System Ready!");
}

// 设置舵机角度 (使用Arduino 3.0的新API)
void setServoAngle(int pin, int angle) {
  // 确保角度在0-180度范围内
  angle = constrain(angle, 0, 180);

  // 计算占空比 (脉冲宽度映射到0-4095)
  uint32_t pulseWidth = map(angle, 0, 180, PWM_MIN_PULSE, PWM_MAX_PULSE);
  uint32_t duty = (pulseWidth * (1 << PWM_RESOLUTION)) / 20000;  // 20000us = 20ms周期

  ledcWrite(pin, duty);
}

// 处理控制命令
void processControlCommand(String command) {
  // 命令格式: "Xangle,Yangle" 例如: "90,45"
  int commaIndex = command.indexOf(',');
  if (commaIndex > 0) {
    String xStr = command.substring(0, commaIndex);
    String yStr = command.substring(commaIndex + 1);

    int newXAngle = xStr.toInt();
    int newYAngle = yStr.toInt();

    // 验证角度范围 (-90到+90转换为0-180)
    if (newXAngle >= X_MIN && newXAngle <= X_MAX && newYAngle >= Y_MIN && newYAngle <= Y_MAX) {
      int xAngle = map(newXAngle, LEFT_RANGE, RIGHT_RANGE, 0, 180);
      int yAngle = map(newYAngle, LEFT_RANGE, RIGHT_RANGE, 0, 180);

      setServoAngle(SERVO_X_PIN, xAngle);
      setServoAngle(SERVO_Y_PIN, yAngle);

      Serial.printf("Servos set to: X=%d°, Y=%d°\n", xAngle, yAngle);
    } else {
      Serial.printf("Invalid angle values. X Must be between %d and %d.", X_MIN, X_MAX);
      Serial.printf(" | Y Must be between %d and %d.", Y_MIN, Y_MAX);
    }
  } else {
    Serial.println("Invalid command format. Use 'Xangle,Yangle'");
  }
}

#if defined(LED_GPIO_NUM)
const int LED_FLASH_INTERVAL = 500;

void updateStatusLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastLedToggle >= LED_FLASH_INTERVAL) {
    lastLedToggle = currentMillis;
    ledState = !ledState;

#ifdef DEBUG_LED_ON
    if (clientConnected) {
      digitalWrite(LED_GPIO_NUM, HIGH);
    } else {
      digitalWrite(LED_GPIO_NUM, ledState ? HIGH : LOW);
    }
#endif
  }
}
#endif

void sendJPEGImage() {
  if (!cameraInitialized) return;

  const unsigned long MIN_CAPTURE_INTERVAL = 100;

  // 控制捕获频率
  unsigned long currentTime = millis();
  if (currentTime - lastCaptureTime < MIN_CAPTURE_INTERVAL) {
    delay(5);
    return;
  }
  lastCaptureTime = currentTime;

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    failCount++;
    if (failCount > 5) {
      Serial.println("Too many capture failures, trying to reinitialize camera...");
      // 尝试重新初始化摄像头
      esp_camera_deinit();
      delay(1000);
      cameraInitialized = false;
    }
    delay(100);
    return;
  }
  failCount = 0;

  if (fb->format != PIXFORMAT_JPEG) {
    Serial.println("Capture format is not JPEG");
    esp_camera_fb_return(fb);
    return;
  }

  // 发送图像大小信息
  uint32_t imageSize = fb->len;
  tcpClient.write((uint8_t *)&imageSize, sizeof(imageSize));

  // 分块发送图像数据
  const size_t chunkSize = 1024;
  size_t bytesSent = 0;

  while (bytesSent < imageSize) {
    size_t toSend = chunkSize < (imageSize - bytesSent) ? chunkSize : (imageSize - bytesSent);
    size_t sent = tcpClient.write(fb->buf + bytesSent, toSend);

    if (sent != toSend) {
      Serial.println("Failed to send image data");
      clientConnected = false;
      break;
    }

    bytesSent += sent;
    delay(1);
  }

  esp_camera_fb_return(fb);
}

void tryReinitializeCamera() {
  // 简化的重新初始化尝试
  camera_config_t config;
  // 这里需要重新设置配置参数...

  esp_err_t err = esp_camera_init(&config);
  if (err == ESP_OK) {
    cameraInitialized = true;
    Serial.println("Camera reinitialized successfully");
  } else {
    Serial.printf("Camera reinit failed with error 0x%x\n", err);
  }
}

void loop() {
  static unsigned long lastErrorCheck = 0;
  const unsigned long ERROR_CHECK_INTERVAL = 5000;

  // 如果摄像头未初始化，尝试重新初始化
  if (!cameraInitialized) {
    static unsigned long lastReinitAttempt = 0;
    if (millis() - lastReinitAttempt > 5000) {
      lastReinitAttempt = millis();
      tryReinitializeCamera();
    }
    delay(100);
    return;
  }

  if (!clientConnected) {
    tcpClient = tcpServer.available();
    if (tcpClient) {
      Serial.println("New client connected");
      clientConnected = true;

      // 设置摄像头参数
      sensor_t *s = esp_camera_sensor_get();
      if (s) {
        s->set_framesize(s, FRAMESIZE_QVGA);
        s->set_quality(s, 12);
      }
    }
  } else {
    if (!tcpClient.connected()) {
      Serial.println("Client disconnected");
      clientConnected = false;
      tcpClient.stop();
    } else {
      // 检查是否有控制命令
      if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        processControlCommand(command);
        Serial.println("URAT command is: " + command);
      } else if (tcpClient.available()) {
        String command = tcpClient.readStringUntil('\n');
        command.trim();
        processControlCommand(command);
      }

      sendJPEGImage();
      delay(100);
    }
  }

  // 定期检查错误
  unsigned long currentMillis = millis();
  if (currentMillis - lastErrorCheck >= ERROR_CHECK_INTERVAL) {
    lastErrorCheck = currentMillis;
    if (WiFi.softAPgetStationNum() == 0) {
      Serial.println("No stations connected to AP");
    }

    // 定期检查内存状态
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  }

#if defined(LED_GPIO_NUM)
  updateStatusLED();
#endif
}