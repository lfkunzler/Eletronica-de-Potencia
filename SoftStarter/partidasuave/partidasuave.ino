#define LED       2
#define LED_VD    25
#define LED_AM    26
#define LED_VM    27

#define X_ZERO_R  4
#define X_ZERO_S  16
#define X_ZERO_T  17

#define CHAVE_R   5
#define CHAVE_S   18
#define CHAVE_T   19

#define TEMPO_GATE_ACIONADO 1000

typedef struct  {
  hw_timer_t *tmr;
  uint16_t tempo_desliga;
  uint8_t flag_liga;
  uint8_t flag_desliga;
} analise_fase_t;

typedef struct {
  uint8_t tempo; // 0 a 15 s
  uint8_t tipo; // mono ou tri
} soft_cfg_t;

analise_fase_t r; // MONO POR ENQUANTO
portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;
soft_cfg_t soft_cfg = {0, 0};

// IRAM_ATTR = coloca o codigo na ram interna
// muito mais rapida
// interrupcao timer
void IRAM_ATTR isr_timer_r(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  r.tempo_desliga = TEMPO_GATE_ACIONADO;
  r.flag_liga = 1;
  //digitalWrite(CHAVE_R, HIGH);
  timerStop(r.tmr);
  portEXIT_CRITICAL_ISR(&timerMUX);
}

/* FUNCAO INTERRUPCAO PARA O CROSSZERO NA FASE R */
void IRAM_ATTR isr_xzero_r(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);  
  timerRestart(r.tmr);
  digitalWrite(LED_VM, !digitalRead(LED_VM));
  portEXIT_CRITICAL_ISR(&timerMUX);
}

void menu(void)
{
  char c = 0;
  uint8_t mult = 1;

  Serial.begin(115200);
  
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
        soft_cfg.tipo = c == 'M' ? 1 : 0;
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
  Serial.end();
}

void setup() 
{
  // Set pin mode
  pinMode(LED,OUTPUT);
  pinMode(LED_VD, OUTPUT);
  pinMode(LED_AM, OUTPUT);
  pinMode(LED_VM, OUTPUT);

  pinMode(CHAVE_R, OUTPUT);
  pinMode(CHAVE_S, OUTPUT);
  pinMode(CHAVE_T, OUTPUT);

  pinMode(X_ZERO_R, INPUT_PULLUP);
  pinMode(X_ZERO_S, INPUT_PULLUP);
  pinMode(X_ZERO_T, INPUT_PULLUP);

  //menu();

  // timerBegin(o numero do timer, prescaler, up = true)
  // como o oscilador eh 80MHz, prescaler em 80 gera 
  // 1000000 incrementos por segundo
  r.tmr = timerBegin(0, 80, 1);
  // recebe um ponteiro de um timer instanciado
  // o endereco da funcao que trata a interrupcao
  // true = edge 
  timerAttachInterrupt(r.tmr, &isr_timer_r, 1);
  // toda vez que o pino inverter o estado, tem um novo semi ciclo
  attachInterrupt(digitalPinToInterrupt(X_ZERO_R), isr_xzero_r, CHANGE);
  // a cada 1ms, dispara a interrupcao
  timerAlarmWrite(r.tmr, 7700, 1);
  timerAlarmEnable(r.tmr);

  digitalWrite(LED, HIGH);
}

void faseHandler(analise_fase_t *fase, uint8_t pino)
{
  if (fase->flag_liga) {
    fase->flag_liga = 0;
    fase->flag_desliga = 1;
    fase->tempo_desliga = TEMPO_GATE_ACIONADO;
    digitalWrite(pino, HIGH);
  }
  else {
    if (fase->flag_desliga && fase->tempo_desliga == 0) {
      fase->flag_desliga = 0;
      digitalWrite(pino, LOW);
    }
    else {
      r.tempo_desliga--;
    }
  }
}

void loop()
{
  //timerStop(timer);
  //timerRestart(timer);
  faseHandler(&r, CHAVE_R);
}
