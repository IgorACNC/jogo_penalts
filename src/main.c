#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Para função usleep()
#include <pthread.h> // Para threads
#include "keyboard.h"
#include "screen.h"

#define BARRA_LARGURA 6
#define GOL_LARGURA 32
#define TELA_LARGURA 80
#define GOL_X ((TELA_LARGURA - GOL_LARGURA) / 2)
#define GOL_Y 5
#define VELOCIDADE_BOLA 50000
#define POSICAO_INICIAL_BOLA_Y 10
#define AUMENTO_VELOCIDADE 1.6

#define MAX_JOGADORES 10

typedef struct {
    char nome[50];
    int pontuacao;
} Jogador;

int posGoleiro = (GOL_LARGURA - BARRA_LARGURA) / 2;
int direcaoGoleiro = 1;
int goleiroMovendo = 1;
int pontosPorGol[3] = {5, 3, 5}; // Pontos iniciais para cada chute ('q', 'w', 'e')

void exibirGol();
int animarBola(int direcaoChute, int *pontos);
void *movimentoGoleiro(void *arg);
void jogoPenaltis();
void exibirMenuInicial();
void salvarPontuacao(char *nome, int pontuacao);
void exibirRanking();

void exibirGol() {
    screenGotoxy(GOL_X, GOL_Y - 1);
    printf("|------------------------------|");

    screenGotoxy(GOL_X, GOL_Y);
    for (int i = 0; i < GOL_LARGURA; i++) {
        if (i >= posGoleiro && i < posGoleiro + BARRA_LARGURA) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    screenUpdate();
}

int animarBola(int direcaoChute, int *pontos) {
    int xInicial;
    int indicePontuacao;

    switch (direcaoChute) {
        case 'q': xInicial = GOL_X + 2; indicePontuacao = 0; break;
        case 'w': xInicial = GOL_X + 12; indicePontuacao = 1; break;
        case 'e': xInicial = GOL_X + 26; indicePontuacao = 2; break;
        default: return 0;
    }

    for (int y = POSICAO_INICIAL_BOLA_Y; y >= GOL_Y; y--) {
        screenClear();
        exibirGol();

        screenGotoxy(xInicial, y);
        printf("o");
        screenUpdate();

        if (y == GOL_Y && xInicial >= posGoleiro + GOL_X && xInicial < posGoleiro + GOL_X + BARRA_LARGURA) {
            return 1;
        }

        usleep(VELOCIDADE_BOLA);
    }

    *pontos += pontosPorGol[indicePontuacao];
    pontosPorGol[indicePontuacao]++;
    return 0;
}

void *movimentoGoleiro(void *arg) {
    int *velocidadeGoleiro = (int *)arg;

    while (goleiroMovendo) {
        posGoleiro += direcaoGoleiro;
        if (posGoleiro <= 0 || posGoleiro >= GOL_LARGURA - BARRA_LARGURA) {
            direcaoGoleiro *= -1;
        }
        usleep(*velocidadeGoleiro);
    }
    return NULL;
}

void jogoPenaltis() {
    int tecla;
    int pontos = 0;
    int tentativas = 5;
    int gols = 0;
    int velocidadeGoleiro = 30000;
    
    pthread_t goleiroThread;
    pthread_create(&goleiroThread, NULL, movimentoGoleiro, &velocidadeGoleiro);

    while (tentativas > 0) {
        exibirGol();
        screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
        printf("o");
        screenUpdate();

        if (keyhit()) {
            tecla = readch();
            if (tecla == 'q' || tecla == 'w' || tecla == 'e') {
                screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                printf(" ");
                screenUpdate();

                if (animarBola(tecla, &pontos)) {
                    screenGotoxy(GOL_X, GOL_Y + 2);
                    printf("Defesa do goleiro!");
                } else {
                    screenGotoxy(GOL_X, GOL_Y + 2);
                    printf("Gol!!!");
                    gols++;
                }
                
                tentativas--;
                screenGotoxy(GOL_X, GOL_Y + 3);
                printf("Tentativas restantes: %d", tentativas);
                screenGotoxy(GOL_X, GOL_Y + 4);
                printf("Gols marcados: %d", gols);

                screenUpdate();
                sleep(1);

                velocidadeGoleiro = (int)(velocidadeGoleiro / AUMENTO_VELOCIDADE);

                if (tentativas > 0) {
                    screenClear();
                }
            }
        }
    }

    if (gols == 5) {
        tentativas = 1;
        screenClear();
        screenGotoxy(GOL_X, GOL_Y + 2);
        printf("Rodada Bonus! Você ganhou uma chance extra!");
        screenUpdate();
        sleep(2);

        while (tentativas > 0) {
            exibirGol();
            screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
            printf("o");
            screenUpdate();

            if (keyhit()) {
                tecla = readch();
                if (tecla == 'q' || tecla == 'w' || tecla == 'e') {
                    screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                    printf(" ");
                    screenUpdate();

                    if (animarBola(tecla, &pontos)) {
                        screenGotoxy(GOL_X, GOL_Y + 2);
                        printf("Defesa do goleiro!");
                    } else {
                        screenGotoxy(GOL_X, GOL_Y + 2);
                        printf("Gol na rodada bonus!");
                        gols++;
                    }

                    tentativas--;
                    screenUpdate();
                    sleep(1);
                }
            }
        }
    }

    goleiroMovendo = 0;
    pthread_join(goleiroThread, NULL);

    char nome[50];
    screenClear();
    screenGotoxy(GOL_X, GOL_Y + 2);
    printf("Fim do jogo! Voce marcou %d ponto(s)!", pontos);
    screenGotoxy(GOL_X, GOL_Y + 4);
    printf("Digite seu nome: ");
    screenUpdate();
    scanf("%49s", nome);

    salvarPontuacao(nome, pontos);
    exibirRanking();
    screenUpdate();
}

void salvarPontuacao(char *nome, int pontuacao) {
    Jogador ranking[MAX_JOGADORES + 1];
    int totalJogadores = 0;

    FILE *arquivo = fopen("ranking.txt", "r");
    if (arquivo != NULL) {
        while (fscanf(arquivo, "%49s %d", ranking[totalJogadores].nome, &ranking[totalJogadores].pontuacao) == 2) {
            totalJogadores++;
        }
        fclose(arquivo);
    }

    ranking[totalJogadores].pontuacao = pontuacao;
    strcpy(ranking[totalJogadores].nome, nome);
    totalJogadores++;

    for (int i = 0; i < totalJogadores - 1; i++) {
        for (int j = i + 1; j < totalJogadores; j++) {
            if (ranking[j].pontuacao > ranking[i].pontuacao) {
                Jogador temp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = temp;
            }
        }
    }

    if (totalJogadores > MAX_JOGADORES) {
        totalJogadores = MAX_JOGADORES;
    }

    arquivo = fopen("ranking.txt", "w");
    for (int i = 0; i < totalJogadores; i++) {
        fprintf(arquivo, "%-20s %d\n", ranking[i].nome, ranking[i].pontuacao);
    }
    fclose(arquivo);
}

void exibirRanking() {
    FILE *arquivo = fopen("ranking.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de ranking.\n");
        return;
    }

    screenClear();
    screenGotoxy(GOL_X, GOL_Y);
    printf("Ranking dos Top 10 Jogadores:\n");

    int linha = GOL_Y + 2;
    char nome[50];
    int pontuacao;
    while (fscanf(arquivo, "%49s %d", nome, &pontuacao) == 2) {
        screenGotoxy(GOL_X, linha);
        printf("%-10s %d", nome, pontuacao);
        linha++;
    }
    fclose(arquivo);

    // Espera até que o usuário digite '0' para sair da tela de ranking
    int sair = -1;
    screenGotoxy(GOL_X, linha + 2);
    printf("Digite 0 para voltar ao menu...");
    screenUpdate();

    while (sair != 0) {
        if (keyhit()) {
            sair = readch() - '0'; // Lê o caractere digitado e converte para inteiro
        }
    }
}


void exibirMenuInicial() {
    screenClear();
    screenGotoxy(GOL_X, GOL_Y + 1);
    printf("Bem-vindo ao Jogo de Penalti!\n\n");

    screenGotoxy(GOL_X + 7, GOL_Y + 4);
    printf("Escolha seu time:");
    screenGotoxy(GOL_X + 10, GOL_Y + 5);
    printf("1 - Sport");
    screenGotoxy(GOL_X + 10, GOL_Y + 6);
    printf("2 - Nautico");
    screenGotoxy(GOL_X + 10, GOL_Y + 7);
    printf("3 - Santa Cruz");

    screenGotoxy(GOL_X, GOL_Y + 10);
    printf("Pressione qualquer tecla para iniciar o jogo...");
    screenUpdate();
    getchar();
}

int main() {
    exibirMenuInicial();
    screenClear();
    jogoPenaltis();
    screenGotoxy(GOL_X, GOL_Y + 2);
    printf("Obrigado por jogar!");
    screenUpdate();
    return 0;
}