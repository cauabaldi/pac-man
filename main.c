/*
 * UERJ - Construcao de Algoritmos - Prof. Eugenio Silva
 * Trabalho - PAC-MAN
 *
 *  Aluno : Cauã Baldi Dios 
 *
 * Formato do mapas.txt:
 *   primeira linha: quantidade de mapas
 *   para cada mapa: "linhas colunas" e em seguida as linhas do labirinto
 *   # parede   . ponto   P jogador   G fantasma   (espaco) livre
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define MAX_L 30
#define MAX_C 60
#define MAX_FANT 10
#define ARQ_MAPAS "mapas.txt"
#define CHANCE_PERSEGUIR 75 /* % de vezes em que o fantasma persegue, da pra 
mudar se quiser mais facil ou mais dificil*/

#define COR_PADRAO 7
#define COR_PAREDE 9
#define COR_PONTO 15
#define COR_JOGADOR 14
#define COR_FANTASMA 12

typedef struct {
    int l, c;
} Posicao;

typedef struct {
    char grade[MAX_L][MAX_C + 2]; /* so paredes, pontos e espacos */
    int linhas, colunas;
    Posicao jogador;
    Posicao fantasmas[MAX_FANT];
    int nFantasmas;
    int coletados, restantes;
    int movimentos;
} Jogo;

/* ---------- utilidades de console ---------- */

void mudarCor(int cor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), cor);
}

void limparTela(void) {
    system("cls");
}

/* ---------- carregamento do mapa ---------- */

/* Le uma linha do labirinto, removendo \r\n e completando com espacos. */
void lerLinha(FILE *f, char *destino, int colunas) {
    char buffer[MAX_C + 10];
    int i, tam;

    if (fgets(buffer, sizeof(buffer), f) == NULL)
        buffer[0] = '\0';

    tam = strcspn(buffer, "\r\n");
    for (i = 0; i < colunas; i++)
        destino[i] = (i < tam) ? buffer[i] : ' ';
    destino[colunas] = '\0';
}

/* Le um mapa do arquivo. Retorna 1 se ok, 0 se houver erro. */
int lerMapa(FILE *f, Jogo *j) {
    char resto[16];
    int l;

    if (fscanf(f, "%d %d", &j->linhas, &j->colunas) != 2)
        return 0;
    if (j->linhas < 3 || j->linhas > MAX_L || j->colunas < 3 || j->colunas > MAX_C)
        return 0;
    fgets(resto, sizeof(resto), f); /* consome o fim da linha do cabecalho */

    for (l = 0; l < j->linhas; l++)
        lerLinha(f, j->grade[l], j->colunas);
    return 1;
}

/* Retira P e G da grade, guardando suas posicoes, e conta os pontos. */
int extrairPersonagens(Jogo *j) {
    int l, c, achouJogador = 0;

    j->nFantasmas = 0;
    j->coletados = 0;
    j->restantes = 0;
    j->movimentos = 0;

    for (l = 0; l < j->linhas; l++) {
        for (c = 0; c < j->colunas; c++) {
            char ch = j->grade[l][c];
            if (ch == 'P') {
                j->jogador.l = l;
                j->jogador.c = c;
                j->grade[l][c] = ' ';
                achouJogador = 1;
            } else if (ch == 'G' && j->nFantasmas < MAX_FANT) {
                j->fantasmas[j->nFantasmas].l = l;
                j->fantasmas[j->nFantasmas].c = c;
                j->nFantasmas++;
                j->grade[l][c] = '.'; /* fantasma comeca em cima de um ponto */
                j->restantes++;
            } else if (ch == '.') {
                j->restantes++;
            }
        }
    }
    return achouJogador;
}

/* Abre o arquivo, sorteia um dos mapas e o carrega. Retorna 1 se ok. */
int carregarMapa(Jogo *j, const char *arq) {
    FILE *f = fopen(arq, "r");
    int qtd, escolhido, k;

    if (f == NULL) {
        printf("Erro: nao foi possivel abrir o arquivo '%s'.\n", arq);
        return 0;
    }
    if (fscanf(f, "%d", &qtd) != 1 || qtd <= 0) {
        printf("Erro: quantidade de mapas invalida em '%s'.\n", arq);
        fclose(f);
        return 0;
    }

    escolhido = rand() % qtd;

    /* le os mapas em sequencia ate chegar ao sorteado */
    for (k = 0; k <= escolhido; k++) {
        if (!lerMapa(f, j)) {
            printf("Erro: mapa %d mal formatado em '%s'.\n", k + 1, arq);
            fclose(f);
            return 0;
        }
    }
    fclose(f);

    if (!extrairPersonagens(j)) {
        printf("Erro: o mapa %d nao tem jogador (P).\n", escolhido + 1);
        return 0;
    }
    return 1;
}

/* ---------- consultas sobre o labirinto ---------- */

int ehParede(const Jogo *j, int l, int c) {
    if (l < 0 || l >= j->linhas || c < 0 || c >= j->colunas)
        return 1;
    return j->grade[l][c] == '#';
}

/* Indica se ha um fantasma na posicao, desconsiderando o de indice 'ignorar'. */
int fantasmaEm(const Jogo *j, int l, int c, int ignorar) {
    int i;
    for (i = 0; i < j->nFantasmas; i++)
        if (i != ignorar && j->fantasmas[i].l == l && j->fantasmas[i].c == c)
            return 1;
    return 0;
}

int colidiu(const Jogo *j) {
    return fantasmaEm(j, j->jogador.l, j->jogador.c, -1);
}

