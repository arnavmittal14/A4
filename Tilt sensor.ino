#include <Wire.h>
#include <TFT_eSPI.h>

const int MPU_ADDR = 0x68;
// Set up display
TFT_eSPI tft = TFT_eSPI();

// define colours
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF


int16_t accelerometer_x, accelerometer_y, accelerometer_z;  // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z;                             // variables for gyro raw data
char tmp_str[7];
char* convert_int16_to_str(int16_t i) {  // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}


// Set up game variables
int car_x = 40;
int car_y = 250;
int score = 0;
int game_over = false;
int obstacle_x = 150;
int obstacle_y = 0;
int obstacle_x2 = 150;
int obstacle_y2 = 0;
// Set up game constants
const int car_width = 20;
const int car_height = 10;
const int obstacle_width = 20;
const int obstacle_height = 20;
const int road_width = 480;
const int road_height = 320;
int obstacle_speed = 10;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.drawRect(0, 0, road_width, road_height, WHITE);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print(score);
}

void drawGame() {

  tft.fillRect(car_x, car_y, car_width, car_height, WHITE);
  tft.fillRect(obstacle_x, obstacle_y, obstacle_width, obstacle_height, RED);
  tft.fillRect(obstacle_x2, obstacle_y2, obstacle_width, obstacle_height, MAGENTA);
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);                         // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false);              // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true);  // request a total of 7*2=14 registers
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read() << 8 | Wire.read();  // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read() << 8 | Wire.read();  // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read() << 8 | Wire.read();  // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  gyro_x = Wire.read() << 8 | Wire.read();           // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read() << 8 | Wire.read();           // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read() << 8 | Wire.read();           // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
                                                     // print out data
  Serial.print("aX = ");
  Serial.print(convert_int16_to_str(accelerometer_x / 100));
  Serial.print(" | aY = ");
  Serial.print(convert_int16_to_str(accelerometer_y / 100));
  Serial.print(" | aZ = ");
  Serial.print(convert_int16_to_str(accelerometer_z / 100));
  Serial.print(" | gX = ");
  Serial.print(convert_int16_to_str(gyro_x / 100));
  Serial.print(" | gY = ");
  Serial.print(convert_int16_to_str(gyro_y / 100));
  Serial.print(" | gZ = ");
  Serial.print(convert_int16_to_str(gyro_z / 100));
  Serial.println();
  if (!game_over) {
    // Move obstacle
    obstacle_y += obstacle_speed;
    obstacle_y2 += obstacle_speed;
    if (obstacle_y > road_height) {
      obstacle_x = random(road_width - obstacle_width);
      obstacle_x2 = random(road_width - obstacle_width);
      obstacle_y2 = random(road_height - obstacle_height);
      obstacle_y = 0;
      score++;
      if (score % 1 == 0) {
        obstacle_speed += 1;
      }
      tft.setCursor(0, 0);
      tft.setTextColor(BLACK);
      tft.print(score);
      tft.setTextColor(WHITE);
      tft.print(score);
    }

    // Move car based on tilt
    car_x += accelerometer_y / 100;
    if (car_x < 0) {
      car_x = 0;
    }
    if (car_x > road_width - car_width) {
      car_x = road_width - car_width;
    }

    // Check for collision
    if (obstacle_x < car_x + car_width && obstacle_x + obstacle_width > car_x && obstacle_y < car_y + car_height && obstacle_y + obstacle_height > car_y) {
      game_over = true;
    }
    if (obstacle_x2 < car_x + car_width && obstacle_x2 + obstacle_width > car_x && obstacle_y2 < car_y + car_height && obstacle_y2 + obstacle_height > car_y) {
      game_over = true;
    }

    // Draw game elements
    tft.drawRect(0, 0, road_width, road_height, WHITE);
    drawGame();
    delay(150);
    tft.fillRect(car_x, car_y, car_width, car_height, BLACK);
    tft.fillRect(obstacle_x, obstacle_y, obstacle_width, obstacle_height, BLACK);
    tft.fillRect(obstacle_x2, obstacle_y2, obstacle_width, obstacle_height, BLACK);
  } else {
    // Game over screen
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Game over!");
    tft.setCursor(0, 20);
    tft.print("Score: ");
    tft.print(score);
    delay(10000);
  }
}
