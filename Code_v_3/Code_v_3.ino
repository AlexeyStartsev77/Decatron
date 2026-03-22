  #include <GyverButton.h>
  #include "pwm.h"

  #define KT0 2    // синий -   1 - Катод 1 (К0)
  #define KT1 3    // зеленый - 4 - Подкатод 4 (1ПК)
  #define KT2 4    // желтый -  6 - Подкатод 6 (2ПК)
  #define KT3 5    // красный - 8 - Подкатод 8 (К)
  #define BTN1 6   // кнопка 1
  #define BTN2 7   // кнопка 2
  #define BTN3 8   // кнопка 3
  #define GEN 9    // генератор

  GButton btnSpeed(BTN1, HIGH_PULL, NORM_OPEN);
  GButton btnEffect(BTN2, HIGH_PULL, NORM_OPEN);
  GButton btnDimm(BTN3, HIGH_PULL, NORM_OPEN);
  
  const int effectsSets = 5;
  int currentEffect = 4;
  int dec_pos;
  boolean effectInit=false;
  boolean smileSide=false;
  int smileDynSise = 2;
  boolean smileDynDir=true;
  int smileDynTimer = 1000;
  unsigned long stroreMillis = millis();
  boolean smileSideMetro = false;
  int smileMetroTimer = 1000;

  const int speedSets = 5;
  int cathodeDelay_mks = 1; //задержка на переход заряда с одного катода на другой - вроде не требуется совсем
  int cathodeDelayInit_mks = 50; // для инициализации
  int InitDelay_mks = 10000;
  int currentSpeed = 0;
  int speedDelays[speedSets] = {250,500,1000,5000,10000}; //microseconds
  int currentDelay_mks = speedDelays[currentSpeed];

  const int dimmSets = 3;
  int currentDimm = 0;
  int dimmValues[dimmSets] = {110,130,180};
  int currentDuty = dimmValues[currentDimm];
  int genWarm = 1000;

void setup() {
  // настройка пинов на выход
  pinMode(KT0, OUTPUT);
  pinMode(KT1, OUTPUT);
  pinMode(KT2, OUTPUT);
  pinMode(KT3, OUTPUT);
  pinMode(GEN, OUTPUT);

  // задаем частоту ШИМ на 9 выводе 31 кГц. Почему именно такая частота - я не знаю
  // возможно, что частота PWM управляет яркостью ламп, либо это только через напряжение 
  set8bitPWM(); // ставим 8 бит для Timer1
  setPWMprescaler(GEN, 1); // множитель х1
  setPWMmode(GEN, 1); // режима Phase-correct PWM
  setPWM(GEN, currentDuty);
  delay(genWarm);
  decathroneInit();
}

void loop() {
  buttonTick();

  switch (currentEffect) {
    case 1:{setDecatronPos(true);break;}
    case 2:{setDecatronPos(false);break;}
    case 3:{smile(2);break;}
    case 4:{smileDyn();break;}
    case 5:{metro(6,4);break;}
  }
  delayMicroseconds(currentDelay_mks);
}

void metro(int size, int delta) {
  //идем по часовой стрелке до центрального нижнего и еще на целую половину size
  int halfSize = size/2;
  if (!effectInit){
    for (int pos=0; pos <=14+halfSize;pos++){
      setDecatronPos(true); 
      delayMicroseconds(currentDelay_mks); 
    }
    effectInit=true;
    smileSide=false;
    stroreMillis = millis();
    smileSideMetro=false;
  }
  for (int pos=1; pos<=halfSize*2-1; pos++){
    setDecatronPos(smileSide); 
    delayMicroseconds(currentDelay_mks);
  }

  if (millis()-stroreMillis >= smileDynTimer){
    if (smileSideMetro == smileSide){
      delta = delta;
    }
    else {
      delta = size+delta;
    }
    for (int pos=1; pos<=delta-1; pos++){
      setDecatronPos(smileSideMetro);
      delayMicroseconds(currentDelay_mks);
    }
    stroreMillis = millis();
    smileSideMetro = !smileSideMetro;
  }
  smileSide=!smileSide;
}

