#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "telag2.h"
#include <time.h>

#define LARGURA_TELA 1280
#define ALTURA_TELA 720

// Definição das áreas de clique para cada opção do menu
typedef struct {
    ponto_t posicao;
    tamanho_t tamanho;
} botao_t;

typedef struct {
    int id;
    int dificuldade;
    int tabuleiro[9][9];
} tabuleiro_t;

typedef struct {
    char jogador[20];
    int tabuleiro_id;
    int pontuacao;
} pontuacao_t;

typedef struct {
    int valor; // 0 para vazio, 1-9 para números
    bool inicial; // true se é um número inicial que não pode ser alterado
    bool marcacoes[9]; // marcadores para os números 1-9
} casa_t;

typedef struct {
    casa_t casas[9][9];
    int tabuleiro_id;
    int dificuldade;
    char jogador[20];
    int valor_jogada;
    time_t inicio_tempo;
    int cursor_x; // Posição do cursor em x
    int cursor_y; // Posição do cursor em y
} jogo_t;

pontuacao_t* ler_pontuacoes(const char* arquivo, int* num_pontuacoes);
void escrever_pontuacoes(const char* arquivo, pontuacao_t* pontuacoes, int num_pontuacoes);
int comparar_pontuacoes(const void* a, const void* b);

void desenhar_cursor_mouse(rato_t rato) {
    cor_t cor_cursor = {1, 0, 0, 1};
    ponto_t posicao = {rato.posicao.x, rato.posicao.y};
    circulo_t cursor = {posicao, 5};
    tela_circulo(cursor, 1, cor_cursor, cor_cursor);
}

// Desenha um botão com fundo
void desenhar_botao(botao_t botao, cor_t cor_fundo, cor_t cor_borda, cor_t cor_texto, char texto[]) {
    retangulo_t retangulo_fundo = {botao.posicao, botao.tamanho};
    tela_retangulo(retangulo_fundo, 2, cor_borda, cor_fundo);

    // Calcula a posição do texto centralizado no botão
    int altura_fonte = 24;
    ponto_t posicao_texto = {botao.posicao.x + botao.tamanho.largura / 2, botao.posicao.y + (botao.tamanho.altura - altura_fonte) / 2};
    tela_texto(posicao_texto, altura_fonte, cor_texto, texto);
}

void exibir_menu_inicial() {
    tamanho_t tamanho_tela = {LARGURA_TELA, ALTURA_TELA};
    tela_inicio(tamanho_tela, "Lucas Sudoku Game");

    cor_t cor_fundo = {0, 0, 0, 1};
    cor_t cor_texto = {0, 1, 0, 1};
    cor_t cor_botao_fundo = {0.2, 0.2, 0.2, 1};
    cor_t cor_botao_borda = {0, 1, 0, 1};

    ponto_t posicao_titulo = {LARGURA_TELA / 2, ALTURA_TELA / 4};
    ponto_t posicao_iniciar = {LARGURA_TELA / 2 - 150, ALTURA_TELA / 2 - 70};
    ponto_t posicao_recordes = {LARGURA_TELA / 2 - 150, ALTURA_TELA / 2};
    ponto_t posicao_sair = {LARGURA_TELA / 2 - 150, ALTURA_TELA / 2 + 70};

    botao_t botao_iniciar = {posicao_iniciar, {300, 60}};
    botao_t botao_recordes = {posicao_recordes, {300, 60}};
    botao_t botao_sair = {posicao_sair, {300, 60}};

    while (1) {
        tela_atualiza();

        tela_texto(posicao_titulo, 36, cor_texto, "Lucas Sudoku Game");

        desenhar_botao(botao_iniciar, cor_botao_fundo, cor_botao_borda, cor_texto, "Iniciar Jogo");
        desenhar_botao(botao_recordes, cor_botao_fundo, cor_botao_borda, cor_texto, "Recordes");
        desenhar_botao(botao_sair, cor_botao_fundo, cor_botao_borda, cor_texto, "Sair");

        // Verificar cliques do mouse
        rato_t rato = tela_rato();
        desenhar_cursor_mouse(rato);

        if (rato.clicado[0]) {  // Clique do botão esquerdo do mouse
            if (rato.posicao.x >= botao_iniciar.posicao.x &&
                rato.posicao.x <= botao_iniciar.posicao.x + botao_iniciar.tamanho.largura &&
                rato.posicao.y >= botao_iniciar.posicao.y &&
                rato.posicao.y <= botao_iniciar.posicao.y + botao_iniciar.tamanho.altura) {
                printf("Iniciar Jogo\n");
                break;
            } else if (rato.posicao.x >= botao_recordes.posicao.x &&
                       rato.posicao.x <= botao_recordes.posicao.x + botao_recordes.tamanho.largura &&
                       rato.posicao.y >= botao_recordes.posicao.y &&
                       rato.posicao.y <= botao_recordes.posicao.y + botao_recordes.tamanho.altura) {
                printf("Recordes\n");
                break;
            } else if (rato.posicao.x >= botao_sair.posicao.x &&
                       rato.posicao.x <= botao_sair.posicao.x + botao_sair.tamanho.largura &&
                       rato.posicao.y >= botao_sair.posicao.y &&
                       rato.posicao.y <= botao_sair.posicao.y + botao_sair.tamanho.altura) {
                printf("Sair\n");
                tela_fim();
                return;
            }
        }

        // Verificar entrada do teclado
        char tecla = tela_tecla();
        if (tecla != '\0') {
            if (tecla == '1') {
                printf("Iniciar Jogo\n");
                break;
            } else if (tecla == '2') {
                printf("Recordes\n");
                break;
            } else if (tecla == '3') {
                printf("Sair\n");
                tela_fim();
                return;
            }
        }
    }

    // Finalizar a tela (pode ser recolocada dependendo do fluxo do jogo)
    tela_fim();
}


