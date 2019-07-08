#include "Arduino.h"
#include <stdint.h>
#include "main.h"

// mutex para interrupcoes
portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;

volatile analise_fase_t r, s, t; // r usada quando mono
volatile soft_cfg_t soft_cfg;

// IRAM_ATTR = coloca o codigo na ram interna (muito mais rapida)

// funcao de interrupcao do timer para fase R, dispara o triac
void IRAM_ATTR isr_timer_r(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  r.f_aceita_disparo = 1;
  r.f_dispara = 1;
  //digitalWrite(CHAVE_R, HIGH);
  //timerStop(r.tmr);
  //timerAlarmDisable(r.tmr);
  portEXIT_CRITICAL_ISR(&timerMUX);
}

/* FUNCAO INTERRUPCAO PARA O CROSSZERO NA FASE R */
void IRAM_ATTR isr_xzero_r(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  if (r.f_aceita_disparo) {
    r.f_aceita_disparo = 0;
    //timerWrite(r.tmr, r.tempo_liga);
    //timerRestart(r.tmr);
    timerWrite(r.tmr, 0);
    timerAlarmWrite(r.tmr, soft_cfg.tempo_prox_disparo, 0);
    timerAlarmEnable(r.tmr);
    soft_cfg.f_recalcula_disparo++;
    digitalWrite(LED_VM, !digitalRead(LED_VM));
  }
  portEXIT_CRITICAL_ISR(&timerMUX);
}

// funcao de interrupcao do timer para fase S, dispara o triac
void IRAM_ATTR isr_timer_s(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  s.f_aceita_disparo = 1;
  s.f_dispara = 1;
  //digitalWrite(CHAVE_R, HIGH);
  //timerStop(r.tmr);
  //timerAlarmDisable(r.tmr);
  portEXIT_CRITICAL_ISR(&timerMUX);
}

/* FUNCAO INTERRUPCAO PARA O CROSSZERO NA FASE R */
void IRAM_ATTR isr_xzero_s(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  if (s.f_aceita_disparo) {
    s.f_aceita_disparo = 0;
    //timerWrite(r.tmr, r.tempo_liga);
    //timerRestart(r.tmr);
    timerWrite(s.tmr, 0);
    timerAlarmWrite(s.tmr, soft_cfg.tempo_prox_disparo, 0);
    timerAlarmEnable(s.tmr);
    soft_cfg.f_recalcula_disparo++;
    digitalWrite(LED_AM, !digitalRead(LED_AM));
  }
  portEXIT_CRITICAL_ISR(&timerMUX);
}

// funcao de interrupcao do timer para fase R, dispara o triac
void IRAM_ATTR isr_timer_t(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  t.f_aceita_disparo = 1;
  t.f_dispara = 1;
  //digitalWrite(CHAVE_R, HIGH);
  //timerStop(r.tmr);
  //timerAlarmDisable(r.tmr);
  portEXIT_CRITICAL_ISR(&timerMUX);
}

/* FUNCAO INTERRUPCAO PARA O CROSSZERO NA FASE R */
void IRAM_ATTR isr_xzero_t(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  if (t.f_aceita_disparo) {
    t.f_aceita_disparo = 0;
    //timerWrite(r.tmr, r.tempo_liga);
    //timerRestart(r.tmr);
    timerWrite(t.tmr, 0);
    timerAlarmWrite(t.tmr, soft_cfg.tempo_prox_disparo, 0);
    timerAlarmEnable(t.tmr);
    soft_cfg.f_recalcula_disparo++;
    digitalWrite(LED_VD, !digitalRead(LED_VD));
  }
  portEXIT_CRITICAL_ISR(&timerMUX);
}


/*
 * nome: faseInic
 * parametros:
 *    fase: o endereco para uma struct do tipo fase
 *    id:   o id passado para o timer do esp32
 *    *cb_tmr:  endereco da funcao de interrupcao para estouro do timer
 *    *cb_x0:   endereco da funcao de interrupcao para passagem por zero
 *    pin_x0:   pino digital para deteccao da passagem por zero
 * retorno:
 *    void
 */
void faseInic(analise_fase_t *fase, uint8_t id, void (*cb_tmr)(), void (*cb_x0)(), uint8_t pin_x0)
{
  fase->f_desl_gate = 0;
  fase->f_dispara = 0;
  fase->tempo_desl_gate = 1;
  fase->f_aceita_disparo = 1;
  // timerBegin(o numero do timer, prescaler, up = true)
  // como o oscilador eh 80MHz, prescaler em 80 gera 
  // 1.000.000 incrementos por segundo
  fase->tmr = timerBegin(id, 80, 1);

  // recebe um ponteiro de um timer instanciado
  // o endereco da funcao que trata a interrupcao
  // true = edge 
  timerAttachInterrupt(fase->tmr, cb_tmr, true);
  // toda vez que o pino inverter o estado, tem um novo semi ciclo
  attachInterrupt(digitalPinToInterrupt(pin_x0), cb_x0, CHANGE);

  //timerAlarmWrite(r.tmr, TEMPO_INIC_ACIONAMENTO, 0);
  //timerAlarmWrite(fase->tmr, fase->tempo_liga, 1);
  //timerAlarmEnable(fase->tmr);
}

