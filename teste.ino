void ver_carrinho() {
  int pagina_atual = 0;
  int indiceProduto = -1;
  int total_itens = 0;

  for (int i = 0; i < quantidadeCarrinho; i++) {
    total_itens += quantidadesCarrinho[i];
  }

  // Verifica se o carrinho esta vazio
  if (quantidadeCarrinho == 0) {
    exibir_texto_grande("Carrinho vazio!", 0, 0);
    mostrar_menu();
    return;
  }

  // Exibe a quantidade total de itens
  exibir_texto_grande("Carrinho: " + String(total_itens) + " itens", 0, 0);

  // Exibe o cabecalho
  exibir_texto_grande("ID | NOME | PRECO | QTD", 0, 0);

  // Mantem a pagina aberta para navegacao
  while (true) {

    // Mostra os produtos do carrinho
    if (pagina_atual < quantidadeCarrinho) {
      int indiceProduto = -1;

      // Procura o produto do carrinho na lista original
      for (int i = 0; i < quantidadeProdutos; i++) {
        if (carrinho[pagina_atual] == produtos[i]) {
          indiceProduto = i;
          break;
        }
      }

      // Exibe as informacoes do produto encontrado
      if (indiceProduto != -1) {
        String informacoesProduto = "";

        informacoesProduto += String(ids[indiceProduto]);
        informacoesProduto += " | ";
        informacoesProduto += produtos[indiceProduto];
        informacoesProduto += " | R$ ";
        informacoesProduto += String(precos[indiceProduto], 2);
        informacoesProduto += " | ";
        informacoesProduto += String(quantidadesCarrinho[pagina_atual]);

        exibir_texto_grande(informacoesProduto, 0, 0);
      }
    }

    // Depois do ultimo produto, mostra o total
    else {
      String textoTotal = "Total: R$ ";
      textoTotal += String(precoCarrinho, 2);

      exibir_texto_grande(textoTotal, 0, 0);
    }

    // Espera o clique dos botoes
    while (true) {

      // Volta ao menu
      if (voltar()) {
        return;
      }

      // Avanca para o proximo produto ou total
      if (digitalRead(BTN_DOWN) == LOW) {
        esperar_soltar(BTN_DOWN);

        if (pagina_atual < quantidadeCarrinho) {
          pagina_atual++;
        }

        break;
      }

      // Volta para o produto anterior
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