// Funções de leitura de arquivo e inicialização
tabuleiro_t* ler_tabuleiros(const char* arquivo, int* num_tabuleiros) {
    FILE* f = fopen(arquivo, "r");
    if (!f) {
        perror("Erro ao abrir arquivo de tabuleiros");
        exit(1);
    }

    fscanf(f, "%d", num_tabuleiros);
    tabuleiro_t* tabuleiros = malloc(*num_tabuleiros * sizeof(tabuleiro_t));

    for (int i = 0; i < *num_tabuleiros; i++) {
        fscanf(f, "%d %d", &tabuleiros[i].id, &tabuleiros[i].dificuldade);
        for (int j = 0; j < 9; j++) {
            for (int k = 0; k < 9; k++) {
                fscanf(f, "%d", &tabuleiros[i].tabuleiro[j][k]);
            }
        }
    }

    fclose(f);
    return tabuleiros;
}

void inicializa_jogo(jogo_t* jogo, tabuleiro_t* tabuleiros, int num_tabuleiros) {
    srand(time(NULL));
    int indice = rand() % num_tabuleiros;
    tabuleiro_t tabuleiro_selecionado = tabuleiros[indice];

    jogo->tabuleiro_id = tabuleiro_selecionado.id;
    jogo->dificuldade = tabuleiro_selecionado.dificuldade;
    jogo->valor_jogada = 1;
    jogo->inicio_tempo = time(NULL);

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            jogo->casas[i][j].valor = tabuleiro_selecionado.tabuleiro[i][j];
            jogo->casas[i][j].inicial = (jogo->casas[i][j].valor != 0);
            for (int k = 0; k < 9; k++) {
                jogo->casas[i][j].marcacoes[k] = false;
            }
        }
    }
}

