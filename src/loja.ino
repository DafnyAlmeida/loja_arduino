// ---------------- ⚙️ Olá, seja bem-vindo(a) à nossa loja! ----------------- //
// Este codigo foi desenvolvido por Alberto Kayron, Dafny Sabino, Icaro Pereira,
// Luna Freitas e Eduarda Andrade
// -----------------------------------------------------------------------------

// Bibliotecas
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
#include <avr/pgmspace.h>

Adafruit_LiquidCrystal lcd(0);

// Componentes: byte ocupa 1 byte, suficiente para os pinos
const byte BTN_DOWN = 2;
const byte BTN_UP = 3;
const byte BTN_BACK = 4;
const byte BTN_OK = 5;

const byte LED_VERDE = 6;
const byte LED_VERMELHO = 7;
const byte BUZZER = 8;

// ----------------------------------------------------------------------------
// TEXTOS SALVOS NA FLASH (PROGMEM), EM VEZ DE OCUPAREM A RAM
// ----------------------------------------------------------------------------

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

const byte TOTAL_OPCOES = 4;
const byte TOTAL_OPCOES_MENU = 6;
const byte QUANTIDADE_PRODUTOS = 3;

byte indice_menu = 0;

// Buffer reutilizado para carregar somente um texto por vez da Flash
char textoBuffer[40];

// ----------------------------------------------------------------------------
// PRODUTOS E CARRINHO
// ----------------------------------------------------------------------------

// O estoque muda durante o programa, portanto continua na RAM
byte quantidades[QUANTIDADE_PRODUTOS] = {6, 8, 5};

// Estes dados nao mudam e ficam na memoria Flash
const byte ids[] PROGMEM = {1, 2, 3};

// Valores em centavos: evita float e reduz operacoes pesadas.
// 1250 = R$ 12,50; 1300 = R$ 13,00; 1150 = R$ 11,50.
const unsigned int precosCentavos[] PROGMEM = {1250, 1300, 1150};

// A quantidade no carrinho usa o mesmo indice da lista de produtos.
// Assim nao e necessario guardar nomes, ids e precos novamente.
byte quantidadesCarrinho[QUANTIDADE_PRODUTOS] = {0, 0, 0};

byte quantidadeCarrinho = 0;                 // Quantidade de tipos de produto
unsigned long precoCarrinhoCentavos = 0;     // Total da compra

// -----------------------------------------------------------------------------
// PROTOTIPOS DAS FUNCOES
// -----------------------------------------------------------------------------

void copiar_texto_flash(const char* const tabela[], byte indice);
void exibir_texto_grande_P(PGM_P texto, byte coluna, byte linha);
void exibir_texto_pequeno(const char* texto, byte linha);
void exibir_texto_pequeno_P(PGM_P texto, byte linha);
void imprimir_preco(unsigned long valorCentavos);
byte obter_id(byte indiceProduto);
unsigned int obter_preco(byte indiceProduto);
int encontrar_indice_carrinho(byte posicao);
void exibir_produto_carrinho(byte posicao);

void processando();
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

// -----------------------------------------------------------------------------
// FUNCOES SECUNDARIAS
// -----------------------------------------------------------------------------

// Copia apenas o texto necessario da Flash para o buffer reutilizavel.
void copiar_texto_flash(const char* const tabela[], byte indice) {
  PGM_P enderecoTexto = (PGM_P) pgm_read_word(&(tabela[indice]));
  strncpy_P(textoBuffer, enderecoTexto, sizeof(textoBuffer) - 1);
  textoBuffer[sizeof(textoBuffer) - 1] = '\0';
}

// Exibe um texto que esta na Flash e realiza scroll quando passar de 16 colunas.
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

// Usa textos carregados no buffer, como regras e nomes dos produtos.
void exibir_texto_pequeno(const char* texto, byte linha) {
  lcd.setCursor(0, linha);
  lcd.print(F("                "));

  lcd.setCursor(0, linha);
  lcd.print(texto);
}

