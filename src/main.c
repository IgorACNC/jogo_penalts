#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // Para função usleep()
#include "keyboard.h"
#include "screen.h"

// Tamanho e posição da barra e do gol
#define BARRA_LARGURA 6        // A barra do goleiro é agora exatamente 6 caracteres de largura
#define GOL_LARGURA 32
#define GOL_X 8
#define GOL_Y 5
#define VELOCIDADE_GOLEIRO 100000  // Intervalo em microssegundos (0.1s) para mover o goleiro
#define VELOCIDADE_BOLA 50000      // Velocidade de movimento da bola (0.05s)
#define POSICAO_INICIAL_BOLA_Y 10  // Linha inicial da bola

// Função para exibir o gol com a barra do goleiro
void exibirGol(int pos) {
    // Exibe o travessão superior
    screenGotoxy(GOL_X, GOL_Y - 1);
    printf("|------------------------------|");

    // Exibe o gol com a barra do goleiro
    screenGotoxy(GOL_X, GOL_Y);
    for (int i = 0; i < GOL_LARGURA; i++) {
        if (i >= pos && i < pos + BARRA_LARGURA) {
            printf("#"); // Representa a barra do goleiro
        } else {
            printf(" ");
        }
    }
    screenUpdate();
}

// Função para animar o movimento da bola em direção ao gol
int animarBola(int direcaoChute, int posGoleiro) {
    int xInicial;

    // Define a posição inicial da bola com base na direção do chute
    switch (direcaoChute) {
        case 'q': xInicial = GOL_X + 2; break;      // Direção esquerda
        case 'w': xInicial = GOL_X + 12; break;     // Direção central
        case 'e': xInicial = GOL_X + 22; break;     // Direção direita
        default: return 0; // Retorna se a direção for inválida
    }

    // Anima a bola do jogador (parte inferior da tela) até o gol (parte superior)
    for (int y = POSICAO_INICIAL_BOLA_Y; y >= GOL_Y; y--) {
        screenClear();

        // Reexibe o gol e o goleiro
        exibirGol(posGoleiro);

        // Desenha a bola na posição atual
        screenGotoxy(xInicial, y);
        printf("o"); // Representa a bola
        screenUpdate();
        usleep(VELOCIDADE_BOLA);

        // Verifica se o goleiro intercepta a bola na linha do gol
        if (y == GOL_Y && xInicial >= posGoleiro + GOL_X && xInicial < posGoleiro + GOL_X + BARRA_LARGURA) {
            return 1; // Defesa do goleiro
        }
    }

    return 0; // Gol
}

// Função para o jogo de pênaltis com movimento automático do goleiro e tentativa de chute
void jogoPenaltis() {
    int posGoleiro = (GOL_LARGURA - BARRA_LARGURA) / 2;  // Posição inicial do goleiro no centro
    int direcaoGoleiro = 1;  // Direção inicial do movimento do goleiro
    int tecla;
    int gols = 0;
    int tentativas = 5;

    while (tentativas > 0) {
        // Atualiza a posição do goleiro
        exibirGol(posGoleiro);

        // Move o goleiro para a esquerda ou direita
        posGoleiro += direcaoGoleiro;
        
        // Verifica se o goleiro atingiu uma das bordas e inverte a direção
        if (posGoleiro <= 0 || posGoleiro >= GOL_LARGURA - BARRA_LARGURA) {
            direcaoGoleiro *= -1;  // Inverte a direção
        }

        // Exibe a bola no centro da tela, pronta para o chute
        screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
        printf("o"); // Exibe a bola no centro
        screenUpdate();

        // Aguarda o jogador escolher a direção do chute
        if (keyhit()) {
            tecla = readch();
            if (tecla == 'q' || tecla == 'w' || tecla == 'e') {
                // Limpa a bola inicial e anima a bola em direção ao gol
                screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                printf(" "); // Limpa a bola inicial
                screenUpdate();

                // Anima a bola e verifica o resultado (gol ou defesa)
                if (animarBola(tecla, posGoleiro)) {
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
                sleep(1); // Espera um momento para exibir o resultado

                if (tentativas > 0) {
                    screenClear();  // Limpa a tela para a próxima tentativa
                }
            }
        }

        usleep(VELOCIDADE_GOLEIRO);  // Controla a velocidade do movimento do goleiro
    }

    // Resultado final
    screenClear();
    screenGotoxy(GOL_X, GOL_Y + 2);
    printf("Fim do jogo! Voce marcou %d gol(s)!", gols);
    screenUpdate();
}

int main() {
    keyboardInit();  // Inicializa o teclado
    screenInit(1);   // Inicializa a tela com bordas

    jogoPenaltis();  // Inicia o jogo

    screenDestroy();  // Limpa a tela
    keyboardDestroy();

    return 0;
}