// Função para desenhar o tabuleiro na tela
void desenha_casa(int x, int y, casa_t casa, bool cursor) {
    int tamanho_casa = 50;
    ponto_t inicio = {x * tamanho_casa, y * tamanho_casa};
    retangulo_t ret = {inicio, {tamanho_casa, tamanho_casa}};
    cor_t cor_linha = {0.5, 0.5, 0.5, 1.0};
    cor_t cor_interna = {1, 1, 1, 1};

    if (cursor) {
        cor_linha = (cor_t){0, 1, 0, 1}; // Verde
    }

    tela_retangulo(ret, 2.0, cor_linha, cor_interna);

    if (casa.valor != 0) {
        char texto[2];
        sprintf(texto, "%d", casa.valor);
        ponto_t pos_texto = {inicio.x + 20, inicio.y + 20};
        cor_t cor_texto = {0, 0, 0, 1.0};
        tela_texto(pos_texto, 20, cor_texto, texto);
    } else {
        for (int i = 0; i < 9; i++) {
            if (casa.marcacoes[i]) {
                ponto_t pos_marcacao = {inicio.x + (i % 3) * 15, inicio.y + (i / 3) * 15};
                tela_texto(pos_marcacao, 10, cor_linha, "*");
            }
        }
    }
}

void desenha_tabuleiro(jogo_t jogo) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            bool cursor = (i == jogo.cursor_x && j == jogo.cursor_y);
            desenha_casa(i, j, jogo.casas[i][j], cursor);
        }
    }

    // Desenha a área de desistência
    ponto_t pos_desistencia = {500, 450};
    cor_t cor_texto = {0, 1, 0, 1};
    tela_texto(pos_desistencia, 20, cor_texto, "Desistir");
}


// Funções para processar entradas do usuário
void processa_entrada_mouse(jogo_t* jogo) {
    rato_t rato = tela_rato();
    desenhar_cursor_mouse(rato);

    // Determina a casa do tabuleiro sob o mouse
    int x = rato.posicao.x / 50;
    int y = rato.posicao.y / 50;

    if (x >= 0 && x < 9 && y >= 0 && y < 9) {
        jogo->cursor_x = x;
        jogo->cursor_y = y;

        if (rato.clicado[0]) { // Clique com o botão esquerdo
            casa_t* casa = &jogo->casas[y][x];
            if (!casa->inicial) {
                if (jogo->valor_jogada == 0) {
                    casa->valor = 0;
                } else {
                    casa->valor = jogo->valor_jogada;
                }
            }
        } else if (rato.clicado[1]) { // Clique com o botão direito
            casa_t* casa = &jogo->casas[y][x];
            if (casa->valor == 0) {
                if (jogo->valor_jogada == 0) {
                    for (int i = 0; i < 9; i++) {
                        casa->marcacoes[i] = false;
                    }
                } else {
                    casa->marcacoes[jogo->valor_jogada - 1] = !casa->marcacoes[jogo->valor_jogada - 1];
                }
            }
        }
    } else {
        jogo->cursor_x = -1;
        jogo->cursor_y = -1;
    }

    if (rato.posicao.x >= 500 && rato.posicao.y >= 450 && rato.clicado[0]) {
        printf("Jogo encerrado pelo jogador.");
        exit(0);
        
    }
}

void processa_entrada_teclado(jogo_t* jogo) {
    int tecla = tela_tecla();
    if (tecla >= '1' && tecla <= '9') {
        jogo->valor_jogada = tecla - '0';
    } else if (tecla == '0' || tecla == ' ') {
        jogo->valor_jogada = 0;
    }
}

// Funções de verificação do término do jogo
bool verifica_preenchimento_completo(jogo_t jogo) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (jogo.casas[i][j].valor == 0) {
                return false;
            }
        }
    }
    return true;
}

bool verifica_regras_sudoku(jogo_t jogo) {
    // Verifica linhas e colunas
    for (int i = 0; i < 9; i++) {
        bool linha[9] = {false};
        bool coluna[9] = {false};
        for (int j = 0; j < 9; j++) {
            if (jogo.casas[i][j].valor != 0) {
                if (linha[jogo.casas[i][j].valor - 1]) {
                    return false;
                }
                linha[jogo.casas[i][j].valor - 1] = true;
            }
            if (jogo.casas[j][i].valor != 0) {
                if (coluna[jogo.casas[j][i].valor - 1]) {
                    return false;
                }
                coluna[jogo.casas[j][i].valor - 1] = true;
            }
        }
    }

    // Verifica blocos 3x3
    for (int bloco_x = 0; bloco_x < 3; bloco_x++) {
        for (int bloco_y = 0; bloco_y < 3; bloco_y++) {
            bool bloco[9] = {false};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int x = bloco_x * 3 + i;
                    int y = bloco_y * 3 + j;
                    if (jogo.casas[x][y].valor != 0) {
                        if (bloco[jogo.casas[x][y].valor - 1]) {
                            return false;
                        }
                        bloco[jogo.casas[x][y].valor - 1] = true;
                    }
                }
            }
        }
    }

    return true;
}

