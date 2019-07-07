#ifndef __INC_MAIN_H
#define __INC_MAIN_H

#include <stdint.h>
#include "Arduino.h"

#define LED       2
#define LED_VD    25
#define LED_AM    26
#define LED_VM    27

#define X_ZERO_R  4
#define X_ZERO_S  16
#define X_ZERO_T  17

#define CHAVE_R   21
#define CHAVE_S   18
#define CHAVE_T   19

#define ID_FASE_R   0
#define ID_FASE_S   1
#define ID_FASE_T   2

#define TEMPO_MIN_SOFT    1  // segundo
#define TEMPO_MAX_SOFT    65 // segundos

#define TEMPO_GATE_ACIONADO   1000
#define TEMPO_INIC_DISPARO    7800 // microssegundos (depende do cross)

#define TIPO_CARGA_MONOFASICO   0
#define TIPO_CARGA_TRIFASICO    1

typedef struct  {
  hw_timer_t *tmr;
  uint8_t f_dispara;
  uint8_t f_desl_gate;
  uint16_t tempo_prox_disparo;
  uint16_t tempo_desl_gate;
  uint8_t f_aceita_disparo; // se aceita um novo disparo
} analise_fase_t;

typedef struct {
  uint8_t tempo_soft; // recebe o tempo do usuario e depois converte
  uint8_t tipo_carga; // mono ou tri
} soft_cfg_t;

// how to use platformIO: https://docs.platformio.org/en/latest/ide/vscode.html#quick-start

void faseInic(analise_fase_t *fase);

#endif