// Usa textos fixos diretamente da Flash.
void exibir_texto_pequeno_P(PGM_P texto, byte linha) {
  lcd.setCursor(0, linha);
  lcd.print(F("                "));

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

// Converte a pagina do carrinho no indice original do produto.
// Somente produtos com quantidade maior que zero sao considerados.
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

  // Limpa somente a segunda linha. O cabecalho da primeira linha permanece fixo.
  lcd.setCursor(0, 1);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);

  // Formato que cabe no LCD 16x2 com os produtos atuais:
  // Exemplo: 2 Feijao 13,00 3
  lcd.print(obter_id(indiceProduto));
  lcd.print(' ');

  // Usa somente 6 caracteres do nome para a linha sempre caber.
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

// Funcao de espera
void processando() {
  lcd.clear();
  exibir_texto_pequeno_P(PSTR("Processando..."), 0);
  delay(1000);
}

// Funcoes de erro e confirmacao
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

// Funcao do botao voltar
bool voltar() {
  if (digitalRead(BTN_BACK) == LOW) {
    while (digitalRead(BTN_BACK) == LOW) {
      delay(10);
    }

    lcd.clear();
    mostrar_menu();

    return true;
  }

  return false;
}

void inicio_carrinho() {
  lcd.clear();

  exibir_texto_pequeno_P(PSTR("Total produtos:"), 0);

  lcd.setCursor(0, 1);
  lcd.print(quantidadeCarrinho);
  lcd.print(F(" produtos"));

  delay(1500);

  lcd.clear();
}

// -----------------------------------------------------------------------------
// INICIO DO CODIGO
// -----------------------------------------------------------------------------

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

// Funcoes de inicializacao (regras e boas-vindas)
void inicializacao() {
  exibir_texto_grande_P(PSTR("Seja bem-vindo a nossa loja!"), 0, 0);
  exibir_texto_grande_P(PSTR("A seguir serao exibidas as regras!"), 0, 0);

  mostrar_regras();
  delay(200);
  exibir_texto_grande_P(PSTR("Voce sera encaminhado para o menu!"), 0, 0);
  delay(200);
  processando();
  mostrar_menu();
}

void mostrar_regras() {
  lcd.clear();
  lcd.home();

  lcd.setCursor(0, 0);
  lcd.print(F("Regras:"));

  for (byte i = 0; i < TOTAL_OPCOES; i++) {
    copiar_texto_flash(regras, i);
    exibir_texto_pequeno(textoBuffer, 1);
    delay(1500);
  }

  lcd.clear();
}

// Funcoes do menu
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
    processando();
    mostrar_regras();
    mostrar_menu();
  }
}

// -----------------------------------------------------------------------------
// FUNCOES (ACOES) DO SISTEMA
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// FUNÇÃO VER PRODUTOS por Luna
// -----------------------------------------------------------------------------

