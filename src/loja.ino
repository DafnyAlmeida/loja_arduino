// --------------- ⚙️ Olá, seja bem-vindo(a) à nossa loja! ---------------- //
// Código otimizado para economizar memória e evitar repetição.
// Desenvolvido por Alberto Kayron, Dafny Sabino, Icaro Pereira,
// Luna Freitas e Eduarda Andrade.
// ---------------------------------------------------------------------------

// Bibliotecas
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
#include <avr/pgmspace.h>

Adafruit_LiquidCrystal lcd(0);

// Componentes
const byte BTN_DOWN = 2;
const byte BTN_UP = 3;
const byte BTN_BACK = 4;
const byte BTN_OK = 5;

const byte LED_VERDE = 6;
const byte LED_VERMELHO = 7;
const byte BUZZER = 8;

// Constantes gerais
const byte TOTAL_REGRAS = 4;
const byte TOTAL_OPCOES_MENU = 6;
const byte QUANTIDADE_PRODUTOS = 3;
const byte CANCELADO = 255;
const unsigned long TEMPO_LIMITE_SERIAL = 15000; // 15 segundos
const unsigned long TEMPO_LIMITE_BOTAO = 1500;  // 1,5 segundo

byte indice_menu = 0;

// Buffer reutilizado para carregar somente um texto por vez da Flash.
// Isso evita guardar várias strings na RAM.
char textoBuffer[40];

// ---------------------------------------------------------------------------
// TEXTOS SALVOS NA FLASH (PROGMEM)
// ---------------------------------------------------------------------------

const char regra0[] PROGMEM = "1 Baixo: btn 1";
const char regra1[] PROGMEM = "2 Cima: btn 2";
const char regra2[] PROGMEM = "3 Voltar: btn 3";
const char regra3[] PROGMEM = "4 OK: btn 4";

const char* const regras[] PROGMEM = {
  regra0, regra1, regra2, regra3
};

const char menu0[] PROGMEM = "Ver produtos";
const char menu1[] PROGMEM = "Adicionar";
const char menu2[] PROGMEM = "Excluir";
const char menu3[] PROGMEM = "Ver carrinho";
const char menu4[] PROGMEM = "Pagar";
const char menu5[] PROGMEM = "Ver regras";

const char* const menu[] PROGMEM = {
  menu0, menu1, menu2, menu3, menu4, menu5
};

const char produto0[] PROGMEM = "Arroz";
const char produto1[] PROGMEM = "Feijao";
const char produto2[] PROGMEM = "Leite";

const char* const produtos[] PROGMEM = {
  produto0, produto1, produto2
};

// ---------------------------------------------------------------------------
// PRODUTOS E CARRINHO
// ---------------------------------------------------------------------------

// O estoque muda durante o programa, portanto fica na RAM.
byte quantidades[QUANTIDADE_PRODUTOS] = {6, 8, 5};

// Dados fixos ficam na memória Flash.
const byte ids[] PROGMEM = {1, 2, 3};

// Valores em centavos para evitar uso de float.
const unsigned int precosCentavos[] PROGMEM = {1250, 1300, 1150};

// A quantidade no carrinho usa o mesmo índice da lista de produtos.
byte quantidadesCarrinho[QUANTIDADE_PRODUTOS] = {0, 0, 0};

// Guarda quantos tipos diferentes de produto estão no carrinho.
byte tiposNoCarrinho = 0;

// Total financeiro do carrinho.
unsigned long precoCarrinhoCentavos = 0;

// ---------------------------------------------------------------------------
// PROTÓTIPOS
// ---------------------------------------------------------------------------

void copiar_texto_flash(const char* const tabela[], byte indice);

void limpar_linha(byte linha);
void mostrar_duas_linhas_P(PGM_P linha1, PGM_P linha2);

void exibir_texto_grande_P(PGM_P texto, byte coluna, byte linha);
void exibir_texto_pequeno(const char* texto, byte linha);
void exibir_texto_pequeno_P(PGM_P texto, byte linha);

byte obter_id(byte indiceProduto);
unsigned int obter_preco(byte indiceProduto);
void imprimir_preco(unsigned long valorCentavos);

