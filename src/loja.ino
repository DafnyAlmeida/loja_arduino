// ---------------- ⚙️ Olá, seja bem-vindo(a) à nossa loja! ------------------- //
// Este coódigo foi desenvolvido por Alberto Kayron, Dafny Sabino, Ícaro Pereira,
// Luna Freitas e Eduarda Andrade
// -------------------------------------------------------------------------------

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

// Carrinho
String carrinho[10];
int idsCarrinho[10];
float precosCarrinho[10];
int quantidadesCarrinho[10];

float precoCarrinho = 0;
int quantidadeCarrinho = 0;
int indice_carrinho = 0;

// Produtos
String produtos[] = {"Arroz", "Feijao", "Leite"};
int quantidades[] = {6, 8, 5};
int ids[] = {1, 2, 3};
float precos[] = {12.50, 13.00, 11.50};
int quantidadeProdutos = 3;

// -------------------------------------------------------------------------------
// PROTOTIPOS DAS FUNCOES
// -------------------------------------------------------------------------------

void exibir_texto_grande(String texto, int coluna, int linha);
void exibir_texto_pequeno(String texto, int linha);
void exibir_produto_carrinho(int posicao);
void processando();
void confirmacao();
void erro();

bool voltar();

void inicializacao();
void mostrar_regras();

void mostrar_menu();
void navegacao_menu();
void esperar_soltar(int botao);
void executar_opcao_menu();

void ver_produtos();
void adicionar_produto();
void ver_carrinho();
void excluir_produto();
void pagar();

// -------------------------------------------------------------------------------
// FUNÇÕES SECUNDARIAS
// -------------------------------------------------------------------------------

// Funções para exibir texto no LCD
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

void exibir_produto_carrinho(int posicao) {
  lcd.clear();
  lcd.home();

  lcd.setCursor(0, 0);

  lcd.print(idsCarrinho[posicao]);
  lcd.print(" | ");
  lcd.print(carrinho[posicao]);
  lcd.print(" | R$ ");
  lcd.print(precosCarrinho[posicao]);
  lcd.print(" | ");
  lcd.print(quantidadesCarrinho[posicao]);

  delay(1000);

  // Faz o texto deslizar para aparecer inteiro
  for (int i = 0; i < 12; i++) {
    lcd.scrollDisplayLeft();
    delay(250);
  }

  delay(1000);

  lcd.clear();
  lcd.home();
}

// Função de espera
void processando() {
  lcd.clear();
  exibir_texto_pequeno("Processando...", 0);
  delay(1000);
}

// Funções de erro e confirmação
void confirmacao() {
  digitalWrite(LED_VERDE, HIGH);

  tone(BUZZER, 1000, 300);
  delay(300);

  digitalWrite(LED_VERDE, LOW);
}

void erro() {
  digitalWrite(LED_VERMELHO, HIGH);

  tone(BUZZER, 300, 500);
  delay(500);

  digitalWrite(LED_VERMELHO, LOW);
}

// Função do botão voltar

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

// -------------------------------------------------------------------------------
// ÍNCIO DO CÓDIGO
// -------------------------------------------------------------------------------

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

// Funções de inicialização (regras e boas vindas)
void inicializacao() {
  exibir_texto_grande("Seja bem-vindo a nossa loja!", 0, 0);
  exibir_texto_grande("A seguir serao exibidas as regras!", 0, 0);

  mostrar_regras();
  delay(200);
  exibir_texto_grande("Voce sera encaminhado para o menu!", 0, 0);
  delay(200);
  processando();
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

// Funções do menu
void mostrar_menu() {
  lcd.clear();
  lcd.home();

  String titulo = "Menu " + String(indice_menu + 1) + "/" + String(total_opcoes_menu);
  String texto = menu[indice_menu];

  lcd.setCursor(0, 0);
  lcd.print(titulo);

  lcd.setCursor(0, 1);
  lcd.print(texto);
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
    esperar_soltar(BTN_OK);
    executar_opcao_menu();
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
    processando();
    mostrar_regras();
    mostrar_menu();
  }
}

// -------------------------------------------------------------------------------
// FUNÇÕES (AÇÕES) DO SISTEMA
// -------------------------------------------------------------------------------

void ver_produtos() {
  //
}