int distancia(Posicao a, Posicao b) {
    return abs(a.l - b.l) + abs(a.c - b.c);
}

/* ---------- desenho ---------- */

void desenhar(const Jogo *j) {
    int l, c;

    limparTela();
    mudarCor(COR_JOGADOR);
    printf("  ===== PAC-MAN =====\n\n");

    for (l = 0; l < j->linhas; l++) {
        printf("  ");
        for (c = 0; c < j->colunas; c++) {
            if (j->jogador.l == l && j->jogador.c == c) {
                mudarCor(COR_JOGADOR);
                putchar('P');
            } else if (fantasmaEm(j, l, c, -1)) {
                mudarCor(COR_FANTASMA);
                putchar('G');
            } else if (j->grade[l][c] == '#') {
                mudarCor(COR_PAREDE);
                putchar('#');
            } else {
                mudarCor(COR_PONTO);
                putchar(j->grade[l][c]);
            }
        }
        putchar('\n');
    }

    mudarCor(COR_PADRAO);
    printf("\n  Pontos coletados: %d   |   Pontos restantes: %d\n",
           j->coletados, j->restantes);
    printf("  Movimentos: %d\n", j->movimentos);
    printf("\n  W (cima)  S (baixo)  A (esquerda)  D (direita)  Q (sair)\n");
}

/* ---------- movimentacao ---------- */

/* Move o jogador conforme a tecla. Retorna 1 se ele se moveu. */
int moverJogador(Jogo *j, char tecla) {
    int dl = 0, dc = 0, nl, nc;

    switch (tecla) {
        case 'w': case 'W': dl = -1; break;
        case 's': case 'S': dl = 1;  break;
        case 'a': case 'A': dc = -1; break;
        case 'd': case 'D': dc = 1;  break;
        default: return 0;
    }

    nl = j->jogador.l + dl;
    nc = j->jogador.c + dc;
    if (ehParede(j, nl, nc))
        return 0;

    j->jogador.l = nl;
    j->jogador.c = nc;
    j->movimentos++;

    if (j->grade[nl][nc] == '.') {
        j->grade[nl][nc] = ' ';
        j->coletados++;
        j->restantes--;
    }
    return 1;
}

/* Move o fantasma i: na maior parte das vezes vai para a casa vizinha
   mais proxima do jogador; nas demais, escolhe uma vizinha aleatoria. */
void moverFantasma(Jogo *j, int i) {
    const int dl[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    Posicao opcoes[4], melhores[4];
    int nOpcoes = 0, nMelhores = 0, menor = -1, k;

    for (k = 0; k < 4; k++) {
        int nl = j->fantasmas[i].l + dl[k];
        int nc = j->fantasmas[i].c + dc[k];
        if (!ehParede(j, nl, nc) && !fantasmaEm(j, nl, nc, i)) {
            opcoes[nOpcoes].l = nl;
            opcoes[nOpcoes].c = nc;
            nOpcoes++;
        }
    }
    if (nOpcoes == 0)
        return; /* encurralado: fica parado */

    if (rand() % 100 >= CHANCE_PERSEGUIR) {
        j->fantasmas[i] = opcoes[rand() % nOpcoes];
        return;
    }

    for (k = 0; k < nOpcoes; k++) {
        int d = distancia(opcoes[k], j->jogador);
        if (menor == -1 || d < menor) {
            menor = d;
            nMelhores = 0;
        }
        if (d == menor)
            melhores[nMelhores++] = opcoes[k];
    }
    j->fantasmas[i] = melhores[rand() % nMelhores];
}

/* ---------- fim de jogo ---------- */

void telaFinal(const Jogo *j, int venceu) {
    desenhar(j);
    printf("\n");
    if (venceu) {
        mudarCor(10);
        printf("  *** PARABENS! Voce coletou todos os pontos e venceu! ***\n");
    } else {
        mudarCor(COR_FANTASMA);
        printf("  *** GAME OVER! Um fantasma alcancou voce. ***\n");
    }
    mudarCor(COR_PADRAO);
    printf("  Pontos coletados: %d   Movimentos: %d\n", j->coletados, j->movimentos);
}

/* Executa uma partida. Retorna 1 se venceu, 0 se perdeu, -1 se saiu. */
int jogar(Jogo *j) {
    int i;
    char tecla;

    while (1) {
        desenhar(j);
        tecla = (char)_getch();

        if (tecla == 'q' || tecla == 'Q')
            return -1;
        if (!moverJogador(j, tecla))
            continue; /* tecla invalida ou parede: fantasmas nao se movem */

        if (colidiu(j))
            return 0;
        if (j->restantes == 0)
            return 1;

        for (i = 0; i < j->nFantasmas; i++) {
            moverFantasma(j, i);
            if (colidiu(j))
                return 0;
        }
    }
}

int main(void) {
    Jogo jogo;
    int resultado;
    char resp;

    srand((unsigned)time(NULL));

    do {
        if (!carregarMapa(&jogo, ARQ_MAPAS)) {
            printf("Pressione qualquer tecla para sair...");
            _getch();
            return 1;
        }

        resultado = jogar(&jogo);
        if (resultado == -1) {
            limparTela();
            printf("Jogo encerrado. Ate a proxima!\n");
            return 0;
        }
        telaFinal(&jogo, resultado);

        printf("\n  Jogar novamente? (S/N) ");
        do {
            resp = (char)_getch();
        } while (resp != 's' && resp != 'S' && resp != 'n' && resp != 'N');
        printf("%c\n", resp);
    } while (resp == 's' || resp == 'S');

    mudarCor(COR_PADRAO);
    return 0;
}
