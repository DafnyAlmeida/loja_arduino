// Bibliotecas
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

// Componentes
const int BTN_DOWN = 2;
const int BTN_UP = 3;
const int BTN_BACK = 4;
const int BTN_OK = 5;

const int LED_VERDE = 6;
const int LED_VERMELHO = 7;
const int BUZZER = 8;

// Funções para mostrat texto no lcd
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

void exibir_texto_pequeno(String texto, int linha) {
  lcd.setCursor(0, linha);
  lcd.print("                ");

  lcd.setCursor(0, linha);
  lcd.print(texto);
}

void processando() {
  exibir_texto_pequeno("Iniciando...", 0);
};

// Regras da inicialização
String regras[] = {
  "1 Baixo: btn 1",
  "2 Cima: btn 2",
  "3 Voltar: btn 3",
  "4 OK: btn 4"
};

int total_opcoes = 4;

String menu[] = {
  "Ver produtos",
  "Adicionar",
  "Excluir",
  "Ver carrinho",
  "Pagar",
  "Ver regras"
};

int indice_menu = 0;
int total_opcoes_menu = 6;

// Funções de erro e confirmação
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

void erro() {
  digitalWrite(LED_VERMELHO, HIGH);
  delay(1000);

  tone(BUZZER, 392);   // Sol grave
  delay(180);
  noTone(BUZZER);
  delay(80);

  tone(BUZZER, 330);   // Mi
  delay(180);
  noTone(BUZZER);
  delay(80);

  tone(BUZZER, 262);   // Dó
  delay(280);
  noTone(BUZZER);

  digitalWrite(LED_VERMELHO, LOW);
}

// Inicio do código

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

void inicializacao() {
  exibir_texto_grande("Seja bem-vindo a nossa loja!", 0, 0);
  exibir_texto_grande("A seguir serao exibidas as regras!", 0, 0);

  mostrar_regras();
  delay(200);
  exibir_texto_grande("Voce sera encaminhado para o menu!", 0, 0);
  delay(200);
  lcd.clear();
  processando();
  delay(1000);
  mostrar_menu();
}

void mostrar_regras() {
  lcd.clear();
  lcd.home();

  lcd.setCursor(0, 0);
  lcd.print("Regras:");

  for (int i = 0; i < total_opcoes; i++) {
    exibir_texto_pequeno(regras[i], 1);
    delay(1500);
  }

  lcd.clear();
}

// Função menu
void mostrar_menu() {
  lcd.clear();
  lcd.home();

  String titulo = "Menu " + String(indice_menu + 1) + "/" + String(total_opcoes_menu);
  String texto = menu[indice_menu];

  lcd.setCursor(0, 0);
  lcd.print(titulo);

  lcd.setCursor(0, 1);
  lcd.print(texto);
};

void loop() {
  navegacao_menu();
}

void navegacao_menu() {
  if (digitalRead(BTN_DOWN) == LOW) {
    indice_menu++;

    if (indice_menu > total_opcoes_menu - 1) {
      indice_menu = 0;
    }

    mostrar_menu();
    esperar_soltar(BTN_DOWN);
  }

  if (digitalRead(BTN_UP) == LOW) {
    indice_menu--;

    if (indice_menu < 0) {
      indice_menu = total_opcoes_menu - 1;
    }

    mostrar_menu();
    esperar_soltar(BTN_UP);
  }

  if (digitalRead(BTN_OK) == LOW) {
    executar_opcao_menu();
    esperar_soltar(BTN_OK);
  }
}

void esperar_soltar(int botao) {
  delay(150);

  while (digitalRead(botao) == LOW) {
    delay(10);
  }

  delay(150);
}

void executar_opcao_menu() {
  if (indice_menu == 0) {
    ver_produtos();
  } 
  else if (indice_menu == 1) {
    adicionar_produto();
  } 
  else if (indice_menu == 2) {
    excluir_produto();
  } 
  else if (indice_menu == 3) {
    ver_carrinho();
  } 
  else if (indice_menu == 4) {
    pagar();
  } 
  else if (indice_menu == 5) {
    lcd.clear();
    processando();
    delay(1000);
    mostrar_regras();
    mostrar_menu();
  }
}

// Funções (ações) do projeto 
void ver_produtos() {
  //
};

void adicionar_produto() {
  //
};

void ver_carrinho() {
  //
};

void excluir_produto() {
  //
};

void pagar() {
  //
};