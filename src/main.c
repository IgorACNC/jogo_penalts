#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Para função usleep()
#include <pthread.h> // Para threads
#include "keyboard.h"
#include "screen.h"

// Tamanho e posição da barra e do gol
#define BARRA_LARGURA 6        // A barra do goleiro é agora exatamente 6 caracteres de largura
#define GOL_LARGURA 32
#define TELA_LARGURA 80        // Largura da tela
#define GOL_X ((TELA_LARGURA - GOL_LARGURA) / 2) // Centraliza o gol
#define GOL_Y 5
#define VELOCIDADE_BOLA 50000      // Velocidade de movimento da bola (0.05s)
#define POSICAO_INICIAL_BOLA_Y 10  // Linha inicial da bola
#define AUMENTO_VELOCIDADE 1.6     // Fator de aumento de 60%

// Variáveis globais para controlar o goleiro
int posGoleiro = (GOL_LARGURA - BARRA_LARGURA) / 2; // Posição inicial do goleiro
int direcaoGoleiro = 1;
int goleiroMovendo = 1; // Flag para controlar a movimentação do goleiro

// Função para exibir o gol com a barra do goleiro
void exibirGol() {
    screenGotoxy(GOL_X, GOL_Y - 1);
    printf("|------------------------------|");

    screenGotoxy(GOL_X, GOL_Y);
    for (int i = 0; i < GOL_LARGURA; i++) {
        if (i >= posGoleiro && i < posGoleiro + BARRA_LARGURA) {
            printf("#"); // Representa a barra do goleiro
        } else {
            printf(" ");
        }
    }
    screenUpdate();
}

// Função para animar o movimento da bola em direção ao gol
int animarBola(int direcaoChute) {
    int xInicial;

    // Define a posição inicial da bola com base na direção do chute
    switch (direcaoChute) {
        case 'q': xInicial = GOL_X + 2; break;      // Direção esquerda
        case 'w': xInicial = GOL_X + 12; break;     // Direção central
        case 'e': xInicial = GOL_X + 26; break;     // Direção direita (mais à direita)
        default: return 0; // Retorna se a direção for inválida
    }

    // Move a bola em direção ao gol
    for (int y = POSICAO_INICIAL_BOLA_Y; y >= GOL_Y; y--) {
        screenClear();
        
        // Exibe o gol com a posição atual do goleiro
        exibirGol();

        // Desenha a bola na posição atual
        screenGotoxy(xInicial, y);
        printf("o");
        screenUpdate();

        // Verifica a colisão com a barra do goleiro
        if (y == GOL_Y && xInicial >= posGoleiro + GOL_X && xInicial < posGoleiro + GOL_X + BARRA_LARGURA) {
            return 1; // Defesa do goleiro
        }

        // Atraso entre os movimentos da bola
        usleep(VELOCIDADE_BOLA);
    }

    return 0; // Gol
}

// Função para movimentação contínua do goleiro
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

// Função para o jogo de pênaltis com movimento do goleiro e tentativas de chute
void jogoPenaltis() {
    int tecla;
    int gols = 0;
    int tentativas = 5;
    int velocidadeGoleiro = 100000; // Velocidade inicial do goleiro (0.1s)
    
    // Cria uma thread para movimentar o goleiro
    pthread_t goleiroThread;
    pthread_create(&goleiroThread, NULL, movimentoGoleiro, &velocidadeGoleiro);

    while (tentativas > 0) {
        exibirGol();

        // Exibe a bola pronta para chute no centro da tela
        screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
        printf("o");
        screenUpdate();

        // Aguarda o chute do jogador
        if (keyhit()) {
            tecla = readch();
            if (tecla == 'q' || tecla == 'w' || tecla == 'e') {
                screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                printf(" ");
                screenUpdate();

                // Anima a bola
                if (animarBola(tecla)) {
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

                // Aumenta a velocidade do goleiro após cada chute
                velocidadeGoleiro = (int)(velocidadeGoleiro / AUMENTO_VELOCIDADE);

                if (tentativas > 0) {
                    screenClear();
                }
            }
        }
    }

    // Termina a movimentação do goleiro e espera a thread finalizar
    goleiroMovendo = 0;
    pthread_join(goleiroThread, NULL);

    screenClear();
    screenGotoxy(GOL_X, GOL_Y + 2);
    printf("Fim do jogo! Voce marcou %d gol(s)!", gols);
    screenUpdate();
}

// Função para exibir o menu inicial e selecionar o time
void exibirMenuInicial() {
    int timeSelecionado = 0;

    while (timeSelecionado < 1 || timeSelecionado > 4) {
        screenClear();
        screenGotoxy(GOL_X, GOL_Y);
        printf("Bem-vindo ao PEnalts!");
        screenGotoxy(GOL_X, GOL_Y + 1);
        printf("Escolha seu time:");
        screenGotoxy(GOL_X, GOL_Y + 2);
        printf("1 - Santa Cruz");
        screenGotoxy(GOL_X, GOL_Y + 3);
        printf("2 - Nautico");
        screenGotoxy(GOL_X, GOL_Y + 4);
        printf("3 - Sport");

        screenUpdate();
        
        if (keyhit()) {
            timeSelecionado = readch() - '0';  // Converte a entrada em número
        }
    }

    screenClear();
    screenGotoxy(GOL_X, GOL_Y);
    printf("Voce escolheu: ");
    if (timeSelecionado == 1) printf("Santa Cruz!");
    else if (timeSelecionado == 2) printf("Nautico!");
    else if (timeSelecionado == 3) printf("Sport!");
    else if (timeSelecionado == 4) printf("Retro!");

    screenUpdate();
    sleep(2);
    screenClear();
}

int main() {
    keyboardInit();
    screenInit(1);

    exibirMenuInicial();  // Exibe o menu inicial e permite seleção de time
    jogoPenaltis();       // Inicia o jogo

    screenDestroy();
    keyboardDestroy();

    return 0;
}