byte buscar_produto_por_id(byte id);
int ler_numero_serial();
byte pedir_indice_produto_P(PGM_P linha1, PGM_P linha2);
byte pedir_quantidade_valida_P(PGM_P linha1, PGM_P linha2, byte limite, PGM_P textoLimite);

int encontrar_indice_carrinho(byte posicao);
void exibir_produto_carrinho(byte posicao);
void limpar_carrinho(bool devolverEstoque);

void confirmacao();
void erro();

bool voltar();

void inicializacao();
void mostrar_regras();

void mostrar_menu();
void navegacao_menu();
void esperar_soltar(byte botao);
void executar_opcao_menu();

void ver_produtos();
void adicionar_produto();
void ver_carrinho();
void excluir_produto();
void pagar();
void inicio_carrinho();

// ---------------------------------------------------------------------------
// FUNÇÕES AUXILIARES
// ---------------------------------------------------------------------------

void copiar_texto_flash(const char* const tabela[], byte indice) {
  PGM_P enderecoTexto = (PGM_P) pgm_read_word(&(tabela[indice]));
  strncpy_P(textoBuffer, enderecoTexto, sizeof(textoBuffer) - 1);
  textoBuffer[sizeof(textoBuffer) - 1] = '\0';
}

void limpar_linha(byte linha) {
  lcd.setCursor(0, linha);
  lcd.print(F("                "));
}

void mostrar_duas_linhas_P(PGM_P linha1, PGM_P linha2) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(linha1));

  lcd.setCursor(0, 1);
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(linha2));
}

// Exibe texto salvo na Flash e faz scroll quando passar de 16 colunas.
void exibir_texto_grande_P(PGM_P texto, byte coluna, byte linha) {
  lcd.clear();
  lcd.home();

  lcd.setCursor(coluna, linha);
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(texto));

  delay(1000);

  int quantidade_scroll = strlen_P(texto) - 16;

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

void exibir_texto_pequeno(const char* texto, byte linha) {
  limpar_linha(linha);
  lcd.setCursor(0, linha);
  lcd.print(texto);
}

void exibir_texto_pequeno_P(PGM_P texto, byte linha) {
  limpar_linha(linha);
  lcd.setCursor(0, linha);
  lcd.print(reinterpret_cast<const __FlashStringHelper*>(texto));
}

byte obter_id(byte indiceProduto) {
  return pgm_read_byte(&(ids[indiceProduto]));
}

unsigned int obter_preco(byte indiceProduto) {
  return pgm_read_word(&(precosCentavos[indiceProduto]));
}

void imprimir_preco(unsigned long valorCentavos) {
  lcd.print(valorCentavos / 100);
  lcd.print(',');

  byte centavos = valorCentavos % 100;

  if (centavos < 10) {
    lcd.print('0');
  }

  lcd.print(centavos);
}

byte buscar_produto_por_id(byte id) {
  for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
    if (id == obter_id(i)) {
      return i;
    }
  }

  return CANCELADO;
}

// Lê número pelo Serial Monitor.
// Se o usuário apertar voltar antes de digitar, retorna -1.
int ler_numero_serial() {
  unsigned long inicioEspera = millis();

  while (Serial.available() == 0) {
    if (voltar()) {
      return -1;
    }

    // Se ninguém digitar nada no Serial Monitor, volta para o menu.
    // Isso evita a impressão de que o Arduino entrou em loop infinito.
    if (millis() - inicioEspera >= TEMPO_LIMITE_SERIAL) {
      mostrar_duas_linhas_P(PSTR("Tempo esgotado"), PSTR("Voltando menu"));
      delay(900);
      mostrar_menu();
      return -1;
    }

    delay(10);
  }

  int valor = Serial.parseInt();

  while (Serial.available() > 0) {
    Serial.read();
  }

  return valor;
}

// Pede o ID do produto e retorna o índice dele.
// Retorna CANCELADO caso o usuário aperte voltar.
byte pedir_indice_produto_P(PGM_P linha1, PGM_P linha2) {
  while (true) {
    mostrar_duas_linhas_P(linha1, linha2);

    int idDigitado = ler_numero_serial();

    if (idDigitado == -1) {
      return CANCELADO;
    }

    if (idDigitado >= 0 && idDigitado <= 255) {
      byte indiceProduto = buscar_produto_por_id((byte) idDigitado);

      if (indiceProduto != CANCELADO) {
        return indiceProduto;
      }
    }

    mostrar_duas_linhas_P(PSTR("ID invalido"), PSTR("Tente novamente"));
    delay(900);
  }
}

