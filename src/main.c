#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // Para função usleep()
#include <pthread.h> // Para threads
#include "keyboard.h"
#include "screen.h"
#include "timer.h"

#define BARRA_LARGURA 8
#define GOL_LARGURA 32
#define TELA_LARGURA 80
#define GOL_X ((TELA_LARGURA - GOL_LARGURA) / 2)
#define GOL_Y 5
#define VELOCIDADE_BOLA 70000
#define POSICAO_INICIAL_BOLA_Y 10
#define AUMENTO_VELOCIDADE 1.7
#define MAX_JOGADORES 10
#define JOGADOR_ALTURA 3
#define JOGADOR_LARGURA 5

const char jogador[JOGADOR_ALTURA][JOGADOR_LARGURA] = {
    " O ",
    "/|\\",
    "/ \\"
};

typedef struct{
    char nome[50];
    int pontuacao;
} Jogador;

int posGoleiro1 = (GOL_LARGURA - BARRA_LARGURA) / 2;
int posGoleiro2 = GOL_LARGURA - BARRA_LARGURA; // Segundo goleiro começa na direção oposta
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

void exibirJogador(int x, int y){
    for (int i = 0; i < JOGADOR_ALTURA; i++){
        screenGotoxy(x, y + i);
        printf("%s", jogador[i]);
    }
    screenUpdate();
}

