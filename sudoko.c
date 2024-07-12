#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "telag2.h"
#include <time.h>

#define LARGURA_TELA 600
#define ALTURA_TELA 500

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
    int valor;
    bool inicial;
    bool marca[9]; // Marcações de 1 a 9
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
tamanho_t tela_texto_dimensoes(int altura_fonte, const char* texto);

void desenhar_cursor_mouse(rato_t rato) {
    cor_t cor_cursor = {1, 0, 0, 1};
    ponto_t posicao = {rato.posicao.x, rato.posicao.y};
    circulo_t cursor = {posicao, 5};
    tela_circulo(cursor, 1, cor_cursor, cor_cursor);
}

tamanho_t tela_texto_dimensoes(int altura_fonte, const char* texto) {
    tamanho_t dimensoes;
    dimensoes.largura = strlen(texto) * (altura_fonte / 2); // Largura média de caracteres ajustável
    dimensoes.altura = altura_fonte; // Altura da fonte

    return dimensoes;
}


void desenhar_botao_centralizado(botao_t botao, cor_t cor_fundo, cor_t cor_borda, cor_t cor_texto, const char* texto) {
    // Desenha retângulo de fundo
    tela_retangulo((retangulo_t){botao.posicao, botao.tamanho}, 2, cor_borda, cor_fundo);

    // Calcula a posição do texto para centralizá-lo no retângulo
    int altura_fonte = 20;
    tamanho_t dimensoes_texto = tela_texto_dimensoes(altura_fonte, texto);
    ponto_t pos_texto = {
        botao.posicao.x + (botao.tamanho.largura - dimensoes_texto.largura) / 2,
        botao.posicao.y + (botao.tamanho.altura - altura_fonte) / 2 + altura_fonte / 2
    };

    // Desenha o texto centralizado no botão
    tela_texto(pos_texto, altura_fonte, cor_texto, (char*)texto);
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

// Funções para processar entradas do usuário
void processa_entrada_mouse(jogo_t* jogo) {
    rato_t rato = tela_rato();
    desenhar_cursor_mouse(rato);

    // Determina a casa do tabuleiro sob o mouse
    int x = rato.posicao.x / 50; // Tamanho da casa é 50 pixels
    int y = rato.posicao.y / 50;

    // Verifica se a posição está dentro dos limites do tabuleiro
    if (x >= 0 && x < 9 && y >= 0 && y < 9) {
        jogo->cursor_x = x;
        jogo->cursor_y = y;

        // Verifica qual botão foi clicado
        if (rato.clicado[0]) { // Botão esquerdo do mouse
            casa_t* casa = &jogo->casas[x][y];
            if (!casa->inicial) {
                casa->valor = jogo->valor_jogada; // Define o valor da casa
            }
        } else if (rato.clicado[1]) { // Botão direito do mouse
            casa_t* casa = &jogo->casas[y][x];
            if (casa->valor == 0) {
                  int num_marca = jogo->valor_jogada - 1; // Converte valor de jogada para índice de marcação (0 a 8)
                casa->marca[num_marca] = !casa->marca[num_marca]; // Alterna a marcação
    
                // Verifica se a marcação é válida (apenas uma marcação por vez)
                if (casa->marca[num_marca]) {
       
                    for (int i = 0; i < 9; i++) {
                        // Desmarca outras casas na mesma linha
                        if (i != x) jogo->casas[x][i].marca[num_marca] = false;
                        if (i != y) jogo->casas[i][y].marca[num_marca] = false;
                    }

                int bloco_x = x / 3;
                int bloco_y = y / 3;
                for (int i = bloco_y * 3; i < bloco_y * 3 + 3; i++) {
                    for (int j = bloco_x * 3; j < bloco_x * 3 + 3; j++) {
                        if (i != y && j != x) jogo->casas[i][j].marca[num_marca] = false;
                    }       
                }

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
                jogo->casas[i][j].marca[k] = false;
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
            if (casa.marca[i]) {
                ponto_t pos_marcacao = {inicio.x + (i % 3) * 15, inicio.y + (i / 3) * 15};
                tela_texto(pos_marcacao, 10, cor_linha, "*");
            }
        }
    }
}

void desenha_tabuleiro(jogo_t jogo) {
    // Desenha as casas do tabuleiro
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            bool cursor = (i == jogo.cursor_x && j == jogo.cursor_y);
            desenha_casa(i, j, jogo.casas[i][j], cursor);
        }
    }

    // Desenha a área de desistência
    ponto_t pos_desistencia = {500, 450};
    tamanho_t tamanho_desistencia = {90, 40};
    cor_t cor_fundo_desistencia = {0.2, 0.2, 0.2, 1.0};
    cor_t cor_borda_desistencia = {0, 1, 0, 1};
    cor_t cor_texto = {0, 1, 0, 1};

    // Desenha retângulo de fundo
    retangulo_t retangulo_desistencia = {pos_desistencia, tamanho_desistencia};
    tela_retangulo(retangulo_desistencia, 2, cor_borda_desistencia, cor_fundo_desistencia);

    // Calcula a posição do texto para centralizá-lo no retângulo
    int altura_fonte = 20;
    ponto_t pos_texto = {
        pos_desistencia.x + (tamanho_desistencia.largura - tela_texto_dimensoes(altura_fonte, "Desistir").largura) / 2,
        pos_desistencia.y + (tamanho_desistencia.altura - altura_fonte) / 2 + altura_fonte / 2
    };
    
    // Desenha o texto "Desistir" centralizado no botão
    tela_texto(pos_texto, altura_fonte, cor_texto, "Desistir");

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

void exibir_menu_inicial() {
    tamanho_t tamanho_tela = {LARGURA_TELA, ALTURA_TELA};
    tela_inicio(tamanho_tela, "Lucas Sudoku Game");

    cor_t cor_fundo = {0, 0, 0, 1};
    cor_t cor_texto = {0, 1, 0, 1};
    cor_t cor_botao_fundo = {0.2, 0.2, 0.2, 1};
    cor_t cor_botao_borda = {0, 1, 0, 1};

    // Centraliza o título
    int altura_fonte_titulo = 20;
    tamanho_t tamanho_titulo = tela_texto_dimensoes(altura_fonte_titulo, "LUCAS SUDOKU GAME :)");
    ponto_t posicao_titulo = {
        (LARGURA_TELA - tamanho_titulo.largura) / 2,
        ALTURA_TELA / 5
    };

    // Centraliza os botões
    int largura_botao = 300;
    int altura_botao = 60;
    ponto_t posicao_jogar = {LARGURA_TELA / 2 - largura_botao / 2, ALTURA_TELA / 2 - 90};
    ponto_t posicao_recordes = {LARGURA_TELA / 2 - largura_botao / 2, ALTURA_TELA / 2 - 30};
    ponto_t posicao_help = {LARGURA_TELA / 2 - largura_botao / 2, ALTURA_TELA / 2 + 30};
    ponto_t posicao_sair = {LARGURA_TELA / 2 - largura_botao / 2, ALTURA_TELA / 2 + 90};

    botao_t botao_jogar = {posicao_jogar, {largura_botao, altura_botao}};
    botao_t botao_recordes = {posicao_recordes, {largura_botao, altura_botao}};
    botao_t botao_help = {posicao_help, {largura_botao, altura_botao}};
    botao_t botao_sair = {posicao_sair, {largura_botao, altura_botao}};

    while (1) {
        tela_atualiza();

        tela_texto(posicao_titulo, altura_fonte_titulo, cor_texto, (char*)"LUCAS SUDOKU GAME");

        desenhar_botao_centralizado(botao_jogar, cor_botao_fundo, cor_botao_borda, cor_texto, "Jogar");
        desenhar_botao_centralizado(botao_recordes, cor_botao_fundo, cor_botao_borda, cor_texto, "Recordes");
        desenhar_botao_centralizado(botao_help, cor_botao_fundo, cor_botao_borda, cor_texto, "Help");
        desenhar_botao_centralizado(botao_sair, cor_botao_fundo, cor_botao_borda, cor_texto, "Sair");

        // Verificar cliques do mouse
        rato_t rato = tela_rato();
        desenhar_cursor_mouse(rato);

        if (rato.clicado[0]) {  // Clique do botão esquerdo do mouse
            if (rato.posicao.x >= botao_jogar.posicao.x &&
                rato.posicao.x <= botao_jogar.posicao.x + botao_jogar.tamanho.largura &&
                rato.posicao.y >= botao_jogar.posicao.y &&
                rato.posicao.y <= botao_jogar.posicao.y + botao_jogar.tamanho.altura) {
                printf("Jogar\n");
                tamanho_t tamanho_tela = {600, 500};
                tela_inicio(tamanho_tela, "Sudoku do Luketa");
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
            } else if (rato.posicao.x >= botao_recordes.posicao.x &&
                       rato.posicao.x <= botao_recordes.posicao.x + botao_recordes.tamanho.largura &&
                       rato.posicao.y >= botao_recordes.posicao.y &&
                       rato.posicao.y <= botao_recordes.posicao.y + botao_recordes.tamanho.altura) {
                printf("Recordes\n");
                break;
            } else if (rato.posicao.x >= botao_help.posicao.x &&
                       rato.posicao.x <= botao_help.posicao.x + botao_help.tamanho.largura &&
                       rato.posicao.y >= botao_help.posicao.y &&
                       rato.posicao.y <= botao_help.posicao.y + botao_help.tamanho.altura) {
                printf("Help\n");
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
                printf("Jogar\n");
                break;
            } else if (tecla == '2') {
                printf("Recordes\n");
                break;
            } else if (tecla == '3') {
                printf("Help\n");
                break;
            } else if (tecla == '4') {
                printf("Sair\n");
                tela_fim();
                return;
            }
        }
    }

    // Finalizar a tela (pode ser recolocada dependendo do fluxo do jogo)
    tela_fim();
}

// Função principal do jogo
int main() {
    exibir_menu_inicial();
}
