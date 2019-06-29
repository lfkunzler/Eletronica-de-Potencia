#define LED       2
#define LED_VD    25
#define LED_AM    26
#define LED_VM    27

volatile int interruptCounter;
int totalInterruptCounter;

hw_timer_t *timer = NULL;
portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;
// interrupcao timer
void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMUX);
  digitalWrite(LED, !digitalRead(LED));
  portEXIT_CRITICAL_ISR(&timerMUX);
}

typedef struct {
  uint8_t tempo : 7; // 0 a 15 s
  uint8_t tipo  : 1; // mono ou tri
} soft_cfg_t;

soft_cfg_t soft_cfg = {0, 0};

void menu(void)
{
  char c = 0;
  uint8_t mult = 1;
  
  Serial.print("Digite um valor de 0 a 15s para a partida: ");
  do {
    if (Serial.available()) {
      c = Serial.read();
      if (c >= '0' && c <= '9') {
        soft_cfg.tempo = soft_cfg.tempo*mult + (c-'0');
        mult *= 10;
      }
    }    
  } while (c != 10);
  Serial.println(soft_cfg.tempo);
  
  Serial.println("Digite M para mono ou T para trifasico:");
  c = 0;
  do {
    if (Serial.available()) {
      c = Serial.read();
      if (c == 'M' && c == 'T') {
        soft_cfg.tipo = c == 'M' ? 0 : 1;
      }
    }
  } while (c != 10);
  
  Serial.print("Tempo da partida suave [s]: ");
  Serial.println(soft_cfg.tempo);
  if (soft_cfg.tipo) {
    Serial.println("Trifasico.");
  }
  else {
    Serial.println("Monofasico.");
  }
  
}
 
void setup() 
{
  Serial.begin(115200);
  // Set pin mode
  pinMode(LED,OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(LED_AM, OUTPUT);
  pinMode(LED_VM, OUTPUT);

  //menu();

  // timerBegin(o numero do timer, prescaler, up = true)
  // como o oscilador eh 80MHz, prescaler em 80 gera 
  // 1000 incrementos por segundo
  timer = timerBegin(0, 80, 1);
  // recebe um ponteiro de um timer instanciado
  // o endereco da funcao que trata a interrupcao
  // true = edge 
  timerAttachInterrupt(timer, &onTimer, 1);
  // a cada 1000ms, dispara a interrupcao
  timerAlarmWrite(timer, 1000000, 1);
  timerAlarmEnable(timer);
}
 
void loop() {
  digitalWrite(LED_VD,HIGH);
  delay(1000);
  digitalWrite(LED_VD,LOW);
  digitalWrite(LED_AM,HIGH);
  delay(1000);
  digitalWrite(LED_AM,LOW);
  digitalWrite(LED_VM,HIGH);
  delay(1000);
  digitalWrite(LED_VM,LOW);
}
