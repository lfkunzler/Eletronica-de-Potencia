#define LED 2

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
 
void setup() {
  Serial.begin(115200);
  // Set pin mode
  pinMode(LED,OUTPUT);

  menu();
}
 
void loop() {
  delay(500);
  digitalWrite(LED,HIGH);
  delay(500);
  digitalWrite(LED,LOW);
}
