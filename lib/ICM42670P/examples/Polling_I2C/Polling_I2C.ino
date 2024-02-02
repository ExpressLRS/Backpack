#include "ICM42670P.h"

// Instantiate an ICM42670P with LSB address set to 0
ICM42670P IMU(Wire,0);

void setup() {
  int ret;
  Serial.begin(115200);
  while(!Serial) {}

  // Initializing the ICM42670P
  ret = IMU.begin();
  if (ret != 0) {
    Serial.print("ICM42670P initialization failed: ");
    Serial.println(ret);
    while(1);
  }
  // Accel ODR = 100 Hz and Full Scale Range = 16G
  IMU.startAccel(100,16);
  // Gyro ODR = 100 Hz and Full Scale Range = 2000 dps
  IMU.startGyro(100,2000);
  // Wait IMU to start
  delay(100);
  // Plotter axis header
  Serial.println("AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,Temperature");
}

void loop() {

  inv_imu_sensor_event_t imu_event;

  // Get last event
  IMU.getDataFromRegisters(&imu_event);

  // Format data for Serial Plotter
  Serial.print(imu_event.accel[0]);
  Serial.print(",");
  Serial.print(imu_event.accel[1]);
  Serial.print(",");
  Serial.print(imu_event.accel[2]);
  Serial.print(",");
  Serial.print(imu_event.gyro[0]);
  Serial.print(",");
  Serial.print(imu_event.gyro[1]);
  Serial.print(",");
  Serial.print(imu_event.gyro[2]);
  Serial.print(",");
  Serial.println(imu_event.temperature);

  // Run @ ODR 100Hz
  delay(10);
}
