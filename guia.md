# Guia Técnico para Desenvolvedores

## Sistema de Carrinho de Compras com Arduino e LCD 16x2

## 1. Visão geral

Este projeto implementa um sistema simples de loja utilizando Arduino, LCD 16x2, botões físicos, LEDs, buzzer e entrada de dados pelo Monitor Serial.

O sistema permite:

* visualizar produtos disponíveis;
* adicionar produtos ao carrinho;
* visualizar os itens selecionados;
* excluir produtos;
* realizar o pagamento;
* consultar as regras de navegação.

O projeto foi adaptado para reduzir o consumo de memória RAM, utilizando:

* `PROGMEM` para dados constantes;
* `PSTR()` e `F()` para textos fixos;
* valores monetários em centavos;
* vetores compactos de `byte`;
* carrinho baseado no índice original dos produtos.

---

# 2. Hardware utilizado

| Componente   |                                    Pino |
| ------------ | --------------------------------------: |
| Botão DOWN   |                                     `2` |
| Botão UP     |                                     `3` |
| Botão BACK   |                                     `4` |
| Botão OK     |                                     `5` |
| LED verde    |                                     `6` |
| LED vermelho |                                     `7` |
| Buzzer       |                                     `8` |
| LCD I2C      | Controlado por `Adafruit_LiquidCrystal` |

Declaração utilizada:

```cpp
const byte BTN_DOWN = 2;
const byte BTN_UP = 3;
const byte BTN_BACK = 4;
const byte BTN_OK = 5;

const byte LED_VERDE = 6;
const byte LED_VERMELHO = 7;
const byte BUZZER = 8;
```

Os botões utilizam:

```cpp
pinMode(BTN_UP, INPUT_PULLUP);
pinMode(BTN_DOWN, INPUT_PULLUP);
pinMode(BTN_OK, INPUT_PULLUP);
pinMode(BTN_BACK, INPUT_PULLUP);
```

Por esse motivo, um botão pressionado é identificado por:

```cpp
digitalRead(botao) == LOW
```

---

# 3. Organização da memória

## 3.1 Dados constantes na Flash

Informações que não mudam durante a execução devem ficar na memória Flash para preservar a RAM do Arduino.

Exemplos:

```cpp
const byte ids[] PROGMEM = {1, 2, 3};

const unsigned int precosCentavos[] PROGMEM = {
  1250, 1300, 1150
};
```

Também devem permanecer na Flash:

* nomes dos produtos;
* opções do menu;
* regras de navegação;
* mensagens fixas.

## 3.2 Dados mutáveis na RAM

Informações que mudam durante a execução precisam permanecer na RAM.

```cpp
byte quantidades[QUANTIDADE_PRODUTOS] = {6, 8, 5};
byte quantidadesCarrinho[QUANTIDADE_PRODUTOS] = {0, 0, 0};

byte quantidadeCarrinho = 0;
unsigned long precoCarrinhoCentavos = 0;
```

### Significado dos dados

| Variável                | Responsabilidade                       |
| ----------------------- | -------------------------------------- |
| `quantidades[]`         | Estoque restante de cada produto       |
| `quantidadesCarrinho[]` | Quantidade comprada de cada produto    |
| `quantidadeCarrinho`    | Número de tipos diferentes adicionados |
| `precoCarrinhoCentavos` | Valor total da compra em centavos      |

---

# 4. Modelo de produtos e carrinho

Todos os vetores de produto utilizam o mesmo índice.

| Índice | Produto |  ID |    Preço | Estoque |
| -----: | ------- | --: | -------: | ------: |
|    `0` | Arroz   | `1` | R$ 12,50 |     `6` |
|    `1` | Feijão  | `2` | R$ 13,00 |     `8` |
|    `2` | Leite   | `3` | R$ 11,50 |     `5` |

Exemplo:

```cpp
quantidadesCarrinho[0] = 2;
```

significa que há duas unidades de arroz no carrinho.

O carrinho não armazena novamente:

* nome;
* ID;
* preço.

Essas informações são recuperadas a partir do índice do produto, evitando duplicação de memória.

---

# 5. Protótipos das funções

Os protótipos devem permanecer antes do `setup()` para declarar as funções utilizadas ao longo do programa.

```cpp
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
```

---

# 6. Referência das funções

## 6.1 Funções de memória e exibição

