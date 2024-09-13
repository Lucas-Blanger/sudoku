# Projeto: Jogo de Sudoku em C

Disciplina: Laboratório de Programação I

Curso: Ciência da Computação

Universidade Federal de Santa Maria (UFSM)

## Descrição

Este projeto consiste em um jogo de Sudoku desenvolvido em linguagem C como parte da disciplina de Laboratório de Programação I. O jogo foi criado com o objetivo de aplicar e consolidar conceitos de programação, como estruturas de dados, algoritmos de backtracking e manipulação de arquivos. Ele permite que o usuário jogue o popular quebra-cabeça de números em um ambiente interativo. 

O Sudoku é um jogo de lógica onde o objetivo é preencher uma grade 9x9 com números de 1 a 9, garantindo que cada linha, coluna e subgrade 3x3 contenham números únicos, sem repetições.

Funcionalidades

    Interface gráfica
    Geração de tabuleiros Sudoku pré-definidos
    Validação de entradas para garantir que o jogador insira valores válidos
    Função de ajuda para o jogador obter dicas
    Verificação automática de vitória ao completar o tabuleiro corretamente
    Contador de tempo para medir a duração da partida
    Tabela de recordes

## Como executar o jogo
Pré-requisitos

    Sistema operacional compatível com compiladores C (Linux, Windows, macOS)
    Compilador de C (ex: GCC)

# Passos para compilar e executar:

  Clone o repositório ou baixe os arquivos do projeto.
  Abra o terminal na pasta onde estão os arquivos do projeto.
  Compile o código com o seguinte comando:

    bash

    gcc -o sudoku sudoku.c

Execute o jogo com o comando:

    bash

    ./sudoku

Como jogar

    O jogo inicia com um tabuleiro parcialmente preenchido.
    O jogador insere números nas posições vazias.
    O sistema valida a jogada e informa se o movimento é permitido.
    O jogo termina quando o jogador preenche corretamente todo o tabuleiro, ou pode ser salvo e retomado mais tarde.

Estrutura do Projeto

    sudoku.c: Arquivo principal com toda a lógica do jogo.
    tela.h: Cabeçalho contendo declarações de funções e estruturas usadas.
    tabuleiros.txt: Arquivo contendo exemplos de tabuleiros pré-definidos.
    recordes.txt: Arquivo gerado para salvara as pontuações
    README.md: Documento de instruções sobre o projeto.

Regras do Jogo

    Preencha a grade 9x9 com números de 1 a 9.
    Cada linha, coluna e subgrade 3x3 deve conter números únicos, sem repetições.
    O jogo só permite a inserção de números em células vazias.

Autor

    Nome do(a) aluno(a): Lucas Blanger

Licença

Este projeto foi desenvolvido para fins acadêmicos e pode ser utilizado livremente com fins educacionais.