// -------------------------------------------------------------------------------
// FUNÇÃO ADICIONAR por Eduarda
// -------------------------------------------------------------------------------

void adicionar_produto() {
  int indiceProduto = -1;
  int indiceProdutoCarrinho = -1;
  int id;

  processando();

  // Continua pedindo o ID enquanto ele for invalido
  while (indiceProduto == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Digite o ID");
    lcd.setCursor(0, 1);
    lcd.print("do produto:");

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

    for (int i = 0; i < quantidadeProdutos; i++) {
      if (id == ids[i]) {
        indiceProduto = i;
        break;
      }
    }

    if (indiceProduto == -1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID invalido");
      lcd.setCursor(0, 1);
      lcd.print("Tente novamente");
      delay(1500);
    }
  }

  // Verifica se o produto tem estoque
  if (quantidades[indiceProduto] <= 0) {
    exibir_texto_grande("Produto sem estoque!", 0, 0);
    mostrar_menu();
    return;
  }

  // Pede a quantidade
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite a");
  lcd.setCursor(0, 1);
  lcd.print("quantidade:");

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
    lcd.print("Qtd invalida");
    lcd.setCursor(0, 1);
    lcd.print("Estoque: ");
    lcd.print(quantidades[indiceProduto]);

    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Digite");
    lcd.setCursor(0, 1);
    lcd.print("novamente:");

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

  // Verifica se o produto ja existe no carrinho
  for (int i = 0; i < quantidadeCarrinho; i++) {
    if (carrinho[i] == produtos[indiceProduto]) {
      indiceProdutoCarrinho = i;
      break;
    }
  }

  // Produto novo: ocupa uma nova posicao
  if (indiceProdutoCarrinho == -1) {
    if (quantidadeCarrinho >= 10) {
      exibir_texto_grande("Carrinho cheio!", 0, 0);
      mostrar_menu();
      return;
    }

    idsCarrinho[quantidadeCarrinho] = ids[indiceProduto];
    carrinho[quantidadeCarrinho] = produtos[indiceProduto];
    precosCarrinho[quantidadeCarrinho] = precos[indiceProduto];
    quantidadesCarrinho[quantidadeCarrinho] = quantidadeProdutoCarrinho;

    quantidadeCarrinho++;
  }
  // Produto repetido: apenas soma a quantidade
  else {
    quantidadesCarrinho[indiceProdutoCarrinho] += quantidadeProdutoCarrinho;
  }

  // Atualiza estoque e total
  quantidades[indiceProduto] -= quantidadeProdutoCarrinho;
  precoCarrinho += quantidadeProdutoCarrinho * precos[indiceProduto];
  processando();
  exibir_texto_grande("Produto adicionado com sucesso!", 0, 0);
  mostrar_menu();
}

// -------------------------------------------------------------------------------
// FUNÇÃO VER CARRINHO por Dafny
// -------------------------------------------------------------------------------

void ver_carrinho() {
  int pagina_atual = 0;
  int total_itens = 0;

  // Soma a quantidade total de itens
  for (int i = 0; i < quantidadeCarrinho; i++) {
    total_itens += quantidadesCarrinho[i];
  }

  // Verifica se o carrinho esta vazio
  if (quantidadeCarrinho == 0) {
    exibir_texto_grande("Carrinho vazio!", 0, 0);
    mostrar_menu();
    return;
  }

  // Exibe quantidade de itens
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Carrinho:");
  lcd.setCursor(0, 1);
  lcd.print(total_itens);
  lcd.print(" itens");
  delay(1500);

  // Exibe cabecalho
  exibir_texto_grande("ID | NOME | PRECO | QTD", 0, 0);

  while (true) {

    // Mostra produto atual
    if (pagina_atual < quantidadeCarrinho) {
      exibir_produto_carrinho(pagina_atual);
    }

    // Mostra total depois do ultimo produto
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Total:");

      lcd.setCursor(0, 1);
      lcd.print("R$ ");
      lcd.print(precoCarrinho);

      delay(1500);
    }

    // Espera navegacao
    while (true) {

      if (voltar()) {
        return;
      }

      if (digitalRead(BTN_DOWN) == LOW) {
        esperar_soltar(BTN_DOWN);

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

void pagar() { // Ícaro
  //
}