void finalizar_jogo(jogo_t* jogo) {
    time_t fim_tempo = time(NULL);
    int tempo_total = difftime(fim_tempo, jogo->inicio_tempo);
    int pontuacao = 1285 + 97418 * (jogo->dificuldade + 1) / tempo_total;
    printf("Jogo finalizado! Pontuação: %d\n", pontuacao);

    pontuacao_t nova_pontuacao = {0};
    strcpy(nova_pontuacao.jogador, jogo->jogador);
    nova_pontuacao.tabuleiro_id = jogo->tabuleiro_id;
    nova_pontuacao.pontuacao = pontuacao;

    int num_pontuacoes;
    pontuacao_t* pontuacoes = ler_pontuacoes("pontuacoes.txt", &num_pontuacoes);
    pontuacoes = realloc(pontuacoes, (num_pontuacoes + 1) * sizeof(pontuacao_t));
    pontuacoes[num_pontuacoes] = nova_pontuacao;

    qsort(pontuacoes, num_pontuacoes + 1, sizeof(pontuacao_t), comparar_pontuacoes);
    escrever_pontuacoes("pontuacoes.txt", pontuacoes, num_pontuacoes + 1);

    free(pontuacoes);
}

// Funções para manipular arquivo de pontuações
pontuacao_t* ler_pontuacoes(const char* arquivo, int* num_pontuacoes) {
    FILE* f = fopen(arquivo, "r");
    if (!f) {
        *num_pontuacoes = 0;
        return malloc(0);
    }

    fscanf(f, "%d", num_pontuacoes);
    pontuacao_t* pontuacoes = malloc(*num_pontuacoes * sizeof(pontuacao_t));

    for (int i = 0; i < *num_pontuacoes; i++) {
        fscanf(f, "%s %d %d", pontuacoes[i].jogador, &pontuacoes[i].tabuleiro_id, &pontuacoes[i].pontuacao);
    }

    fclose(f);
    return pontuacoes;
}

void escrever_pontuacoes(const char* arquivo, pontuacao_t* pontuacoes, int num_pontuacoes) {
    FILE* f = fopen(arquivo, "w");
    if (!f) {
        perror("Erro ao abrir arquivo de pontuações para escrita");
        exit(1);
    }

    fprintf(f, "%d\n", num_pontuacoes);
    for (int i = 0; i < num_pontuacoes; i++) {
        fprintf(f, "%s %d %d\n", pontuacoes[i].jogador, pontuacoes[i].tabuleiro_id, pontuacoes[i].pontuacao);
    }

    fclose(f);
}

int comparar_pontuacoes(const void* a, const void* b) {
    pontuacao_t* pa = (pontuacao_t*)a;
    pontuacao_t* pb = (pontuacao_t*)b;
    return pb->pontuacao - pa->pontuacao;
}

int main() {
    exibir_menu_inicial();
    tamanho_t tamanho_tela = {600, 500};
    tela_inicio(tamanho_tela, "Sudoku");

    int num_tabuleiros;
    tabuleiro_t* tabuleiros = ler_tabuleiros("tabuleiros.txt", &num_tabuleiros);

    jogo_t jogo;
    inicializa_jogo(&jogo, tabuleiros, num_tabuleiros);

    while (true) {
        processa_entrada_mouse(&jogo);
        processa_entrada_teclado(&jogo);
        desenha_tabuleiro(jogo);

        tela_atualiza();

        if (verifica_preenchimento_completo(jogo) && verifica_regras_sudoku(jogo)) {
            finalizar_jogo(&jogo);
            break;
        }
    }

    tela_fim();
    free(tabuleiros);

    return 0;

}