void ver_produtos() {
  processando();
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

// ----------------------------------------------------------------------------
// FUNCAO ADICIONAR por Eduarda
// ----------------------------------------------------------------------------

void adicionar_produto() {
  int indiceProduto = -1;
  int id;

  processando();

  // Continua pedindo o ID enquanto ele for invalido
  while (indiceProduto == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Digite o ID"));
    lcd.setCursor(0, 1);
    lcd.print(F("do produto:"));

    while (Serial.available() == 0) {
      if (voltar()) {
        return;
      }
      delay(10);
    }

    id = Serial.parseInt();

    while (Serial.available() > 0) {
      Serial.read();
    }

    for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
      if (id == obter_id(i)) {
        indiceProduto = i;
        break;
      }
    }

    if (indiceProduto == -1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("ID invalido"));
      lcd.setCursor(0, 1);
      lcd.print(F("Tente novamente"));
      delay(1500);
    }
  }

  // Verifica se o produto tem estoque
  if (quantidades[indiceProduto] <= 0) {
    exibir_texto_grande_P(PSTR("Produto sem estoque!"), 0, 0);
    mostrar_menu();
    return;
  }

  // Pede a quantidade
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Digite a"));
  lcd.setCursor(0, 1);
  lcd.print(F("quantidade:"));

  while (Serial.available() == 0) {
    if (voltar()) {
      return;
    }
    delay(10);
  }

  int quantidadeProdutoCarrinho = Serial.parseInt();

  while (Serial.available() > 0) {
    Serial.read();
  }

  // Continua pedindo enquanto a quantidade for invalida
  while (quantidadeProdutoCarrinho <= 0 || quantidadeProdutoCarrinho > quantidades[indiceProduto]) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Qtd invalida"));
    lcd.setCursor(0, 1);
    lcd.print(F("Estoque: "));
    lcd.print(quantidades[indiceProduto]);

    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Digite"));
    lcd.setCursor(0, 1);
    lcd.print(F("novamente:"));

    while (Serial.available() == 0) {
      if (voltar()) {
        return;
      }

      delay(10);
    }

    quantidadeProdutoCarrinho = Serial.parseInt();

    while (Serial.available() > 0) {
      Serial.read();
    }
  }

  // Se esse produto ainda nao estava no carrinho, ele vira um novo tipo.
  if (quantidadesCarrinho[indiceProduto] == 0) {
    quantidadeCarrinho++;
  }

  // Soma a quantidade diretamente usando o indice do produto.
  quantidadesCarrinho[indiceProduto] += quantidadeProdutoCarrinho;

  // Atualiza estoque e total.
  quantidades[indiceProduto] -= quantidadeProdutoCarrinho;
  precoCarrinhoCentavos += (unsigned long) quantidadeProdutoCarrinho * obter_preco(indiceProduto);

  processando();
  exibir_texto_grande_P(PSTR("Produto adicionado com sucesso!"), 0, 0);
  mostrar_menu();
}

// -------------------------------------------------------------------------------
// FUNCAO VER CARRINHO por Dafny
// -------------------------------------------------------------------------------

void ver_carrinho() {
  byte pagina_atual = 0;

  // quantidadeCarrinho representa somente os tipos de produtos no carrinho.
  // O total em reais nao vira uma pagina contada como produto.
  if (quantidadeCarrinho == 0) {
    exibir_texto_grande_P(PSTR("Carrinho vazio!"), 0, 0);
    erro();
    delay(1000);
    mostrar_menu();
    return;
  }

  inicio_carrinho();

  lcd.clear();

  while (true) {
    // Cabecalho fixo da primeira linha: cabe nas 16 colunas do LCD.
    lcd.setCursor(0, 0);
    lcd.print(F("ID|NOME|PRECO|Q"));

    if (pagina_atual < quantidadeCarrinho) {
      exibir_produto_carrinho(pagina_atual);
    } 
    else {
      // Depois do ultimo produto, a segunda linha mostra apenas o total.
      // Essa tela nao altera quantidadeCarrinho.
      lcd.setCursor(0, 1);
      lcd.print(F("                "));
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

        // Vai ate a pagina do total, mas nao passa dela.
        if (pagina_atual < quantidadeCarrinho) {
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

void excluir_produto() { // Alberto
  // Exibir de forma estatica no topo a mensagem
}

void pagar() { // Icaro
  if (quantidadeCarrinho == 0) {
    exibir_texto_grande_P(PSTR("Carrinho vazio!"), 0, 0);
    erro();
    mostrar_menu();
    return;
  }

  processando();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Total: R$ "));
  imprimir_preco(precoCarrinhoCentavos);
  lcd.setCursor(0, 1);
  lcd.print(F("1:Confirmar"));

  while (true) {
    if (voltar()) {
      return;
    }

    if (digitalRead(BTN_DOWN) == LOW) {
      esperar_soltar(BTN_DOWN);
      break;
    }
  }

  processando();
  confirmacao();
  exibir_texto_grande_P(PSTR("Compra realizada com sucesso!"), 0, 0);

  for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
    quantidadesCarrinho[i] = 0;
  }
  quantidadeCarrinho = 0;
  precoCarrinhoCentavos = 0;

  mostrar_menu();
}
