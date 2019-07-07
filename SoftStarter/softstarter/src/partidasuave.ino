#include "Arduino.h"
#include <stdint.h>
#include "main.h"

// mutex para interrupcoes
portMUX_TYPE timerMUX = portMUX_INITIALIZER_UNLOCKED;

volatile analise_fase_t r, s, t; // r usada quando mono
soft_cfg_t soft_cfg;

// IRAM_ATTR = coloca o codigo na ram interna (muito mais rapida)
void IRAM_ATTR isr_timer_r(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  r.f_aceita_disparo = 1;
  r.tempo_desl_gate = TEMPO_GATE_ACIONADO;
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
    timerAlarmWrite(r.tmr, r.tempo_prox_disparo, 0);
    timerAlarmEnable(r.tmr);
    digitalWrite(LED_VM, !digitalRead(LED_VM));
  }
  portEXIT_CRITICAL_ISR(&timerMUX);
}

// IRAM_ATTR = coloca o codigo na ram interna (muito mais rapida)
void IRAM_ATTR isr_timer_s(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  s.f_aceita_disparo = 1;
  s.tempo_desl_gate = TEMPO_GATE_ACIONADO;
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
    timerAlarmWrite(s.tmr, s.tempo_prox_disparo, 0);
    timerAlarmEnable(s.tmr);
    digitalWrite(LED_AM, !digitalRead(LED_AM));
  }
  portEXIT_CRITICAL_ISR(&timerMUX);
}

// IRAM_ATTR = coloca o codigo na ram interna (muito mais rapida)
void IRAM_ATTR isr_timer_t(void)
{
  portENTER_CRITICAL_ISR(&timerMUX);
  t.f_aceita_disparo = 1;
  t.tempo_desl_gate = TEMPO_GATE_ACIONADO;
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
    timerAlarmWrite(t.tmr, t.tempo_prox_disparo, 0);
    timerAlarmEnable(t.tmr);
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
  fase->tempo_desl_gate = 0;
  fase->tempo_prox_disparo = TEMPO_INIC_DISPARO;
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
        soft_cfg.tempo_soft = soft_cfg.tempo_soft*mult + (c-'0');
        mult *= 10;
      }
    }    
  } while (c != 10);
  Serial.println(soft_cfg.tempo_soft);
  
  Serial.println("Digite M para mono ou T para trifasico:");
  c = 0;
  do {
    if (Serial.available()) {
      c = Serial.read();
      if (c == 'M' && c == 'T') {
        soft_cfg.tipo_carga = c == 'M' ? 1 : 0;
      }
    }
  } while (c != 10);
  
  Serial.print("Tempo da partida suave [s]: ");
  Serial.println(soft_cfg.tipo_carga);
  if (soft_cfg.tipo_carga) {
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

  // inicializa o pino como entrada pulldown
  pinMode(X_ZERO_R, INPUT_PULLDOWN);
  pinMode(X_ZERO_S, INPUT_PULLDOWN);
  pinMode(X_ZERO_T, INPUT_PULLUP);
  
  //menu();
  soft_cfg.tempo_soft = 10;
  soft_cfg.tipo_carga = TIPO_CARGA_MONOFASICO;

  faseInic((analise_fase_t *)&r, ID_FASE_R, &isr_timer_r, &isr_xzero_r, X_ZERO_R);

  digitalWrite(LED, HIGH);
}

void faseHandler(analise_fase_t *fase, uint8_t pino)
{
  if (fase->f_dispara) {
    fase->f_dispara = 0;
    fase->f_desl_gate = 1;
    digitalWrite(pino, HIGH);
    // proximo ciclo = ciclo_atual - (66 - segundos_escolhidos)
    if (fase->tempo_prox_disparo > soft_cfg.tempo_soft && fase->tempo_prox_disparo <= TEMPO_INIC_DISPARO) {
      fase->tempo_prox_disparo -= soft_cfg.tempo_soft;
      fase->tempo_desl_gate = TEMPO_GATE_ACIONADO;
    }
    else {
      fase->tempo_prox_disparo = TEMPO_INIC_DISPARO+10;
      fase->tempo_desl_gate = TEMPO_GATE_ACIONADO*4;
    }
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

void loop()
{
  //timerStop(timer);
  //timerRestart(timer);
  faseHandler((analise_fase_t *)&r, CHAVE_R);
}