/*
 * handler para cada fase
 */
void faseHandler(analise_fase_t *fase, uint8_t pino)
{
  if (fase->f_dispara) {
    fase->f_dispara = 0;
    fase->f_desl_gate = 1;
    digitalWrite(pino, HIGH);    
    fase->tempo_desl_gate = soft_cfg.tempo_desliga_gate;
  }
  else {
    if (fase->f_desl_gate) {
      if (fase->tempo_desl_gate > 0) {
        fase->tempo_desl_gate--;
      }
      else {
        fase->tempo_desl_gate = 0;
        digitalWrite(pino, LOW);
      }
    }
  }
}

void menu(void)
{
  char c = 0;
  uint8_t mult = 1;

  Serial.begin(115200);
  
  Serial.print("Digite um valor de 1 a 65s para a partida: ");
  do {
    if (Serial.available()) {
      c = Serial.read();
      if (c >= '0' && c <= '9') {
        soft_cfg.tempo_soft = soft_cfg.tempo_soft*mult + (c-'0');
        mult *= 10;
      }
    }    
  } while (c != 10);
  if (soft_cfg.tempo_soft == 0 || soft_cfg.tempo_soft > 65) {
    soft_cfg.tempo_soft = 30; // padrao trinta segundos
  }
  Serial.println(soft_cfg.tempo_soft);
  c = 0;
  Serial.print("Digite M para mono ou T para trifasico: ");
  do {
    if (Serial.available()) {
      c = Serial.read();
      if (c == 'T') {
        soft_cfg.tipo_carga = TIPO_CARGA_TRIFASICO;
        Serial.println("Trifasico");
        c = 10; // forca sair do loop
      }
      else {
        soft_cfg.tipo_carga = TIPO_CARGA_MONOFASICO;
        Serial.println("Mnofasico");
        c = 10; // forca sair do loop
      }
    }    
  } while (c != 10);

  delay(100); // delay para dar tempo de imprimir a resposta
  Serial.end();
}

void softInic(void)
{
  soft_cfg.f_recalcula_disparo = 0;
  soft_cfg.tempo_prox_disparo = TEMPO_INIC_DISPARO;
  soft_cfg.tempo_desliga_gate = TEMPO_GATE_ACIONADO;
  soft_cfg.tempo_soft = 10; // 10 segundos
  soft_cfg.tipo_carga = TIPO_CARGA_MONOFASICO;
  // menu()

  //soft_cfg.tempo_soft = 66-soft_cfg.tempo_soft; // o decremento da soft por semi-ciclo
  soft_cfg.tempo_soft = (60/soft_cfg.tempo_soft);

  faseInic((analise_fase_t *)&r, ID_FASE_R, &isr_timer_r, &isr_xzero_r, X_ZERO_R);
  if (soft_cfg.tipo_carga == TIPO_CARGA_TRIFASICO) {
    faseInic((analise_fase_t *)&s, ID_FASE_S, &isr_timer_s, &isr_xzero_s, X_ZERO_S);
    faseInic((analise_fase_t *)&t, ID_FASE_T, &isr_timer_t, &isr_xzero_t, X_ZERO_T);
  }
}

void softHandler(void)
{ 
  // cada semi-ciclo incrementa um
  // quando for o mesmo numero incrementos que qtde de fases configuradas
  if (soft_cfg.f_recalcula_disparo >= soft_cfg.tipo_carga) {
    soft_cfg.f_recalcula_disparo = 0;

  // proximo ciclo = ciclo_atual - (66 - segundos_escolhidos)
    if (soft_cfg.tempo_prox_disparo > soft_cfg.tempo_soft && soft_cfg.tempo_prox_disparo <= TEMPO_INIC_DISPARO) {
      soft_cfg.tempo_prox_disparo -= soft_cfg.tempo_soft;
      soft_cfg.tempo_desliga_gate = TEMPO_GATE_ACIONADO;
    }
    else {
      soft_cfg.tempo_prox_disparo = TEMPO_INIC_DISPARO+1;
      soft_cfg.tempo_desliga_gate = TEMPO_GATE_ACIONADO*4;
    }
  }

  faseHandler((analise_fase_t *)&r, CHAVE_R);
  if (soft_cfg.tipo_carga == TIPO_CARGA_TRIFASICO) {
    faseHandler((analise_fase_t *)&s, CHAVE_S);
    faseHandler((analise_fase_t *)&t, CHAVE_T);
  }
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

  // inicializa o pino como entrada pulldown
  pinMode(X_ZERO_R, INPUT_PULLDOWN);
  pinMode(X_ZERO_S, INPUT_PULLDOWN);
  pinMode(X_ZERO_T, INPUT_PULLUP);

  digitalWrite(CHAVE_R, LOW);
  digitalWrite(CHAVE_S, LOW);
  digitalWrite(CHAVE_T, LOW);

  delay(200);
  
  menu();
  digitalWrite(LED, HIGH);

  softInic();
}

void loop()
{
  softHandler();
}