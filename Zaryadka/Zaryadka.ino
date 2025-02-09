#include <avr/io.h>
#include <avr/interrupt.h>

volatile int signalMode = 0;  // 0 - Синус 50 кГц, 1 - Треугольник 50 кГц, 2 - Треугольник 100 кГц, 3 - Синус 100 кГц
volatile uint8_t waveformIndex = 0;
const uint8_t numSamples = 10;
const uint16_t triangleWave50[numSamples] = {46, 103, 160, 217, 274, 274, 217, 160, 103, 46};
const uint16_t triangleWave100[numSamples] = {23, 52, 80, 108, 137, 137, 108, 80, 52, 23};

// Прерывание Timer2: обновляет скважность PWM
ISR(TIMER2_OVF_vect) {
  if (signalMode == 0 || signalMode == 3) {
    OCR1A = ICR1 / 2;  // Синус (50% duty cycle)
  } else {
    waveformIndex = (waveformIndex + 1) % numSamples;
    if (signalMode == 1) {
      OCR1A = triangleWave50[waveformIndex];  // Треугольник 50 кГц
    } else {
      OCR1A = triangleWave100[waveformIndex]; // Треугольник 100 кГц
    }
  }
}
// Функция изменения частоты Timer1 (50 кГц или 100 кГц)
void setPWMFrequency(uint16_t topValue) {
  ICR1 = topValue;  
  OCR1A = ICR1 / 2; 
}

void setup() {
  //Настройка Timer1 для генерации PWM
  pinMode(9, OUTPUT);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  setPWMFrequency(319);  // Стартовая частота 50 кГц
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  //Настройка Timer2
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  TCCR2B |= (1 << CS22);  
  TIMSK2 |= (1 << TOIE2);
  //Настройка кнопки и светодиода
  pinMode(2, INPUT_PULLUP);  
  pinMode(13, OUTPUT);  
  sei();  
}

void loop() {
  static bool lastButtonState = HIGH;
  static uint32_t lastDebounceTime = 0;
  const uint32_t debounceDelay = 50;
  bool reading = digitalRead(2);
  if (reading == LOW && lastButtonState == HIGH) {
    if (millis() - lastDebounceTime > debounceDelay) {
      signalMode = (signalMode + 1) % 4;  // Переключение режимов
      // Устанавливаем частоту в зависимости от режима
      if (signalMode == 0 || signalMode == 1) {
        setPWMFrequency(319);  // 50 кГц
      } else {
        setPWMFrequency(159);  
      }
      digitalWrite(13, signalMode % 2);  // Светодиод: включен в режиме треугольника, выключен в режиме синуса
      lastDebounceTime = millis();
    }
  }
  lastButtonState = reading;
}