### `copiar_texto_flash()`

```cpp
void copiar_texto_flash(const char* const tabela[], byte indice);
```

#### Responsabilidade

Copiar para `textoBuffer` um texto armazenado em uma tabela localizada na memória Flash.

#### Parâmetros

| Parâmetro | Tipo                  | Descrição                                |
| --------- | --------------------- | ---------------------------------------- |
| `tabela`  | `const char* const[]` | Vetor de textos armazenados em `PROGMEM` |
| `indice`  | `byte`                | Posição do texto que será carregado      |

#### Utilização típica

```cpp
copiar_texto_flash(menu, indice_menu);
lcd.print(textoBuffer);
```

#### Chamadores esperados

* `mostrar_menu()`;
* `mostrar_regras()`;
* `exibir_produto_carrinho()`;
* `ver_produtos()`.

#### Efeito colateral

Substitui o conteúdo anterior de:

```cpp
char textoBuffer[40];
```

---

### `exibir_texto_grande_P()`

```cpp
void exibir_texto_grande_P(PGM_P texto, byte coluna, byte linha);
```

#### Responsabilidade

Exibir mensagens fixas maiores, armazenadas na Flash, com possibilidade de rolagem horizontal.

#### Utilização típica

```cpp
exibir_texto_grande_P(PSTR("Produto adicionado!"), 0, 0);
```

#### Chamadores esperados

* `inicializacao()`;
* `adicionar_produto()`;
* `ver_carrinho()`;
* `excluir_produto()`;
* `pagar()`.

#### Restrições

Não utilizar para telas que precisam manter uma das linhas fixa. A rolagem do LCD desloca as duas linhas simultaneamente.

---

### `exibir_texto_pequeno()`

```cpp
void exibir_texto_pequeno(const char* texto, byte linha);
```

#### Responsabilidade

Exibir um texto já disponível na RAM, normalmente armazenado em `textoBuffer`.

#### Utilização típica

```cpp
copiar_texto_flash(regras, i);
exibir_texto_pequeno(textoBuffer, 1);
```

#### Chamadores esperados

* `mostrar_regras()`;
* funções que exibam textos carregados de tabelas dinâmicas.

#### Efeito colateral

Limpa a linha escolhida antes de imprimir o novo conteúdo.

---

### `exibir_texto_pequeno_P()`

```cpp
void exibir_texto_pequeno_P(PGM_P texto, byte linha);
```

#### Responsabilidade

Exibir diretamente uma mensagem fixa da Flash, sem usar o buffer temporário.

#### Utilização típica

```cpp
exibir_texto_pequeno_P(PSTR("Processando..."), 0);
```

#### Chamadores esperados

* `processando()`;
* `inicio_carrinho()`;
* mensagens estáticas curtas.

#### Diferença para `exibir_texto_pequeno()`

| Função                     | Origem do texto         |
| -------------------------- | ----------------------- |
| `exibir_texto_pequeno()`   | Texto presente na RAM   |
| `exibir_texto_pequeno_P()` | Texto presente na Flash |

---

## 6.2 Funções de produto e valor monetário

### `imprimir_preco()`

```cpp
void imprimir_preco(unsigned long valorCentavos);
```

#### Responsabilidade

Formatar e imprimir valores monetários armazenados em centavos.

#### Exemplo

```cpp
imprimir_preco(3650);
```

Saída esperada:

```txt
36,50
```

#### Chamadores esperados

* `ver_produtos()`;
* `exibir_produto_carrinho()`;
* `ver_carrinho()`;
* `pagar()`.

#### Regra de implementação

O símbolo monetário deve ser impresso pelo chamador:

```cpp
lcd.print(F("R$ "));
imprimir_preco(precoCarrinhoCentavos);
```

---

### `obter_id()`

```cpp
byte obter_id(byte indiceProduto);
```

#### Responsabilidade

Ler da memória Flash o ID associado ao produto informado.

#### Entrada e saída

| Entrada | Saída |
| ------: | ----: |
|     `0` |   `1` |
|     `1` |   `2` |
|     `2` |   `3` |

#### Chamadores esperados

* `adicionar_produto()`;
* `ver_produtos()`;
* `exibir_produto_carrinho()`.

---

### `obter_preco()`

```cpp
unsigned int obter_preco(byte indiceProduto);
```