// Pede uma quantidade válida entre 1 e o limite.
// Retorna CANCELADO caso o usuário aperte voltar.
byte pedir_quantidade_valida_P(PGM_P linha1, PGM_P linha2, byte limite, PGM_P textoLimite) {
  while (true) {
    mostrar_duas_linhas_P(linha1, linha2);

    int quantidadeDigitada = ler_numero_serial();

    if (quantidadeDigitada == -1) {
      return CANCELADO;
    }

    if (quantidadeDigitada > 0 && quantidadeDigitada <= limite) {
      return (byte) quantidadeDigitada;
    }

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(F("Qtd invalida"));

    lcd.setCursor(0, 1);
    lcd.print(reinterpret_cast<const __FlashStringHelper*>(textoLimite));
    lcd.print(limite);

    delay(900);
  }
}

// Converte a página do carrinho no índice original do produto.
// Somente produtos com quantidade maior que zero são considerados.
int encontrar_indice_carrinho(byte posicao) {
  byte posicaoAtual = 0;

  for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
    if (quantidadesCarrinho[i] > 0) {
      if (posicaoAtual == posicao) {
        return i;
      }

      posicaoAtual++;
    }
  }

  return -1;
}

void exibir_produto_carrinho(byte posicao) {
  int indiceProduto = encontrar_indice_carrinho(posicao);

  if (indiceProduto == -1) {
    return;
  }

  copiar_texto_flash(produtos, indiceProduto);

  limpar_linha(1);
  lcd.setCursor(0, 1);

  lcd.print(obter_id(indiceProduto));
  lcd.print(' ');

  // Usa somente 6 caracteres do nome para caber no LCD 16x2.
  for (byte i = 0; i < 6; i++) {
    if (textoBuffer[i] != '\0') {
      lcd.print(textoBuffer[i]);
    } 
    else {
      lcd.print(' ');
    }
  }

  lcd.print(' ');
  imprimir_preco(obter_preco(indiceProduto));
  lcd.print(' ');
  lcd.print(quantidadesCarrinho[indiceProduto]);
}

// Limpa o carrinho.
// Se devolverEstoque for true, devolve os produtos para o estoque.
void limpar_carrinho(bool devolverEstoque) {
  for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
    if (devolverEstoque) {
      quantidades[i] += quantidadesCarrinho[i];
    }

    quantidadesCarrinho[i] = 0;
  }

  tiposNoCarrinho = 0;
  precoCarrinhoCentavos = 0;
}

// ---------------------------------------------------------------------------
// FUNÇÕES DE SOM, ERRO E CONFIRMAÇÃO
// ---------------------------------------------------------------------------


void confirmacao() {
  digitalWrite(LED_VERDE, HIGH);

  tone(BUZZER, 700, 120);
  delay(150);

  tone(BUZZER, 1000, 120);
  delay(150);

  tone(BUZZER, 1400, 250);
  delay(280);

  noTone(BUZZER);
  digitalWrite(LED_VERDE, LOW);
}

void erro() {
  digitalWrite(LED_VERMELHO, HIGH);

  tone(BUZZER, 250, 250);
  delay(1000);

  tone(BUZZER, 180, 350);
  delay(1000);

  noTone(BUZZER);
  digitalWrite(LED_VERMELHO, LOW);
}

// Botão voltar.
bool voltar() {
  if (digitalRead(BTN_BACK) == LOW) {
    esperar_soltar(BTN_BACK);
    mostrar_menu();
    return true;
  }

  return false;
}

void inicio_carrinho() {
  lcd.clear();

  exibir_texto_pequeno_P(PSTR("Total produtos:"), 0);

  lcd.setCursor(0, 1);
  lcd.print(tiposNoCarrinho);
  lcd.print(F(" produtos"));

  delay(900);

  lcd.clear();
}

// ---------------------------------------------------------------------------
// SETUP E LOOP
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

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
  navegacao_menu();
}

// ---------------------------------------------------------------------------
// INICIALIZAÇÃO
// ---------------------------------------------------------------------------

