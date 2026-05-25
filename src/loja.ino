#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

const int BTN_DOWN = 2;
const int BTN_UP = 3;
const int BTN_BACK = 4;
const int BTN_OK = 5;

const int LED_VERDE = 6;
const int LED_VERMELHO = 7;
const int BUZZER = 8;

String regras[] = {
  "1. Use botao 1 para baixo",
  "2. Use botao 2 para cima",
  "3. Use botao 3 para voltar",
  "4. Use botao 4 para confirmar"
};

int indice = 0;
int total_opcoes = 4;

void confirmacao() {
  digitalWrite(LED_VERDE, HIGH);   
  delay(1000);

  tone(BUZZER, 784);   // Sol
  delay(120);
  noTone(BUZZER);
  delay(40);

  tone(BUZZER, 988);   // Si
  delay(120);
  noTone(BUZZER);
  delay(40);

  tone(BUZZER, 1175);  // Re
  delay(180);
  noTone(BUZZER);
  delay(40);

  tone(BUZZER, 1568);  // Sol agudo
  delay(280);
  noTone(BUZZER);
}

void setup() {
  lcd.begin(16, 2);
  lcd.setBacklight(1);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  inicializacao();
}

void loop() {
  navegacao_regras();
}

void exibir_texto_grande(String texto, int coluna, int linha) {
  lcd.clear();
  lcd.home();

  lcd.setCursor(coluna, linha);
  lcd.print(texto);

  delay(1000);

  int quantidade_scroll = texto.length() - 16;

  if (quantidade_scroll < 0) {
    quantidade_scroll = 0;
  }

  for (int i = 0; i < quantidade_scroll; i++) {
    lcd.scrollDisplayLeft();
    delay(250);
  }

  delay(1000);

  lcd.clear();
  lcd.home();
}

void escrever_linha(String texto, int linha) {
  lcd.setCursor(0, linha);
  lcd.print("                ");

  lcd.setCursor(0, linha);
  lcd.print(texto);
}

void inicializacao() {
  exibir_texto_grande("Seja bem-vindo a nossa loja!", 0, 0);
  exibir_texto_grande("A seguir serao exibidas as regras!", 0, 0);

  mostrar_regras();
}

void navegacao_regras() {
  if (digitalRead(BTN_DOWN) == LOW) {
    indice++;

    if (indice > total_opcoes - 1) {
      indice = 0;
    }

    mostrar_regras();
    esperar_soltar(BTN_DOWN);
  }

  if (digitalRead(BTN_UP) == LOW) {
    indice--;

    if (indice < 0) {
      indice = total_opcoes - 1;
    }

    mostrar_regras();
    esperar_soltar(BTN_UP);
  }
}

void mostrar_regras() {
  lcd.clear();
  lcd.home();

  String titulo = String(total_opcoes) + " regras:";
  String texto = regras[indice];

  lcd.setCursor(0, 0);
  lcd.print(titulo);

  lcd.setCursor(0, 1);
  lcd.print(texto);

  delay(1000);

  int quantidade_scroll = texto.length() - 16;

  if (quantidade_scroll < 0) {
    quantidade_scroll = 0;
  }

  for (int i = 0; i < quantidade_scroll; i++) {
    lcd.scrollDisplayLeft();
    delay(250);
  }

  delay(800);
}

void esperar_soltar(int botao) {
  delay(150);

  while (digitalRead(botao) == LOW) {
    delay(10);
  }

  delay(150);
}