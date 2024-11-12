#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Para função usleep()
#include <pthread.h> // Para threads
#include "keyboard.h"
#include "screen.h"

#define BARRA_LARGURA 8 // Aumentado de 6 para 8 caracteres
#define GOL_LARGURA 32
#define TELA_LARGURA 80
#define GOL_X ((TELA_LARGURA - GOL_LARGURA) / 2)
#define GOL_Y 5
#define VELOCIDADE_BOLA 70000
#define POSICAO_INICIAL_BOLA_Y 10
#define AUMENTO_VELOCIDADE 1.7

#define MAX_JOGADORES 10

typedef struct {
    char nome[50];
    int pontuacao;
} Jogador;

int posGoleiro1 = (GOL_LARGURA - BARRA_LARGURA) / 2;
int posGoleiro2 = GOL_LARGURA - BARRA_LARGURA; // Segundo goleiro começa do outro lado
int direcaoGoleiro1 = 1;
int direcaoGoleiro2 = -1; // Direção oposta
int goleiroMovendo = 1;
int modoDificil = 0; // Flag para o modo difícil
int pontosPorGol[3] = {2, 4, 2};

void exibirGol();
int animarBola(int direcaoChute, int *pontos);
void *movimentoGoleiro(void *arg);
void jogoPenaltis();
void exibirMenuInicial();
void salvarPontuacao(char *nome, int pontuacao);
void exibirRanking();

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
        if ((i >= posGoleiro1 && i < posGoleiro1 + BARRA_LARGURA) || 
            (modoDificil && i >= posGoleiro2 && i < posGoleiro2 + BARRA_LARGURA)) {
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

        if (y == GOL_Y && 
            ((xInicial >= posGoleiro1 + GOL_X && xInicial < posGoleiro1 + GOL_X + BARRA_LARGURA) ||
            (modoDificil && xInicial >= posGoleiro2 + GOL_X && xInicial < posGoleiro2 + GOL_X + BARRA_LARGURA))) {
            return 1; // Defesa do goleiro
        }

        usleep(VELOCIDADE_BOLA);
    }

    // Ajuste da pontuação de acordo com o modo
    if (modoDificil) {
        if (indicePontuacao == 0 || indicePontuacao == 2) {
            *pontos += 6;
        } else {
            *pontos += 9;
        }
    } else {
        *pontos += pontosPorGol[indicePontuacao];
        pontosPorGol[indicePontuacao]++;
    }
    return 0; // Gol marcado
}

void *movimentoGoleiro(void *arg) {
    int *velocidadeGoleiro = (int *)arg;

    while (goleiroMovendo) {
        posGoleiro1 += direcaoGoleiro1;
        if (modoDificil) posGoleiro2 += direcaoGoleiro2;

        if (posGoleiro1 <= 0 || posGoleiro1 >= GOL_LARGURA - BARRA_LARGURA) {
            direcaoGoleiro1 *= -1;
        }

        if (modoDificil && (posGoleiro2 <= 0 || posGoleiro2 >= GOL_LARGURA - BARRA_LARGURA)) {
            direcaoGoleiro2 *= -1;
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
    int velocidadeGoleiro = 25000;
    
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
    printf("Digite 0 para voltar ao menu:");
    screenUpdate();

    while (sair != 0) {
        if (keyhit()) {
            sair = readch() - '0'; // Lê o caractere digitado e converte para inteiro
        }
    }
}


void exibirMenuInicial() {
    screenClear();
    screenGotoxy(GOL_X - 12, GOL_Y + 1);
    printf("     _______ _______ __    _ _______ ___   _______ _______ ");
    screenGotoxy(GOL_X - 12, GOL_Y + 2);
    printf("    |       |       |  |  | |   _   |   | |       |       |");
    screenGotoxy(GOL_X - 12, GOL_Y + 3);
    printf("    |    _  |    ___|   |_| |  |_|  |   | |_     _|  _____|");
    screenGotoxy(GOL_X - 12, GOL_Y + 4);
    printf("    |   |_| |   |___|       |       |   |   |   | | |_____ ");
    screenGotoxy(GOL_X - 12, GOL_Y + 5);
    printf("    |    ___|    ___|  _    |       |   |___|   | |_____  |");
    screenGotoxy(GOL_X - 12, GOL_Y + 6);
    printf("    |   |   |   |___| | |   |   _   |       |   |  _____| |");
    screenGotoxy(GOL_X - 12, GOL_Y + 7);
    printf("    |___|   |_______|_|  |__|__| |__|_______|___| |_______|");

    screenGotoxy(GOL_X + 7, GOL_Y + 10);
    printf("Escolha o Modo de Jogo:");
    screenGotoxy(GOL_X + 10, GOL_Y + 11);
    printf("1 - Fácil");
    screenGotoxy(GOL_X + 10, GOL_Y + 12);
    printf("2 - Difícil");

    screenGotoxy(GOL_X + 10, GOL_Y + 15);
    printf("Digite o número do modo: ");
    screenUpdate();

    int modo;
    scanf("%d", &modo);
    modoDificil = (modo == 2); // Define o modo difícil se a entrada for '2'
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