#### Responsabilidade

Ler da memória Flash o preço em centavos do produto informado.

#### Entrada e saída

| Entrada | Produto |  Saída |
| ------: | ------- | -----: |
|     `0` | Arroz   | `1250` |
|     `1` | Feijão  | `1300` |
|     `2` | Leite   | `1150` |

#### Chamadores esperados

* `adicionar_produto()`;
* `ver_produtos()`;
* `exibir_produto_carrinho()`;
* `excluir_produto()`.

---

### `encontrar_indice_carrinho()`

```cpp
int encontrar_indice_carrinho(byte posicao);
```

#### Responsabilidade

Converter a posição visual da navegação do carrinho no índice real do produto.

#### Necessidade

Produtos com quantidade zero não devem aparecer ao usuário.

Exemplo:

```cpp
quantidadesCarrinho = {2, 0, 1};
```

| Posição visual | Produto exibido | Índice real |
| -------------: | --------------- | ----------: |
|            `0` | Arroz           |         `0` |
|            `1` | Leite           |         `2` |

#### Retorno

* Índice real do produto encontrado;
* `-1` caso a posição solicitada não exista.

#### Chamadores esperados

* `exibir_produto_carrinho()`.

---

### `exibir_produto_carrinho()`

```cpp
void exibir_produto_carrinho(byte posicao);
```

#### Responsabilidade

Exibir um produto adicionado ao carrinho na segunda linha do LCD.

#### Layout esperado

```txt
I NOME   PRECO Q
1 Arroz  12,50 2
```

#### Regras de interface

* A primeira linha contém o cabeçalho fixo;
* A segunda linha contém apenas o produto atual;
* A função não deve utilizar `scrollDisplayLeft()`;
* Nomes maiores que o espaço disponível devem ser truncados apenas para exibição.

#### Chamadores esperados

* `ver_carrinho()`.

---

## 6.3 Funções de feedback ao usuário

### `processando()`

```cpp
void processando();
```

#### Responsabilidade

Exibir uma mensagem temporária indicando execução de uma operação.

#### Exemplo de saída

```txt
Processando...
```

#### Chamadores esperados

* `adicionar_produto()`;
* `excluir_produto()`;
* `pagar()`.

---

### `confirmacao()`

```cpp
void confirmacao();
```

#### Responsabilidade

Executar o feedback visual e sonoro de sucesso.

#### Comportamento esperado

* Acender LED verde;
* Emitir som crescente ou positivo no buzzer;
* Desligar o buzzer;
* Apagar LED verde.

#### Chamadores esperados

* Após adição válida;
* Após exclusão válida;
* Após pagamento confirmado.

---

### `erro()`

```cpp
void erro();
```

#### Responsabilidade

Executar o feedback visual e sonoro para operações inválidas.

#### Comportamento esperado

* Acender LED vermelho;
* Emitir som grave no buzzer;
* Desligar o buzzer;
* Apagar LED vermelho.

#### Chamadores esperados

* ID inválido;
* Estoque insuficiente;
* Quantidade inválida;
* Carrinho vazio;
* Tentativa de exclusão inválida.

---

## 6.4 Funções de navegação

### `voltar()`

```cpp
bool voltar();
```

#### Responsabilidade

Verificar se o usuário solicitou retorno ao menu principal.

#### Retorno

| Valor   | Significado                    |
| ------- | ------------------------------ |
| `true`  | Usuário pressionou `BTN_BACK`  |
| `false` | Nenhuma solicitação de retorno |

#### Utilização típica

```cpp
if (voltar()) {
  return;
}
```

#### Chamadores esperados

* Telas que aguardam entrada serial;
* Navegação pelo carrinho;
* Navegação pelos produtos;
* Confirmação de pagamento.

---

### `esperar_soltar()`

```cpp
void esperar_soltar(byte botao);
```

#### Responsabilidade

Aplicar debounce simples e impedir que um clique seja processado várias vezes.

#### Utilização típica

```cpp
if (digitalRead(BTN_DOWN) == LOW) {
  indice_menu++;
  esperar_soltar(BTN_DOWN);
}
```

#### Chamadores esperados

* `navegacao_menu()`;
* `ver_produtos()`;
* `ver_carrinho()`;
* `pagar()`.

---

## 6.5 Funções de inicialização e menu

### `inicializacao()`

