#include <Arduino.h>
#include <Wire.h>

void testMultipleWireInit() {
    Serial.println("Testing multiple Wire initialization...");
    
    // Тест 1: Инициализация на разных пинах
    Serial.println("Test 1: Initializing Wire on D5, D6");
    Wire.begin(D5, D6);
    
    // Проверка работы I2C
    uint8_t testAddress = 0x40;
    Wire.beginTransmission(testAddress);
    byte error1 = Wire.endTransmission();
    Serial.print("Error code on D5,D6: ");
    Serial.println(error1);
    
    // Тест 2: Повторная инициализация на других пинах
    Serial.println("Test 2: Re-initializing Wire on D6, D5");
    Wire.begin(D6, D5);
    
    Wire.beginTransmission(testAddress);
    byte error2 = Wire.endTransmission();
    Serial.print("Error code on D6,D5: ");
    Serial.println(error2);
    
    // Тест 3: Инициализация на D1, D2
    Serial.println("Test 3: Initializing Wire on D1, D2");
    Wire.begin(D1, D2);
    
    Wire.beginTransmission(testAddress);
    byte error3 = Wire.endTransmission();
    Serial.print("Error code on D1,D2: ");
    Serial.println(error3);
    
    // Тест 4: Возврат к первоначальным пинам
    Serial.println("Test 4: Returning to D5, D6");
    Wire.begin(D5, D6);
    
    Wire.beginTransmission(testAddress);
    byte error4 = Wire.endTransmission();
    Serial.print("Error code on D5,D6 (again): ");
    Serial.println(error4);
    
    Serial.println("Multiple Wire initialization test completed.");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    testMultipleWireInit();
}

void loop() {
    delay(1000);
}