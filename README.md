# ⚽PEnalts⚽

PEnalts é um jogo emocionante de pênaltis desenvolvido em C utilizando a biblioteca CLI-lib. Ele combina habilidade, estratégia e precisão, níveis de dificuldade e pontuação baseada em precisão. Prepare-se para desafiar o goleiro, melhorar sua pontaria e alcançar o topo do ranking global!

![2D pixel art of a player taking a penalty kick against a goalkeeper, with PEnalts written above the bar, and the player facing away from the viewers perspective](https://github.com/user-attachments/assets/2c923c0d-5940-4529-88d9-14b8b7c78c70)

---

## **📃Descrição do Jogo📃**

Em PEnalts, você assume o papel de um jogador tentando marcar gols contra um goleiro que se movimenta lateralmente.

O jogo possui dois modos: 
- **Fácil:** Um goleiro com velocidade crescente a cada chute.
- **Difícil:** Dois goleiros se movendo em direções opostas, com pontuações mais altas para chutes desafiadores.

O objetivo é acumular a maior pontuação possível em 5 tentativas e alcançar uma posição de destaque no ranking global.

---

## **🎮Controles🎮**

### Os controles são simples e divididos para as diferentes áreas do gol:

- **q**: Canto superior esquerdo  
- **w**: Meio superior  
- **e**: Canto superior direito  


Cada chute resulta em pontuações diferentes dependendo da precisão e da área escolhida.  

---

## **💻Como Jogar💻**

### **Requisitos**
1. Um ambiente que suporte programas em C.
2. A biblioteca **CLI-lib** instalada.

### **Passos para Jogar**
1. **Clone o repositório do jogo:**  
   ```bash
   git clone git clone https://github.com/IgorACNC/jogo_penalts.git
   cd penalts

2. **Compile o jogo:**
   Certifique-se de que a CLI-lib esteja configurada no ambiente.
   ```bash
   gcc ./src/*.c -I./include -o jogo-penalti

3. **Execute o jogo:**
   ```bash
   ./jogo-penalti

4. **Selecione o nível de dificuldade:**
   - Fácil: Um goleiro.
   - Difícil: Dois goleiros que se movem em direções opostas.

5. **Comece a chutar:**
   Use os controles para escolher a direção do chute. Tente prever os movimentos do goleiro e marque pontos!

6. **Confira sua posição no ranking global ao final do jogo.**

---
Aproveite a experiência e desafie seus amigos para ver quem consegue o melhor desempenho em PEnalts! 🎮⚽