```cpp
void inicializacao();
```

#### Responsabilidade

Apresentar a aplicação e direcionar o usuário ao menu principal.

#### Chamador

Deve ser chamada apenas em:

```cpp
void setup()
```

#### Fluxo esperado

```txt
Boas-vindas
   ↓
Regras
   ↓
Mensagem de carregamento
   ↓
Menu principal
```

---

### `mostrar_regras()`

```cpp
void mostrar_regras();
```

#### Responsabilidade

Exibir as instruções dos botões cadastradas em `regras[]`.

#### Chamadores esperados

* `inicializacao()`;
* opção `Ver regras` do menu.

---

### `mostrar_menu()`

```cpp
void mostrar_menu();
```

#### Responsabilidade

Renderizar a opção atualmente selecionada no menu.

#### Dependências

* `indice_menu`;
* `menu[]`;
* `copiar_texto_flash()`.

#### Exemplo de saída

```txt
Menu 2/6
Adicionar
```

#### Chamadores esperados

* `inicializacao()`;
* `voltar()`;
* após execução de ações;
* `navegacao_menu()`.

---

### `navegacao_menu()`

```cpp
void navegacao_menu();
```

#### Responsabilidade

Monitorar os botões do menu principal e alterar a opção selecionada.

#### Chamador

Executada continuamente em:

```cpp
void loop() {
  navegacao_menu();
}
```

#### Eventos tratados

| Evento     | Ação                          |
| ---------- | ----------------------------- |
| `BTN_DOWN` | Avança para a próxima opção   |
| `BTN_UP`   | Retorna para a opção anterior |
| `BTN_OK`   | Executa a opção selecionada   |

---

### `executar_opcao_menu()`

```cpp
void executar_opcao_menu();
```

#### Responsabilidade

Direcionar o fluxo para a funcionalidade selecionada no menu.

#### Mapeamento esperado

| Índice | Opção        | Função executada      |
| -----: | ------------ | --------------------- |
|    `0` | Ver produtos | `ver_produtos()`      |
|    `1` | Adicionar    | `adicionar_produto()` |
|    `2` | Excluir      | `excluir_produto()`   |
|    `3` | Ver carrinho | `ver_carrinho()`      |
|    `4` | Pagar        | `pagar()`             |
|    `5` | Ver regras   | `mostrar_regras()`    |

---

## 6.6 Funções principais da loja

### `ver_produtos()`

```cpp
void ver_produtos();
```

#### Responsabilidade

Exibir os produtos cadastrados e o estoque atual.

#### Requisitos de interface

* Utilizar cabeçalho compacto compatível com LCD 16x2;
* Permitir navegação usando `BTN_UP` e `BTN_DOWN`;
* Permitir retorno com `BTN_BACK`;
* Mostrar estoque atualizado após operações no carrinho.

#### Dados utilizados

* `produtos[]`;
* `quantidades[]`;
* `obter_id()`;
* `obter_preco()`.

---

### `adicionar_produto()`

```cpp
void adicionar_produto();
```

#### Responsabilidade

Adicionar uma quantidade válida de um produto existente ao carrinho.

#### Fluxo obrigatório

1. Solicitar ID do produto;
2. Encontrar índice correspondente;
3. Validar existência;
4. Validar estoque disponível;
5. Solicitar quantidade;
6. Validar quantidade solicitada;
7. Atualizar carrinho;
8. Atualizar estoque;
9. Atualizar total;
10. Executar feedback de confirmação.

#### Atualizações realizadas

```cpp
quantidadesCarrinho[indiceProduto] += quantidadeSolicitada;
quantidades[indiceProduto] -= quantidadeSolicitada;
precoCarrinhoCentavos += quantidadeSolicitada * obter_preco(indiceProduto);
```

Somente quando o produto ainda não existe no carrinho:

```cpp
if (quantidadesCarrinho[indiceProduto] == 0) {
  quantidadeCarrinho++;
}
```

#### Validações obrigatórias

| Situação                         | Ação                       |
| -------------------------------- | -------------------------- |
| ID não encontrado                | Executar `erro()`          |
| Produto sem estoque              | Informar indisponibilidade |
| Quantidade menor ou igual a zero | Solicitar novamente        |
| Quantidade maior que estoque     | Solicitar novamente        |

---

### `inicio_carrinho()`