void exibirGol(){
    screenSetColor(GREEN, LIGHTGREEN);
    screenGotoxy(GOL_X, GOL_Y - 1);
    printf("|------------------------------|");

    screenGotoxy(GOL_X, GOL_Y);
    for (int i = 0; i < GOL_LARGURA; i++){
        if ((i >= posGoleiro1 && i < posGoleiro1 + BARRA_LARGURA) || 
            (modoDificil && i >= posGoleiro2 && i < posGoleiro2 + BARRA_LARGURA)) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    screenUpdate();
}

int animarBola(int direcaoChute, int *pontos){
    int xInicial;
    int indicePontuacao;

    switch (direcaoChute){
        case 'q': xInicial = GOL_X + 2; indicePontuacao = 0; break;
        case 'w': xInicial = GOL_X + 12; indicePontuacao = 1; break;
        case 'e': xInicial = GOL_X + 26; indicePontuacao = 2; break;
        default: return 0;
    }

    for (int y = POSICAO_INICIAL_BOLA_Y; y >= GOL_Y; y--){
        screenClear();
        exibirGol();

        // Exibe a bola
        screenGotoxy(xInicial, y);
        printf("o");

        // Exibe o jogador abaixo da bola
        exibirJogador(xInicial - 1, y + 2);

        screenUpdate();

        if (y == GOL_Y && 
            ((xInicial >= posGoleiro1 + GOL_X && xInicial < posGoleiro1 + GOL_X + BARRA_LARGURA) ||
            (modoDificil && xInicial >= posGoleiro2 + GOL_X && xInicial < posGoleiro2 + GOL_X + BARRA_LARGURA))){
            return 1;
        }

        usleep(VELOCIDADE_BOLA);
    }

    if (modoDificil){
        if (indicePontuacao == 0 || indicePontuacao == 2) {
            *pontos += 6;
        } else {
            *pontos += 9;
        }
    }else{
        *pontos += pontosPorGol[indicePontuacao];
        pontosPorGol[indicePontuacao]++;
    }
    return 0;
}

void *movimentoGoleiro(void *arg){
    int *velocidadeGoleiro = (int *)arg;

    while (goleiroMovendo){
        posGoleiro1 += direcaoGoleiro1;
        if (modoDificil) posGoleiro2 += direcaoGoleiro2;

        if (posGoleiro1 <= 0 || posGoleiro1 >= GOL_LARGURA - BARRA_LARGURA){
            direcaoGoleiro1 *= -1;
        }

        if (modoDificil && (posGoleiro2 <= 0 || posGoleiro2 >= GOL_LARGURA - BARRA_LARGURA)) {
            direcaoGoleiro2 *= -1;
        }

        usleep(*velocidadeGoleiro);
    }
    return NULL;
}

void jogoPenaltis(){
    screenInit(1);
    int tecla;
    int pontos = 0;
    int tentativas = 5;
    int gols = 0;
    int velocidadeGoleiro = 25000;
    
    pthread_t goleiroThread;
    pthread_create(&goleiroThread, NULL, movimentoGoleiro, &velocidadeGoleiro);

    while (tentativas > 0){
        exibirGol();
        screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
        printf("o");
        screenUpdate();

        if (keyhit()) {
            tecla = readch();
            if (tecla == 'q' || tecla == 'w' || tecla == 'e'){
                screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                printf(" ");
                screenUpdate();

                if (animarBola(tecla, &pontos)){
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

                if (tentativas > 0){
                    screenClear();
                }
            }
        }
    }

    if (gols == 5){
        tentativas = 1;
        screenClear();
        screenGotoxy(GOL_X, GOL_Y + 2);
        printf("Rodada Bonus! Você ganhou uma chance extra!");
        screenUpdate();
        sleep(2);

        while (tentativas > 0){
            exibirGol();
            screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
            printf("o");
            screenUpdate();

            if (keyhit()) {
                tecla = readch();
                if (tecla == 'q' || tecla == 'w' || tecla == 'e'){
                    screenGotoxy(GOL_X + GOL_LARGURA / 2, POSICAO_INICIAL_BOLA_Y);
                    printf(" ");
                    screenUpdate();

                    if (animarBola(tecla, &pontos)){
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
    screenInit(1);
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

void salvarPontuacao(char *nome, int pontuacao){
    Jogador *ranking = NULL;
    int totalJogadores = 0;
    int capacidade = 10;

    ranking = (Jogador *)malloc(capacidade * sizeof(Jogador));
    if (ranking == NULL){
        perror("Erro ao alocar memória para ranking");
        return;
    }

    // Lendo o arquivo
    FILE *arquivo = fopen("ranking.txt", "r");
    if (arquivo != NULL){
        while (fscanf(arquivo, "%49s %d", ranking[totalJogadores].nome, 
                      &ranking[totalJogadores].pontuacao) == 2){
            totalJogadores++;
            // Realocar memória
            if (totalJogadores >= capacidade){
                capacidade *= 2;
                ranking = (Jogador *)realloc(ranking, capacidade * sizeof(Jogador));
                if (ranking == NULL){
                    perror("Erro ao realocar memória para ranking");
                    fclose(arquivo);
                    return;
                }
            }
        }
        fclose(arquivo);
    }

    // Adicionar novo jogador
    if (totalJogadores >= capacidade){
        capacidade++;
        ranking = (Jogador *)realloc(ranking, capacidade * sizeof(Jogador));
        if (ranking == NULL){
            perror("Erro ao realocar memória para novo jogador");
            return;
        }
    }
    strcpy(ranking[totalJogadores].nome, nome);
    ranking[totalJogadores].pontuacao = pontuacao;
    totalJogadores++;

    // Ordenando por pontuação
    for (int i = 0; i < totalJogadores - 1; i++){
        for (int j = i + 1; j < totalJogadores; j++){
            if (ranking[j].pontuacao > ranking[i].pontuacao){
                Jogador temp = ranking[i];
                ranking[i] = ranking[j];
                ranking[j] = temp;
            }
        }
    }

    // Limitar a lista até 10
    if (totalJogadores > MAX_JOGADORES){
        totalJogadores = MAX_JOGADORES;
    }

    // Salvar ranking de volta no arquivo
    arquivo = fopen("ranking.txt", "w");
    if (arquivo == NULL){
        perror("Erro ao abrir arquivo para escrita");
        free(ranking);
        return;
    }
    for (int i = 0; i < totalJogadores; i++){
        fprintf(arquivo, "%-20s %d\n", ranking[i].nome, ranking[i].pontuacao);
    }
    fclose(arquivo);

    free(ranking);
}

void exibirRanking(){
    screenSetColor(RED, WHITE);
    FILE *arquivo = fopen("ranking.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de ranking.\n");
        return;
    }

    screenClear();
    screenInit(1);
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

    int sair = -1;
    screenGotoxy(GOL_X, linha + 2);
    printf("Digite 0 para voltar ao menu:");
    screenUpdate();

    while (sair != 0){
        if (keyhit()){
            sair = readch() - '0'; // Lê o 0 digitado e converte para inteiro
        }
    }
}


void exibirMenuInicial(){
    screenClear();
    screenSetColor(YELLOW, WHITE);
    screenInit(1);
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
    modoDificil = (modo == 2);
}


int main(){
    exibirMenuInicial();
    screenClear();
    jogoPenaltis();
    screenGotoxy(GOL_X, GOL_Y + 2);
    printf("Obrigado por jogar!");
    screenUpdate();
    screenDestroy();
    return 0;
}