void inicializacao() {
  // Inicializacao leve, mantendo as regras obrigatorias.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Bem-vindo(a)!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Regras a seguir"));
  delay(1000);

  mostrar_regras();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Abrindo menu..."));
  delay(700);

  mostrar_menu();
}

void mostrar_regras() {
  lcd.clear();
  lcd.home();

  lcd.setCursor(0, 0);
  lcd.print(F("Regras:"));

  for (byte i = 0; i < TOTAL_REGRAS; i++) {
    copiar_texto_flash(regras, i);
    exibir_texto_pequeno(textoBuffer, 1);
    delay(900);
  }

  lcd.clear();
}

// ---------------------------------------------------------------------------
// MENU
// ---------------------------------------------------------------------------

void mostrar_menu() {
  lcd.clear();
  lcd.home();

  lcd.setCursor(0, 0);
  lcd.print(F("Menu "));
  lcd.print(indice_menu + 1);
  lcd.print('/');
  lcd.print(TOTAL_OPCOES_MENU);

  copiar_texto_flash(menu, indice_menu);

  lcd.setCursor(0, 1);
  lcd.print(textoBuffer);
}

void navegacao_menu() {
  if (digitalRead(BTN_DOWN) == LOW) {
    indice_menu++;

    if (indice_menu >= TOTAL_OPCOES_MENU) {
      indice_menu = 0;
    }

    mostrar_menu();
    esperar_soltar(BTN_DOWN);
  }

  if (digitalRead(BTN_UP) == LOW) {
    if (indice_menu == 0) {
      indice_menu = TOTAL_OPCOES_MENU - 1;
    } 
    else {
      indice_menu--;
    }

    mostrar_menu();
    esperar_soltar(BTN_UP);
  }

  if (digitalRead(BTN_OK) == LOW) {
    esperar_soltar(BTN_OK);
    executar_opcao_menu();
  }
}

void esperar_soltar(byte botao) {
  delay(80);

  unsigned long inicio = millis();

  // Evita travamento caso algum botão fique preso ou ligado errado.
  while (digitalRead(botao) == LOW && millis() - inicio < TEMPO_LIMITE_BOTAO) {
    delay(10);
  }

  delay(120);
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
    mostrar_regras();
    mostrar_menu();
  }
}

// ---------------------------------------------------------------------------
// FUNÇÕES PRINCIPAIS DO SISTEMA
// ---------------------------------------------------------------------------

void ver_produtos() {

  byte indiceProduto = 0;

  while (true) {
    lcd.clear();

    copiar_texto_flash(produtos, indiceProduto);

    lcd.setCursor(0, 0);
    lcd.print(F("ID: "));
    lcd.print(obter_id(indiceProduto));
    lcd.print(' ');
    lcd.print(textoBuffer);

    lcd.setCursor(0, 1);
    lcd.print(F("Qtd: "));
    lcd.print(quantidades[indiceProduto]);
    lcd.print(F(" R$"));
    imprimir_preco(obter_preco(indiceProduto));

    while (true) {
      if (voltar()) {
        return;
      }

      if (digitalRead(BTN_DOWN) == LOW) {
        esperar_soltar(BTN_DOWN);

        indiceProduto++;

        if (indiceProduto >= QUANTIDADE_PRODUTOS) {
          indiceProduto = 0;
        }

        break;
      }

      if (digitalRead(BTN_UP) == LOW) {
        esperar_soltar(BTN_UP);

        if (indiceProduto == 0) {
          indiceProduto = QUANTIDADE_PRODUTOS - 1;
        } 
        else {
          indiceProduto--;
        }

        break;
      }

      delay(10);
    }
  }
}

void adicionar_produto() {

  byte indiceProduto = pedir_indice_produto_P(PSTR("Digite o ID"), PSTR("do produto:"));

  if (indiceProduto == CANCELADO) {
    return;
  }

  if (quantidades[indiceProduto] == 0) {
    exibir_texto_grande_P(PSTR("Produto sem estoque!"), 0, 0);
    mostrar_menu();
    return;
  }

  byte quantidadeProduto = pedir_quantidade_valida_P(
    PSTR("Digite a"),
    PSTR("quantidade:"),
    quantidades[indiceProduto],
    PSTR("Estoque: ")
  );

  if (quantidadeProduto == CANCELADO) {
    return;
  }

  if (quantidadesCarrinho[indiceProduto] == 0) {
    tiposNoCarrinho++;
  }

  quantidadesCarrinho[indiceProduto] += quantidadeProduto;
  quantidades[indiceProduto] -= quantidadeProduto;

  precoCarrinhoCentavos += (unsigned long) quantidadeProduto * obter_preco(indiceProduto);
  exibir_texto_grande_P(PSTR("Produto adicionado!"), 0, 0);
  mostrar_menu();
}