```cpp
void inicio_carrinho();
```

#### Responsabilidade

Exibir uma tela introdutória antes da listagem dos produtos adicionados.

#### Exemplo de saída

```txt
Total produtos:
2 produtos
```

#### Regra importante

`quantidadeCarrinho` representa a quantidade de **tipos diferentes** no carrinho, e não a quantidade total de unidades.

Exemplo:

```txt
2 Arroz + 1 Leite = 2 produtos diferentes
```

#### Chamador esperado

* `ver_carrinho()`.

---

### `ver_carrinho()`

```cpp
void ver_carrinho();
```

#### Responsabilidade

Permitir a consulta dos produtos adicionados e do valor total da compra.

#### Fluxo obrigatório

1. Verificar se o carrinho está vazio;
2. Executar `inicio_carrinho()`;
3. Renderizar cabeçalho fixo;
4. Mostrar o produto correspondente à página atual;
5. Permitir navegação vertical;
6. Mostrar a tela de total após o último produto;
7. Permitir retorno ao menu.

#### Layout recomendado

Produto:

```txt
I NOME   PRECO Q
1 Arroz  12,50 2
```

Total:

```txt
I NOME   PRECO Q
TOTAL R$ 36,50
```

#### Regra importante

A página de total é apenas uma tela adicional de consulta. Ela não deve alterar:

```cpp
quantidadeCarrinho
quantidadesCarrinho[]
```

---

### `excluir_produto()`

```cpp
void excluir_produto();
```

#### Responsabilidade

Remover parcialmente ou completamente um produto do carrinho.

#### Fluxo esperado

1. Solicitar ID;
2. Validar existência do produto;
3. Validar presença no carrinho;
4. Solicitar quantidade a remover;
5. Validar quantidade;
6. Atualizar carrinho;
7. Devolver unidades ao estoque;
8. Subtrair valor do total;
9. Atualizar quantidade de tipos diferentes;
10. Confirmar operação.

#### Atualizações esperadas

```cpp
quantidadesCarrinho[indiceProduto] -= quantidadeRemovida;
quantidades[indiceProduto] += quantidadeRemovida;
precoCarrinhoCentavos -= quantidadeRemovida * obter_preco(indiceProduto);
```

Caso todas as unidades do produto sejam removidas:

```cpp
if (quantidadesCarrinho[indiceProduto] == 0) {
  quantidadeCarrinho--;
}
```

---

### `pagar()`

```cpp
void pagar();
```

#### Responsabilidade

Finalizar a compra e limpar o carrinho após confirmação.

#### Fluxo esperado

1. Verificar se existem produtos no carrinho;
2. Mostrar total a pagar;
3. Aguardar confirmação do usuário;
4. Emitir feedback de sucesso;
5. Zerar dados do carrinho;
6. Retornar ao menu.

#### Limpeza do carrinho

```cpp
for (byte i = 0; i < QUANTIDADE_PRODUTOS; i++) {
  quantidadesCarrinho[i] = 0;
}

quantidadeCarrinho = 0;
precoCarrinhoCentavos = 0;
```

#### Decisão de estoque

Ao realizar o pagamento, o estoque já deve permanecer reduzido, pois a compra foi finalizada.

Caso o usuário cancele a compra antes do pagamento, as quantidades devem ser devolvidas ao estoque somente se for implementada uma ação específica de cancelamento geral.

---

# 7. Fluxo geral de chamadas

```txt
setup()
  └── inicializacao()
        ├── exibir_texto_grande_P()
        ├── mostrar_regras()
        │     ├── copiar_texto_flash()
        │     └── exibir_texto_pequeno()
        ├── processando()
        └── mostrar_menu()
              └── copiar_texto_flash()

loop()
  └── navegacao_menu()
        ├── esperar_soltar()
        ├── mostrar_menu()
        └── executar_opcao_menu()
              ├── ver_produtos()
              ├── adicionar_produto()
              │     ├── processando()
              │     ├── voltar()
              │     ├── obter_id()
              │     ├── obter_preco()
              │     ├── erro()
              │     └── confirmacao()
              ├── excluir_produto()
              │     ├── obter_id()
              │     ├── obter_preco()
              │     ├── erro()
              │     └── confirmacao()
              ├── ver_carrinho()
              │     ├── inicio_carrinho()
              │     ├── exibir_produto_carrinho()
              │     │     ├── encontrar_indice_carrinho()
              │     │     ├── copiar_texto_flash()
              │     │     ├── obter_id()
              │     │     ├── obter_preco()
              │     │     └── imprimir_preco()
              │     ├── imprimir_preco()
              │     └── voltar()
              ├── pagar()
              │     ├── imprimir_preco()
              │     ├── confirmacao()
              │     └── erro()
              └── mostrar_regras()
```