void smileDyn() {
  //идем по часовой стрелке до центрального нижнего и еще на целую половину size

  if (!effectInit){
    for (int pos=0; pos <=14+smileDynSise/2; pos++){
      setDecatronPos(true); 
      delayMicroseconds(currentDelay_mks); 
    }
    effectInit=true;
    smileSide=false;
    stroreMillis = millis();
  }
  //качнем
    for (int pos=1; pos<=smileDynSise-1; pos++){
      setDecatronPos(smileSide); 
      delayMicroseconds(currentDelay_mks);
  }
  
  if (millis()-stroreMillis >= smileMetroTimer){
    if (smileDynSise>=30){smileDynDir=false;}
    else if (smileDynSise<=2){smileDynDir=true;}
    if (smileDynDir) {
      smileDynSise=smileDynSise+2;
      setDecatronPos(smileSide); 
    }
    else {
      smileDynSise=smileDynSise-2;
      setDecatronPos(!smileSide);
      }
    stroreMillis = millis();
    delayMicroseconds(currentDelay_mks);
  }
  smileSide=!smileSide;
}

void smile(int size) {
  //идем по часовой стрелке до центрального нижнего и еще на целую половину size
  int halfSize = size/2;
  if (!effectInit){
    for (int pos=0; pos <=14+halfSize;pos++){
      setDecatronPos(true); 
      delayMicroseconds(currentDelay_mks); 
    }
    effectInit=true;
    smileSide=false;
  }
  //качнем
    for (int pos=1; pos<=halfSize*2-1; pos++){
      setDecatronPos(smileSide); 
      delayMicroseconds(currentDelay_mks);
  }
  smileSide=!smileSide;
}

//при поджиге точки на катоде 1 - точка почемуто зажигается где угодно и ее надо привести в поз.0 (К0)
void decathroneInit () {
  digitalWrite(KT0, HIGH);
  for (int pos = 0; pos <=29; pos++) {
    byte dec = pos % 3; // Определение текущего пина для подачи импульса
    // Подача короткого импульса на соответствующий пин управления
    digitalWrite(KT1, LOW); digitalWrite(KT2, LOW); digitalWrite(KT3, LOW);
    delayMicroseconds(cathodeDelayInit_mks);
    if (dec == 0) digitalWrite(KT3, HIGH);
    if (dec == 1) digitalWrite(KT1, HIGH);
    if (dec == 2) digitalWrite(KT2, HIGH);
    delayMicroseconds(InitDelay_mks);
  }
}

void buttonTick () {

  btnSpeed.tick();
  btnDimm.tick();
  btnEffect.tick();

  if (btnSpeed.isClick()) {
    currentSpeed = (currentSpeed + 1) % speedSets;
    currentDelay_mks = speedDelays[currentSpeed];
  }
  
  if (btnDimm.isClick()) {
    currentDimm = (currentDimm + 1) % dimmSets;
    currentDuty = dimmValues[currentDimm];
    setPWM(GEN, 0);
    setPWM(GEN, currentDuty);
  }

  if (btnEffect.isClick()) {
    currentEffect = (currentEffect + 1) % effectsSets;
    decathroneInit();
    dec_pos = 0;
    effectInit=false;
  }
}

// Установка позиции декатрона на один шаг
void setDecatronPos(boolean step) {
  if (step) { // Шаг вперед
    dec_pos++;
    if (dec_pos >= 30) dec_pos = 0; // Сброс позиции после полного круга (30 шагов)
  } else { // Шаг назад
    if (dec_pos == 0) dec_pos = 29; // Переход с 0 на 29
    else dec_pos--;
  }

  byte dec = dec_pos % 3; // Определение текущего пина для подачи импульса
  // Подача короткого импульса на соответствующий пин управления
  digitalWrite(KT0, LOW); digitalWrite(KT1, LOW); digitalWrite(KT2, LOW); digitalWrite(KT3, LOW);
  delayMicroseconds(cathodeDelay_mks);
  if (dec_pos == 0) digitalWrite(KT0, HIGH);
  else {
    if (dec == 0) digitalWrite(KT3, HIGH);
    if (dec == 1) digitalWrite(KT1, HIGH);
    if (dec == 2) digitalWrite(KT2, HIGH);
  }
}