void ver_carrinho() {
  byte pagina_atual = 0;

  if (tiposNoCarrinho == 0) {
    exibir_texto_grande_P(PSTR("Carrinho vazio!"), 0, 0);
    erro();
    mostrar_menu();
    return;
  }

  inicio_carrinho();

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print(F("ID|NOME|PRECO|Q"));

    if (pagina_atual < tiposNoCarrinho) {
      exibir_produto_carrinho(pagina_atual);
    } 
    else {
      limpar_linha(1);
      lcd.setCursor(0, 1);
      lcd.print(F("Total R$ "));
      imprimir_preco(precoCarrinhoCentavos);
    }

    while (true) {
      if (voltar()) {
        return;
      }

      if (digitalRead(BTN_DOWN) == LOW) {
        esperar_soltar(BTN_DOWN);

        // Vai até a página do total, mas não passa dela.
        if (pagina_atual < tiposNoCarrinho) {
          pagina_atual++;
        }

        break;
      }

      if (digitalRead(BTN_UP) == LOW) {
        esperar_soltar(BTN_UP);

        if (pagina_atual > 0) {
          pagina_atual--;
        }

        break;
      }

      delay(10);
    }
  }
}

void excluir_produto() {

  if (tiposNoCarrinho == 0) {
    exibir_texto_grande_P(PSTR("Carrinho vazio!"), 0, 0);
    erro();
    mostrar_menu();
    return;
  }

  byte indiceProduto = pedir_indice_produto_P(PSTR("Excluir: ID do"), PSTR("produto:"));

  if (indiceProduto == CANCELADO) {
    return;
  }

  if (quantidadesCarrinho[indiceProduto] == 0) {
    exibir_texto_grande_P(PSTR("Nao esta no carrinho!"), 0, 0);
    mostrar_menu();
    return;
  }

  byte quantidadeRemover = pedir_quantidade_valida_P(
    PSTR("Qtd para remover"),
    PSTR("do carrinho:"),
    quantidadesCarrinho[indiceProduto],
    PSTR("No carrinho: ")
  );

  if (quantidadeRemover == CANCELADO) {
    return;
  }

  quantidadesCarrinho[indiceProduto] -= quantidadeRemover;
  quantidades[indiceProduto] += quantidadeRemover;

  precoCarrinhoCentavos -= (unsigned long) quantidadeRemover * obter_preco(indiceProduto);

  if (quantidadesCarrinho[indiceProduto] == 0) {
    tiposNoCarrinho--;
  }
  exibir_texto_grande_P(PSTR("Produto removido!"), 0, 0);
  mostrar_menu();
}

void pagar() {
  if (tiposNoCarrinho == 0) {
    exibir_texto_grande_P(PSTR("Carrinho vazio!"), 0, 0);
    erro();
    mostrar_menu();
    return;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(F("Total: R$ "));
  imprimir_preco(precoCarrinhoCentavos);

  lcd.setCursor(0, 1);
  lcd.print(F("4:OK 3:Cancela"));

  while (true) {
    // Botão 4 confirma a compra.
    if (digitalRead(BTN_OK) == LOW) {
      esperar_soltar(BTN_OK);
      break;
    }

    // Botão 3 cancela a compra.
    // Como a compra foi cancelada, os itens voltam para o estoque.
    if (digitalRead(BTN_BACK) == LOW) {
      esperar_soltar(BTN_BACK);

      limpar_carrinho(true);

      erro();
      exibir_texto_grande_P(PSTR("Compra cancelada!"), 0, 0);
      mostrar_menu();
      return;
    }

    delay(10);
  }
  confirmacao();
  exibir_texto_grande_P(PSTR("Compra realizada!"), 0, 0);

  // Compra realizada: não devolve para o estoque.
  limpar_carrinho(false);

  mostrar_menu();
}