---

# 8. Convenções para manutenção

## 8.1 Novos produtos

Para adicionar novos produtos, o desenvolvedor deverá atualizar, na mesma ordem:

```cpp
const char produtoNovo[] PROGMEM = "Nome";
const char* const produtos[] PROGMEM = { ... };

const byte ids[] PROGMEM = { ... };
const unsigned int precosCentavos[] PROGMEM = { ... };

byte quantidades[QUANTIDADE_PRODUTOS] = { ... };
byte quantidadesCarrinho[QUANTIDADE_PRODUTOS] = { ... };

const byte QUANTIDADE_PRODUTOS = novoTotal;
```

Todos os vetores devem possuir a mesma quantidade de posições e preservar a correspondência por índice.

## 8.2 Valores monetários

Não utilizar `float` para preços. Todos os valores devem ser armazenados em centavos:

```cpp
1990   // R$ 19,90
500    // R$ 5,00
```

## 8.3 Textos fixos

Evitar:

```cpp
lcd.print("Mensagem fixa");
```

Preferir:

```cpp
lcd.print(F("Mensagem fixa"));
```

Para funções que recebem texto em Flash:

```cpp
exibir_texto_grande_P(PSTR("Mensagem fixa"), 0, 0);
```

## 8.4 Uso de `String`

Evitar o tipo:

```cpp
String
```

em novas funcionalidades, pois pode aumentar o consumo e a fragmentação da RAM em microcontroladores com pouca memória.

Preferir:

* `char[]`;
* `textoBuffer`;
* textos em `PROGMEM`.

## 8.5 Interface do LCD

O LCD possui apenas 16 colunas e 2 linhas.

A exibição deve priorizar:

* cabeçalhos compactos;
* textos abreviados;
* uma informação principal por linha;
* navegação entre páginas em vez de textos excessivamente longos.

Evitar rolagem em telas com cabeçalho fixo, pois `scrollDisplayLeft()` movimenta as duas linhas simultaneamente.

---

# 9. Checklist para implementar uma nova funcionalidade

Antes de finalizar uma nova função, verificar:

* [ ] A função utiliza `byte` para valores pequenos?
* [ ] Mensagens fixas utilizam `F()` ou `PSTR()`?
* [ ] Não foi criado nenhum novo `String` desnecessário?
* [ ] O carrinho continua usando o índice original do produto?
* [ ] Os preços continuam sendo calculados em centavos?
* [ ] As alterações no carrinho também atualizam estoque e total?
* [ ] O botão voltar funciona durante esperas?
* [ ] O LCD não ultrapassa 16 caracteres por linha em telas fixas?
* [ ] O LED e o buzzer são acionados apenas em situações adequadas?
* [ ] O menu reaparece após a conclusão ou cancelamento da operação?

---

# 10. Resumo das responsabilidades

| Categoria               | Funções                                                                                                                                               |
| ----------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| Memória e textos        | `copiar_texto_flash()`, `exibir_texto_grande_P()`, `exibir_texto_pequeno()`, `exibir_texto_pequeno_P()`                                               |
| Produtos e valores      | `imprimir_preco()`, `obter_id()`, `obter_preco()`, `encontrar_indice_carrinho()`, `exibir_produto_carrinho()`                                         |
| Feedback                | `processando()`, `confirmacao()`, `erro()`                                                                                                            |
| Navegação               | `voltar()`, `mostrar_menu()`, `navegacao_menu()`, `esperar_soltar()`, `executar_opcao_menu()`                                                         |
| Funcionalidades da loja | `inicializacao()`, `mostrar_regras()`, `ver_produtos()`, `adicionar_produto()`, `inicio_carrinho()`, `ver_carrinho()`, `excluir_produto()`, `pagar()` |

Este documento deve ser atualizado sempre que forem adicionadas novas opções de menu, novos produtos ou alterações importantes na lógica do carrinho.
