/**
    Estrutura inicial para um jogo
    versao: 0.2 (Prof. Alex, Adaptado Prof. Felski)
    Novidades: menu principal, tela "Como jogar" e escolha de dificuldade.

    MUSICA: fase_1_e_2.mp3 (fases 1 e 2) e boss.mp3 (fase do boss).
    Code::Blocks: Project > Build options > Linker settings > Add: winmm
    Som de morte: faaah.mp3.
    Deixe os .mp3 na pasta do projeto (ou junto do .exe).
*/

#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>
#include <string>
#include <cctype>

using namespace std;

const int LINHAS = 15;
const int COLUNAS = 21;
const int MAX_INIMIGOS = 14;
const int TOTAL_FASES = 3;

const int TEMPO_EXPLOSAO_MILISSEGUNDOS = 200;
const int TEMPO_MENSAGEM_MILISSEGUNDOS = 500;
const int TOTAL_TIPOS_BONUS = 7;
const int CHANCE_DROP_PORCENTO = 25;
const int TEMPO_MAXIMO_REMOTA_SEGUNDOS = 10;

int quantidadeInimigos = 8;
int alcanceExplosao = 2;
int tempoBombaSegundos = 2;
int tempoSorteioInimigoMs = 700;
int tempoPassoInimigoMs = 200;
int vidaBoss = 3;
int tempoPassoBossMs = 400;
int dificuldadeAtual = 2;

const int CAMINHO = 0;
const int PAREDE_DESTRUTIVEL = 1;
const int PAREDE_INDESTRUTIVEL = 2;
const int BOMBA = 3;
const int EXPLOSAO = 4;
const int INIMIGO = 5;
const int PORTAL = 6;
const int BOSS = 7;
const int BOSS_FERIDO = 8;

const string FUNDO_VERMELHO = "\033[41m";
const string VERMELHO = "\033[31m";
const string FUNDO_AMARELO = "\033[43m";
const string AMARELO = "\033[33m";
const string AZUL = "\033[34m";
const string FUNDO_AZUL = "\033[44m";
const string ROXO = "\033[35m";
const string FUNDO_ROXO = "\033[45m";
const string FUNDO_CINZA = "\033[100m";
const string VERDE = "\033[32m";
const string RESETAR_COR = "\033[0m";

const string SIMBOLO_ALCANCE   = "R+";
const string SIMBOLO_BOMBAS    = "@+";
const string SIMBOLO_VIDA      = "<3";
const string SIMBOLO_REMOTA    = "@R";
const string SIMBOLO_ESCUDO    = "[]";
const string SIMBOLO_PIERCE    = "<>";
const string SIMBOLO_ATRAVESSA = string(1, char(174)) + char(175);

struct RegistroRanking {
    string data;
    string jogador;
    int pontuacao = 0;
    int bombas = 0;
    int movimentos = 0;
};

const char* ARQUIVO_RANKING = "ranking.txt";

template <typename T>
void trocar(T& a, T& b) {
    T aux = a;
    a = b;
    b = aux;
}

template <typename T>
T limitar(T valor, T minimo, T maximo) {
    if(valor < minimo) return minimo;
    if(valor > maximo) return maximo;
    return valor;
}

string textoHud(bool ligado) {
    return ligado ? "SIM" : "NAO";
}

string textoHud(int valor) {
    return to_string(valor);
}

string formatarTempo(long long segundosTotais) {
    ostringstream texto;
    texto << setfill('0') << setw(2) << segundosTotais / 60 << ':'
          << setw(2) << segundosTotais % 60;
    return texto.str();
}

string pedirNome() {
    string nome;
    do {
        cout << "Digite seu nome (ate 30 caracteres, sem ponto e virgula): ";
        if(!getline(cin, nome)) return "";
        size_t inicio = nome.find_first_not_of(" \t\r");
        if(inicio == string::npos) nome.clear();
        else nome = nome.substr(inicio, nome.find_last_not_of(" \t\r") - inicio + 1);
    } while(nome.empty() || nome.size() > 30 || nome.find(';') != string::npos);
    return nome;
}

bool salvarRanking(RegistroRanking registro) {
    time_t agora = time(nullptr);
    tm* horario = localtime(&agora);
    if(horario == nullptr) return false;
    ostringstream data;
    data << put_time(horario, "%Y-%m-%d %H:%M:%S");
    registro.data = data.str();

    ifstream consulta(ARQUIVO_RANKING, ios::binary | ios::ate);
    bool vazio = !consulta || consulta.tellg() == streampos(0);
    consulta.close();
    ofstream arquivo(ARQUIVO_RANKING, ios::app);
    if(!arquivo) return false;
    if(vazio) arquivo << "Data;Jogador;Pontuacao;Bombas;Movimentos\n";
    arquivo << registro.data << ';' << registro.jogador << ';'
            << registro.pontuacao << ';' << registro.bombas << ';'
            << registro.movimentos << '\n';
    arquivo.close();
    return !arquivo.fail();
}

void telaRanking() {
    system("cls");
    cout << "================ RANKING ================\n\n";
    ifstream arquivo(ARQUIVO_RANKING);
    vector<RegistroRanking> registros;
    string linha;
    getline(arquivo, linha);
    while(getline(arquivo, linha)) {
        istringstream campos(linha);
        RegistroRanking registro;
        string pontos, bombas, movimentos;
        if(!getline(campos, registro.data, ';') ||
           !getline(campos, registro.jogador, ';') ||
           !getline(campos, pontos, ';') ||
           !getline(campos, bombas, ';') ||
           !getline(campos, movimentos)) continue;
        istringstream numeros(pontos + " " + bombas + " " + movimentos);
        if(!(numeros >> registro.pontuacao >> registro.bombas >> registro.movimentos)) continue;
        registros.push_back(registro);
    }
    int total = static_cast<int>(registros.size());
    for(int i = 0; i < total - 1; i++) {
        for(int j = 0; j < total - 1 - i; j++) {
            if(registros[j].pontuacao < registros[j + 1].pontuacao) {
                trocar(registros[j], registros[j + 1]);
            }
        }
    }
    if(registros.empty()) cout << "Nenhuma partida registrada.\n";
    else {
        cout << left << setw(21) << "Data" << setw(32) << "Jogador"
             << setw(12) << "Pontuacao" << setw(9) << "Bombas" << "Movimentos\n";
        for(const RegistroRanking& r : registros) {
            cout << left << setw(21) << r.data << setw(32) << r.jogador
                 << setw(12) << r.pontuacao << setw(9) << r.bombas << r.movimentos << '\n';
        }
    }
    cout << "\nPressione qualquer tecla para voltar...";
    _getch();
}

struct Inimigo {
    int x;
    int y;
    bool vivo;
    int direcao;
    int passosRestantes;
    chrono::steady_clock::time_point ultimoPasso;
};

void criarMapa(int m[LINHAS][COLUNAS]) {
    unsigned int semente = static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count());
    mt19937 gerador(semente);
    bernoulli_distribution criarCaminho(0.8);

    for(int i = 0; i < LINHAS; i++) {
        for(int j = 0; j < COLUNAS; j++) {
            bool borda = i == 0 || i == LINHAS - 1 ||
                         j == 0 || j == COLUNAS - 1;
            bool paredeFixa = i % 2 == 0 && j % 2 == 0;

            if(borda || paredeFixa) {
                m[i][j] = PAREDE_INDESTRUTIVEL;
            }
            else if(criarCaminho(gerador)) {
                m[i][j] = CAMINHO;
            }
            else {
                m[i][j] = PAREDE_DESTRUTIVEL;
            }
        }
    }
}

int aleatorio(int minimo, int maximo) {
    static mt19937 gerador(static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));
    uniform_int_distribution<int> distribuicao(minimo, maximo);
    return distribuicao(gerador);
}

int aleatorio(int maximo) {
    return aleatorio(0, maximo);
}

void verificarInimigosAtingidos(int m[LINHAS][COLUNAS], Inimigo inimigos[MAX_INIMIGOS]) {
    for(int i = 0; i < quantidadeInimigos; i++) {
        if(inimigos[i].vivo && m[inimigos[i].x][inimigos[i].y] == EXPLOSAO) {
            inimigos[i].vivo = false;
            inimigos[i].passosRestantes = 0;
        }
    }
}

void movimentoInimigo(int m[LINHAS][COLUNAS], Inimigo inimigos[MAX_INIMIGOS], bool sortearNovoMovimento, chrono::steady_clock::time_point agora) {
    int mudarX[4] = {1, -1, 0, 0};
    int mudarY[4] = {0, 0, 1, -1};

    for(int i = 0; i < quantidadeInimigos; i++) {
        Inimigo &inimigo = inimigos[i];

        if(!inimigo.vivo) {
            continue;
        }

        if(inimigo.passosRestantes == 0) {
            if(!sortearNovoMovimento) {
                continue;
            }

            int livres[4];
            int quantidade = 0;

            for(int direcao = 0; direcao < 4; direcao++) {
                int x = inimigo.x + mudarX[direcao];
                int y = inimigo.y + mudarY[direcao];

                if(m[x][y] == CAMINHO || m[x][y] == EXPLOSAO) {
                    livres[quantidade] = direcao;
                    quantidade++;
                }
            }

            if(quantidade == 0) {
                continue;
            }

            inimigo.direcao = livres[aleatorio(quantidade - 1)];
            inimigo.passosRestantes = aleatorio(1, 3);
        }
        else {
            auto tempo = chrono::duration_cast<chrono::milliseconds>(agora - inimigo.ultimoPasso);

            if(tempo.count() < tempoPassoInimigoMs) {
                continue;
            }
        }

        int x = inimigo.x + mudarX[inimigo.direcao];
        int y = inimigo.y + mudarY[inimigo.direcao];

        if(m[x][y] != CAMINHO && m[x][y] != EXPLOSAO) {
            inimigo.passosRestantes = 0;
            continue;
        }

        m[inimigo.x][inimigo.y] = CAMINHO;
        inimigo.x = x;
        inimigo.y = y;

        if(m[x][y] == EXPLOSAO) {
            inimigo.vivo = false;
            inimigo.passosRestantes = 0;
        }
        else {
            m[x][y] = INIMIGO;
            inimigo.passosRestantes--;
            inimigo.ultimoPasso = agora;
        }
    }
}

bool todosInimigosMortos(Inimigo inimigos[MAX_INIMIGOS]) {
    for(int i = 0; i < quantidadeInimigos; i++) {
        if(inimigos[i].vivo) {
            return false;
        }
    }
    return true;
}

void criarPortal(int m[LINHAS][COLUNAS], int jogadorX, int jogadorY) {
    bool visitado[LINHAS][COLUNAS] = {};
    int filaX[LINHAS * COLUNAS], filaY[LINHAS * COLUNAS];
    int inicio = 0, fim = 1;
    filaX[0] = jogadorX;
    filaY[0] = jogadorY;
    visitado[jogadorX][jogadorY] = true;
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};
    int portalX = jogadorX, portalY = jogadorY;
    while(inicio < fim) {
        int x = filaX[inicio], y = filaY[inicio];
        inicio++;
        portalX = x;
        portalY = y;
        for(int i = 0; i < 4; i++) {
            int nx = x + dx[i], ny = y + dy[i];
            if(nx >= 0 && nx < LINHAS && ny >= 0 && ny < COLUNAS &&
               !visitado[nx][ny] && m[nx][ny] == CAMINHO) {
                visitado[nx][ny] = true;
                filaX[fim] = nx;
                filaY[fim] = ny;
                fim++;
            }
        }
    }
    m[portalX][portalY] = PORTAL;
}

void imprimirJogador(const string& nome, int casa) {
    if(casa == INIMIGO || casa == BOSS || casa == BOSS_FERIDO) {
        cout << FUNDO_AZUL << nome << RESETAR_COR;
    }
    else if(casa == BOMBA) {
        cout << FUNDO_VERMELHO << nome << RESETAR_COR;
    }
    else if(casa == EXPLOSAO) {
        cout << FUNDO_AMARELO << nome << RESETAR_COR;
    }
    else if(casa == PAREDE_DESTRUTIVEL) {
        cout << FUNDO_CINZA << nome << RESETAR_COR;
    }
    else {
        cout << nome;
    }
}

void imprimirJogo(int m[LINHAS][COLUNAS], int jogadorX, int jogadorY, COORD coord, int jogador2X = -1, int jogador2Y = -1) {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);

    for(int i = 0; i < LINHAS; i++) {
        for(int j = 0; j < COLUNAS; j++) {
            if(i == jogador2X && j == jogador2Y) {
                imprimirJogador("P2", m[i][j]);
            }
            else if(i == jogadorX && j == jogadorY) {
                imprimirJogador("P1", m[i][j]);
            }
            else {
                switch(m[i][j]) {
                    case 11: cout << AMARELO << SIMBOLO_ALCANCE   << RESETAR_COR; break;
                    case 12: cout << AMARELO << SIMBOLO_BOMBAS    << RESETAR_COR; break;
                    case 13: cout << AMARELO << SIMBOLO_VIDA      << RESETAR_COR; break;
                    case 14: cout << AMARELO << SIMBOLO_REMOTA    << RESETAR_COR; break;
                    case 15: cout << AMARELO << SIMBOLO_ESCUDO    << RESETAR_COR; break;
                    case 16: cout << AMARELO << SIMBOLO_PIERCE    << RESETAR_COR; break;
                    case 17: cout << AMARELO << SIMBOLO_ATRAVESSA << RESETAR_COR; break;
                    case CAMINHO:
                        cout << "  ";
                        break;
                    case PAREDE_DESTRUTIVEL:
                        cout << char(176)<< char(176);
                        break;
                    case PAREDE_INDESTRUTIVEL:
                        cout << char(219)<< char(219);
                        break;
                    case BOMBA:
                        cout << VERMELHO << char(219)<< char(219) << RESETAR_COR;
                        break;
                    case EXPLOSAO:
                        cout << FUNDO_VERMELHO << AMARELO << char(176)<< char(176) << RESETAR_COR;
                        break;
                    case PORTAL:
                        cout << ROXO << "[]" << RESETAR_COR;
                        break;
                    case BOSS:
                        cout << FUNDO_ROXO << AMARELO << "BS" << RESETAR_COR;
                        break;
                    case BOSS_FERIDO:
                        cout << FUNDO_AMARELO << ROXO << "BS" << RESETAR_COR;
                        break;
                    case INIMIGO:
                        cout << AZUL << char(219)<< char(219) << RESETAR_COR;
                        break;
                }
            }
        }
        cout << "\n";
    }
    cout.flush();
}

void aplicarDificuldade(int nivel) {
    dificuldadeAtual = nivel;

    if(nivel == 1) {
        quantidadeInimigos = 4;
        alcanceExplosao = 3;
        tempoBombaSegundos = 2;
        tempoSorteioInimigoMs = 1000;
        tempoPassoInimigoMs = 350;
        vidaBoss = 2;
        tempoPassoBossMs = 500;
    }
    else if(nivel == 2) {
        quantidadeInimigos = 8;
        alcanceExplosao = 2;
        tempoBombaSegundos = 2;
        tempoSorteioInimigoMs = 700;
        tempoPassoInimigoMs = 200;
        vidaBoss = 3;
        tempoPassoBossMs = 400;
    }
    else {
        quantidadeInimigos = 12;
        alcanceExplosao = 2;
        tempoBombaSegundos = 3;
        tempoSorteioInimigoMs = 450;
        tempoPassoInimigoMs = 120;
        vidaBoss = 5;
        tempoPassoBossMs = 300;
    }
}

void aplicarFase(int fase) {
    aplicarDificuldade(dificuldadeAtual);
    quantidadeInimigos = limitar(quantidadeInimigos + (fase - 1), 0, MAX_INIMIGOS);
}

string nomeDificuldade() {
    if(dificuldadeAtual == 1) return "Facil";
    if(dificuldadeAtual == 2) return "Medio";
    return "Dificil";
}

void imprimirLegenda() {
    cout << "  ITENS DO JOGO\n";
    cout << "  P1  Jogador 1 (e o computador, no modo automatico)\n";
    cout << "  P2  Jogador 2 (modo cooperativo)\n";
    cout << "  " << AZUL << char(219) << char(219) << RESETAR_COR << "  Inimigo: se encostar em voce, voce perde\n";
    cout << "  " << FUNDO_ROXO << AMARELO << "BS" << RESETAR_COR << "  Boss (ultima fase): segue voce e aguenta varias explosoes\n";
    cout << "  " << VERMELHO << char(219) << char(219) << RESETAR_COR << "  Bomba: explode depois de alguns segundos\n";
    cout << "  " << FUNDO_VERMELHO << AMARELO << char(176) << char(176) << RESETAR_COR << "  Explosao: mata inimigos e quebra paredes (e mata voce tambem!)\n";
    cout << "  " << char(176) << char(176) << "  Parede destrutivel: a explosao quebra e para nela\n";
    cout << "  " << char(219) << char(219) << "  Parede indestrutivel: nada atravessa\n";
    cout << "  " << ROXO << "[]" << RESETAR_COR << "  Portal: aparece quando todos os inimigos morrem\n\n";
    cout << "  BONUS (aparecem embaixo das paredes quebradas, " << CHANCE_DROP_PORCENTO << "% de chance)\n";
    cout << "  " << AMARELO << SIMBOLO_ALCANCE   << RESETAR_COR << "  Alcance da explosao +1 (cumulativo)\n";
    cout << "  " << AMARELO << SIMBOLO_BOMBAS    << RESETAR_COR << "  Uma bomba a mais ao mesmo tempo (cumulativo)\n";
    cout << "  " << AMARELO << SIMBOLO_VIDA      << RESETAR_COR << "  Vida extra (cumulativo)\n";
    cout << "  " << AMARELO << SIMBOLO_REMOTA    << RESETAR_COR << "  Bomba remota: voce escolhe quando explodir\n";
    cout << "  " << AMARELO << SIMBOLO_ESCUDO    << RESETAR_COR << "  Escudo: a explosao nao te mata\n";
    cout << "  " << AMARELO << SIMBOLO_PIERCE    << RESETAR_COR << "  Pierce: a explosao atravessa paredes destrutiveis\n";
    cout << "  " << AMARELO << SIMBOLO_ATRAVESSA << RESETAR_COR << "  Atravessa: voce anda por dentro das paredes destrutiveis\n";
    cout << "  Os bonus resetam ao passar de fase.\n";
}

void telaComoJogar() {
    system("cls");
    cout << "=========== COMO JOGAR ===========\n\n";
    cout << "  OBJETIVO\n";
    cout << "  Elimine todos os inimigos usando bombas.\n";
    cout << "  Voce perde se for atingido pela explosao ou tocar em um inimigo.\n\n";
    cout << "  CONTROLES\n";
    cout << "  W / Seta cima ....... mover para cima\n";
    cout << "  S / Seta baixo ...... mover para baixo\n";
    cout << "  A / Seta esquerda ... mover para a esquerda\n";
    cout << "  D / Seta direita .... mover para a direita\n";
    cout << "  Jogador 1: WASD + ESPACO para colocar bomba.\n";
    cout << "  Jogador 2 (P2): setas + ENTER para colocar bomba.\n";
    cout << "  Uma bomba por jogador inicialmente; " << SIMBOLO_BOMBAS << " aumenta o limite.\n";
    cout << "  Cada parede destruida tem " << CHANCE_DROP_PORCENTO << "% de chance de dropar um bonus.\n";
    cout << "  " << SIMBOLO_ALCANCE << ": alcance +1; " << SIMBOLO_BOMBAS << ": bomba +1; "
         << SIMBOLO_VIDA << ": vida +1 (cumulativos).\n";
    cout << "  " << SIMBOLO_REMOTA << ": bombas remotas; detonar com E (P1) / Ctrl direito (P2).\n";
    cout << "      Se nao detonar, a bomba explode sozinha apos " << TEMPO_MAXIMO_REMOTA_SEGUNDOS << "s.\n";
    cout << "  " << SIMBOLO_ESCUDO << ": imunidade a explosoes.\n";
    cout << "  " << SIMBOLO_PIERCE << ": a explosao atravessa paredes destrutiveis (ate o alcance).\n";
    cout << "  " << SIMBOLO_ATRAVESSA << ": voce anda por dentro das paredes destrutiveis.\n";
    cout << "  Bonus individuais resetam a cada fase.\n";
    cout << "  No modo solo, WASD e setas controlam o mesmo jogador.\n";
    cout << "  Modo automatico: escolha 7 no menu e o computador joga pelo P1.\n";
    cout << "  Cooperativo: se um morrer, a dupla perde.\n";
    cout << "  Qualquer jogador pode entrar no portal para avancar.\n\n";
    cout << "  DICA\n";
    cout << "  Coloque a bomba e corra! A explosao vai em cruz (cima, baixo,\n";
    cout << "  esquerda, direita) e para ao bater em qualquer parede.\n\n";
    cout << "  Ao eliminar todos os inimigos, entre no portal [] para avancar.\n\n";
    imprimirLegenda();
    cout << "\n  Pressione qualquer tecla para voltar...";
    _getch();
}

void telaPontuacao() {
    system("cls");
    cout << "============== PONTUACAO ==============\n\n";
    cout << "  A pontuacao comeca em 0 e vale para a partida inteira\n";
    cout << "  (as 3 fases somadas). No modo cooperativo, e da dupla.\n\n";
    cout << "  GANHA PONTOS\n";
    cout << "  +100 ... por inimigo eliminado\n";
    cout << "  +5 ..... por parede destrutivel destruida\n";
    cout << "  +500 ... por boss derrotado (ultima fase)\n\n";
    cout << "  PERDE PONTOS\n";
    cout << "  -1 ..... por movimento (cada casa andada)\n";
    cout << "  -5 ..... por bomba colocada\n\n";
    cout << "  EXEMPLO\n";
    cout << "  4 inimigos, 10 paredes, 80 movimentos e 12 bombas:\n";
    cout << "  4*100 + 10*5 - 80*1 - 12*5 = 310 pontos\n\n";
    cout << "  DICAS\n";
    cout << "  - Cada bomba custa 5 pontos: quebre varias paredes com a mesma bomba.\n";
    cout << "  - Andar sem necessidade tambem tira pontos.\n";
    cout << "  - A pontuacao pode ficar negativa.\n\n";
    cout << "  RANKING\n";
    cout << "  Ao final de cada partida sao salvos data, jogador, pontuacao,\n";
    cout << "  bombas e movimentos. O ranking e ordenado da maior pontuacao\n";
    cout << "  para a menor; em caso de empate, vale a ordem em que foram jogadas.\n";
    cout << "\n  Pressione qualquer tecla para voltar...";
    _getch();
}

void telaDificuldade() {
    while(true) {
        system("cls");
        cout << "=========== DIFICULDADE ===========\n\n";
        cout << "  Atual: " << nomeDificuldade() << "\n\n";
        cout << "  1 - Facil\n";
        cout << "      4 inimigos, lentos. Explosao grande (alcance 3).\n\n";
        cout << "  2 - Medio\n";
        cout << "      8 inimigos. Explosao media (alcance 2).\n\n";
        cout << "  3 - Dificil\n";
        cout << "      12 inimigos, rapidos. Bomba demora mais (3s) para explodir.\n\n";
        cout << "  Itens que mudam com a dificuldade:\n";
        cout << "   - Quantidade de inimigos\n";
        cout << "   - Velocidade dos inimigos\n";
        cout << "   - Alcance da explosao da bomba\n";
        cout << "   - Tempo da bomba ate explodir\n\n";
        imprimirLegenda();
        cout << "\n";
        cout << "  0 - Voltar\n\n";
        cout << "  Escolha: ";

        int tecla = _getch();

        if(tecla == '1' || tecla == '2' || tecla == '3') {
            aplicarDificuldade(tecla - '0');
        }
        else if(tecla == '0') {
            return;
        }
    }
}

enum TipoBonus { SEM_BONUS, MAIS_ALCANCE, MAIS_BOMBAS, VIDA_EXTRA,
                 RELOGIO, ESCUDO, PIERCE, ATRAVESSA };
struct Jogador {
    int x = 1, y = 1;
    int alcance = 2, capacidade = 1, vidas = 1;
    bool relogio = false, escudo = false, pierce = false;
    bool atravessa = false;
    bool colocarAnterior = false, detonarAnterior = false;
    chrono::steady_clock::time_point ultimoPasso, invulneravelAte;
};
struct BombaJogador {
    int x, y, dono, alcance;
    bool remota, pierce, ativa;
    chrono::steady_clock::time_point inicio;
};
struct EfeitoCelula {
    bool ativo = false;
    TipoBonus drop = SEM_BONUS;
    chrono::steady_clock::time_point fim;
};
void aplicarBonus(Jogador& j, TipoBonus bonus) {
    switch(bonus) {
        case MAIS_ALCANCE: j.alcance++; break;
        case MAIS_BOMBAS: j.capacidade++; break;
        case VIDA_EXTRA: j.vidas++; break;
        case RELOGIO: j.relogio = true; break;
        case ESCUDO: j.escudo = true; break;
        case PIERCE: j.pierce = true; break;
        case ATRAVESSA: j.atravessa = true; break;
        default: break;
    }
}
TipoBonus sortearBonus() {
    return aleatorio(1, 100) <= CHANCE_DROP_PORCENTO
        ? static_cast<TipoBonus>(aleatorio(1, TOTAL_TIPOS_BONUS)) : SEM_BONUS;
}
bool receberDano(Jogador& j, bool explosao, bool inimigo,
                 chrono::steady_clock::time_point agora) {
    if((!inimigo && (!explosao || j.escudo)) || agora < j.invulneravelAte)
        return true;
    j.vidas--;
    j.invulneravelAte = agora + chrono::milliseconds(1500);
    return j.vidas > 0;
}
void detonarBomba(int indice, int m[LINHAS][COLUNAS],
                  vector<BombaJogador>& bombas,
                  EfeitoCelula efeitos[LINHAS][COLUNAS],
                  RegistroRanking& registro,
                  chrono::steady_clock::time_point agora) {
    if(!bombas[indice].ativa) return;
    BombaJogador bomba = bombas[indice];
    bombas[indice].ativa = false;
    auto atingir = [&](int x, int y) {
        bool parede = m[x][y] == PAREDE_DESTRUTIVEL;
        if(parede) {
            registro.pontuacao += 5;
            efeitos[x][y].drop = sortearBonus();
        }
        efeitos[x][y].ativo = true;
        efeitos[x][y].fim = agora + chrono::milliseconds(TEMPO_EXPLOSAO_MILISSEGUNDOS);
        m[x][y] = EXPLOSAO;
    };
    atingir(bomba.x, bomba.y);
    int dx[4] = {1,-1,0,0}, dy[4] = {0,0,1,-1};
    for(int d = 0; d < 4; d++) {
        for(int passo = 1; passo <= bomba.alcance; passo++) {
            int x = bomba.x + dx[d]*passo, y = bomba.y + dy[d]*passo;
            if(x < 0 || x >= LINHAS || y < 0 || y >= COLUNAS ||
               m[x][y] == PAREDE_INDESTRUTIVEL) break;
            bool parede = m[x][y] == PAREDE_DESTRUTIVEL;
            atingir(x,y);
            if(parede && !bomba.pierce) break;
        }
    }
    for(int i = 0; i < static_cast<int>(bombas.size()); i++)
        if(bombas[i].ativa && efeitos[bombas[i].x][bombas[i].y].ativo)
            detonarBomba(i, m, bombas, efeitos, registro, agora);
}

struct Chefe {
    bool ativo = false;
    int x = 0, y = 0;
    int vida = 0;
    chrono::steady_clock::time_point ultimoPasso, invulneravelAte;
};

bool chefeVivo(const Chefe& chefe) {
    return chefe.ativo && chefe.vida > 0;
}

bool ehJogador(const Jogador jogadores[], int numeroJogadores, int x, int y) {
    for(int i = 0; i < numeroJogadores; i++)
        if(jogadores[i].x == x && jogadores[i].y == y) return true;
    return false;
}

bool proximoPassoChefe(int m[LINHAS][COLUNAS], const Chefe& chefe,
                       const Jogador jogadores[], int numeroJogadores,
                       int& passoX, int& passoY) {
    bool visitado[LINHAS][COLUNAS] = {};
    int casaAnteriorLinha[LINHAS][COLUNAS], casaAnteriorColuna[LINHAS][COLUNAS];
    int filaLinhas[LINHAS * COLUNAS], filaColunas[LINHAS * COLUNAS];
    int inicio = 0, fim = 1;
    filaLinhas[0] = chefe.x;
    filaColunas[0] = chefe.y;
    visitado[chefe.x][chefe.y] = true;
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};
    while(inicio < fim) {
        int x = filaLinhas[inicio], y = filaColunas[inicio];
        inicio++;
        if(ehJogador(jogadores, numeroJogadores, x, y)) {
            if(x == chefe.x && y == chefe.y) return false;
            while(casaAnteriorLinha[x][y] != chefe.x || casaAnteriorColuna[x][y] != chefe.y) {
                int px = casaAnteriorLinha[x][y], py = casaAnteriorColuna[x][y];
                x = px;
                y = py;
            }
            passoX = x;
            passoY = y;
            return true;
        }
        for(int d = 0; d < 4; d++) {
            int nx = x + dx[d], ny = y + dy[d];
            if(nx < 0 || nx >= LINHAS || ny < 0 || ny >= COLUNAS || visitado[nx][ny]) continue;
            if(m[nx][ny] != CAMINHO && !ehJogador(jogadores, numeroJogadores, nx, ny)) continue;
            visitado[nx][ny] = true;
            casaAnteriorLinha[nx][ny] = x;
            casaAnteriorColuna[nx][ny] = y;
            filaLinhas[fim] = nx;
            filaColunas[fim] = ny;
            fim++;
        }
    }
    return false;
}

void chefeAtingidoPelaExplosao(Chefe& chefe, bool explosaoNaCasa,
                               RegistroRanking& registro,
                               chrono::steady_clock::time_point agora) {
    if(!chefeVivo(chefe) || !explosaoNaCasa || agora < chefe.invulneravelAte) return;
    chefe.vida--;
    chefe.invulneravelAte = agora + chrono::milliseconds(2000);
    if(chefe.vida <= 0) registro.pontuacao += 500;
}

void movimentoChefe(int m[LINHAS][COLUNAS], Chefe& chefe,
                    const Jogador jogadores[], int numeroJogadores,
                    chrono::steady_clock::time_point agora) {
    if(!chefeVivo(chefe)) {
        if(chefe.ativo && m[chefe.x][chefe.y] == BOSS) m[chefe.x][chefe.y] = CAMINHO;
        return;
    }
    if(m[chefe.x][chefe.y] == CAMINHO) m[chefe.x][chefe.y] = BOSS;

    auto passo = chrono::duration_cast<chrono::milliseconds>(agora - chefe.ultimoPasso);
    if(passo.count() < tempoPassoBossMs) return;

    int nx, ny;
    if(!proximoPassoChefe(m, chefe, jogadores, numeroJogadores, nx, ny)) return;
    if(m[nx][ny] != CAMINHO) return;
    if(m[chefe.x][chefe.y] == BOSS) m[chefe.x][chefe.y] = CAMINHO;
    chefe.x = nx;
    chefe.y = ny;
    m[nx][ny] = BOSS;
    chefe.ultimoPasso = agora;
}

string caminhoMusica(const string& nome) {
    if(GetFileAttributesA(nome.c_str()) != INVALID_FILE_ATTRIBUTES) return nome;
    char exe[MAX_PATH];
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    string pasta = exe;
    size_t barra = pasta.find_last_of("\\/");
    if(barra != string::npos) pasta = pasta.substr(0, barra + 1);
    return pasta + nome;
}

void tocarMusica(const string& arquivo) {
    mciSendStringA("close musica", NULL, 0, NULL);
    string comando = "open \"" + caminhoMusica(arquivo) + "\" type mpegvideo alias musica";
    MCIERROR erro = mciSendStringA(comando.c_str(), NULL, 0, NULL);
    if(erro == 0) erro = mciSendStringA("play musica repeat", NULL, 0, NULL);
    if(erro != 0) {
        char texto[256];
        mciGetErrorStringA(erro, texto, sizeof(texto));
        cout << "\n[Musica: " << texto << " - " << arquivo << "]\n";
        Sleep(1500);
        system("cls");
    }
}

void tocarEfeito(const string& arquivo) {
    mciSendStringA("close efeito", NULL, 0, NULL);
    string comando = "open \"" + caminhoMusica(arquivo) + "\" type mpegvideo alias efeito";
    if(mciSendStringA(comando.c_str(), NULL, 0, NULL) == 0)
        mciSendStringA("play efeito", NULL, 0, NULL);
}

void pararMusica() {
    mciSendStringA("stop musica", NULL, 0, NULL);
    mciSendStringA("close musica", NULL, 0, NULL);
}

bool casaSeguraParaBot(int m[LINHAS][COLUNAS],
                       const Jogador& j,
                       const vector<BombaJogador>& bombas,
                       EfeitoCelula efeitos[LINHAS][COLUNAS],
                       int x, int y) {
    if(x < 0 || x >= LINHAS || y < 0 || y >= COLUNAS) return false;
    if(m[x][y] != CAMINHO && m[x][y] != PORTAL && m[x][y] != EXPLOSAO) return false;
    if(efeitos[x][y].ativo) return false;

    for(const BombaJogador& bomba : bombas)
        if(bomba.ativa && bomba.x == x && bomba.y == y)
            return false;

    return true;
}

bool botTemPerigoPerto(int m[LINHAS][COLUNAS],
                       const Jogador& j,
                       const vector<BombaJogador>& bombas,
                       EfeitoCelula efeitos[LINHAS][COLUNAS]) {
    if(efeitos[j.x][j.y].ativo) return true;

    for(const BombaJogador& bomba : bombas) {
        if(!bomba.ativa) continue;

        if(bomba.x == j.x) {
            int inicio = min(bomba.y, j.y) + 1;
            int fim = max(bomba.y, j.y);
            bool bloqueada = false;
            for(int y = inicio; y < fim; y++) {
                if(m[j.x][y] == PAREDE_INDESTRUTIVEL ||
                   m[j.x][y] == PAREDE_DESTRUTIVEL) {
                    bloqueada = true;
                    break;
                }
            }
            if(!bloqueada && abs(bomba.y - j.y) <= bomba.alcance)
                return true;
        }

        if(bomba.y == j.y) {
            int inicio = min(bomba.x, j.x) + 1;
            int fim = max(bomba.x, j.x);
            bool bloqueada = false;
            for(int x = inicio; x < fim; x++) {
                if(m[x][j.y] == PAREDE_INDESTRUTIVEL ||
                   m[x][j.y] == PAREDE_DESTRUTIVEL) {
                    bloqueada = true;
                    break;
                }
            }
            if(!bloqueada && abs(bomba.x - j.x) <= bomba.alcance)
                return true;
        }
    }

    return false;
}

void movimentoBot(int m[LINHAS][COLUNAS],
                  Jogador& j,
                  vector<BombaJogador>& bombas,
                  EfeitoCelula efeitos[LINHAS][COLUNAS],
                  Inimigo inimigos[MAX_INIMIGOS],
                  const Chefe& chefe,
                  bool& deveColocarBomba,
                  chrono::steady_clock::time_point agora) {
    deveColocarBomba = false;

    if(chrono::duration_cast<chrono::milliseconds>(agora - j.ultimoPasso).count() < 150)
        return;

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    if(botTemPerigoPerto(m, j, bombas, efeitos)) {
        int melhorX = j.x, melhorY = j.y;
        int melhorSeguranca = -1;

        for(int d = 0; d < 4; d++) {
            int nx = j.x + dx[d], ny = j.y + dy[d];
            if(!casaSeguraParaBot(m, j, bombas, efeitos, nx, ny)) continue;

            int seguranca = 0;

            for(const BombaJogador& bomba : bombas) {
                if(!bomba.ativa) continue;
                seguranca += abs(nx - bomba.x) + abs(ny - bomba.y);
            }

            if(seguranca > melhorSeguranca) {
                melhorSeguranca = seguranca;
                melhorX = nx;
                melhorY = ny;
            }
        }

        if(melhorX != j.x || melhorY != j.y) {
            j.x = melhorX;
            j.y = melhorY;
            j.ultimoPasso = agora;
            return;
        }
    }

    bool temParede = false;
    for(int d = 0; d < 4; d++) {
        int nx = j.x + dx[d], ny = j.y + dy[d];
        if(nx >= 0 && nx < LINHAS && ny >= 0 && ny < COLUNAS &&
           m[nx][ny] == PAREDE_DESTRUTIVEL) {
            temParede = true;
            break;
        }
    }

    int bombasAtivas = 0;
    for(const BombaJogador& bomba : bombas)
        if(bomba.ativa && bomba.dono == 0)
            bombasAtivas++;

    if(temParede && bombasAtivas < j.capacidade &&
       m[j.x][j.y] == CAMINHO && !efeitos[j.x][j.y].ativo) {
        deveColocarBomba = true;
        return;
    }

    int melhorX = j.x, melhorY = j.y;
    int melhorValor = -1000000;

    for(int d = 0; d < 4; d++) {
        int nx = j.x + dx[d], ny = j.y + dy[d];

        if(!casaSeguraParaBot(m, j, bombas, efeitos, nx, ny))
            continue;

        int valor = aleatorio(0, 10);

        for(int d2 = 0; d2 < 4; d2++) {
            int ax = nx + dx[d2], ay = ny + dy[d2];
            if(ax >= 0 && ax < LINHAS && ay >= 0 && ay < COLUNAS &&
               m[ax][ay] == PAREDE_DESTRUTIVEL)
                valor += 30;
        }

        for(int i = 0; i < quantidadeInimigos; i++) {
            if(!inimigos[i].vivo) continue;
            int distancia = abs(nx - inimigos[i].x) + abs(ny - inimigos[i].y);
            if(distancia <= 2) valor -= 20;
        }

        if(chefeVivo(chefe)) {
            int distancia = abs(nx - chefe.x) + abs(ny - chefe.y);
            if(distancia <= 2) valor -= 30;
        }

        if(valor > melhorValor) {
            melhorValor = valor;
            melhorX = nx;
            melhorY = ny;
        }
    }

    if(melhorX != j.x || melhorY != j.y) {
        j.x = melhorX;
        j.y = melhorY;
        j.ultimoPasso = agora;
    }
}

bool jogar(int fase, RegistroRanking& registro, int numeroJogadores, bool modoAutomatico = false) {
    aplicarFase(fase);
    int m[LINHAS][COLUNAS];
    Jogador jogadores[2];
    EfeitoCelula efeitos[LINHAS][COLUNAS];
    TipoBonus itens[LINHAS][COLUNAS] = {};
    vector<BombaJogador> bombas;
    Inimigo inimigos[MAX_INIMIGOS] = {};
    auto agora = chrono::steady_clock::now();
    auto inicioMovimentoInimigo = agora;
    const auto inicioFase = agora;
    for(int i = 0; i < numeroJogadores; i++) {
        jogadores[i].alcance = alcanceExplosao;
        jogadores[i].ultimoPasso = agora - chrono::milliseconds(150);
    }
    jogadores[1].y = COLUNAS - 2;
    system("cls");
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO     cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);
    short int CX=0, CY=0;
    COORD coord;
    coord.X = CX;
    coord.Y = CY;

    criarMapa(m);
    for(int i = 0; i < numeroJogadores; i++) {
        int y = jogadores[i].y;
        m[1][y] = m[2][y] = CAMINHO;
        m[1][i == 0 ? y+1 : y-1] = CAMINHO;
    }
    vector<pair<int,int> > livres;
    for(int x = 6; x < LINHAS-1; x++)
        for(int y = 6; y < COLUNAS-1; y++)
            if(m[x][y] == CAMINHO) livres.push_back(make_pair(x,y));
    bool temBoss = fase == TOTAL_FASES;
    tocarMusica(temBoss ? "boss.mp3" : "fase_1_e_2.mp3");
    if(static_cast<int>(livres.size()) < quantidadeInimigos + (temBoss ? 1 : 0)) return false;
    for(int i = 0; i < quantidadeInimigos; i++) {
        int escolhido = aleatorio(static_cast<int>(livres.size())-1);
        inimigos[i].x = livres[escolhido].first;
        inimigos[i].y = livres[escolhido].second;
        inimigos[i].vivo = true;
        m[inimigos[i].x][inimigos[i].y] = INIMIGO;
        livres.erase(livres.begin()+escolhido);
    }
    Chefe chefe;
    if(temBoss) {
        int melhor = 0;
        for(int i = 1; i < static_cast<int>(livres.size()); i++)
            if(livres[i].first + livres[i].second > livres[melhor].first + livres[melhor].second)
                melhor = i;
        chefe.ativo = true;
        chefe.vida = vidaBoss;
        chefe.x = livres[melhor].first;
        chefe.y = livres[melhor].second;
        chefe.ultimoPasso = agora;
        chefe.invulneravelAte = agora;
        m[chefe.x][chefe.y] = BOSS;
    }
    auto tocaInimigo = [&](int x, int y) {
        return m[x][y] == INIMIGO ||
               (chefe.ativo && chefe.vida > 0 && chefe.x == x && chefe.y == y);
    };
    bool portalAtivo = false;
    int contabilizados = 0;
    auto pressionada = [](int tecla) { return (GetAsyncKeyState(tecla)&0x8000)!=0; };
    const int teclas[2][6] = {
        {'W','S','A','D',VK_SPACE,'E'},
        {VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_RETURN,VK_RCONTROL}
    };
    while(true) {
        agora = chrono::steady_clock::now();
        for(int x = 0; x < LINHAS; x++)
            for(int y = 0; y < COLUNAS; y++)
                if(efeitos[x][y].ativo && agora >= efeitos[x][y].fim) {
                    efeitos[x][y].ativo = false;
                    if(m[x][y] == EXPLOSAO) m[x][y] = CAMINHO;
                    itens[x][y] = efeitos[x][y].drop;
                    efeitos[x][y].drop = SEM_BONUS;
                }
        for(int i = 0; i < static_cast<int>(bombas.size()); i++)
            if(bombas[i].ativa &&
               chrono::duration_cast<chrono::seconds>(agora-bombas[i].inicio).count() >=
               (bombas[i].remota ? TEMPO_MAXIMO_REMOTA_SEGUNDOS : tempoBombaSegundos))
                detonarBomba(i,m,bombas,efeitos,registro,agora);
        chefeAtingidoPelaExplosao(chefe,efeitos[chefe.x][chefe.y].ativo,registro,agora);

        verificarInimigosAtingidos(m,inimigos);
        bool perdeu = false;
        for(int i = 0; i < numeroJogadores; i++) {
            Jogador& j = jogadores[i];
            if(!receberDano(j,efeitos[j.x][j.y].ativo,tocaInimigo(j.x,j.y),agora))
                perdeu = true;
        }
        bool sortear = chrono::duration_cast<chrono::milliseconds>(
            agora-inicioMovimentoInimigo).count() >= tempoSorteioInimigoMs;
        if(sortear) inicioMovimentoInimigo = agora;
        if(!perdeu) movimentoInimigo(m,inimigos,sortear,agora);
        if(!perdeu) movimentoChefe(m,chefe,jogadores,numeroJogadores,agora);

        for(int i = 0; i < numeroJogadores && !perdeu; i++) {
            Jogador& j = jogadores[i];
            if(!receberDano(j,efeitos[j.x][j.y].ativo,tocaInimigo(j.x,j.y),agora)) {
                perdeu = true;
                break;
            }
            if(chrono::duration_cast<chrono::milliseconds>(agora-j.ultimoPasso).count() >= 150) {
                int dx = 0, dy = 0;

                if(modoAutomatico && i == 0) {
                    dx = 0;
                    dy = 0;
                }
                else if(pressionada(teclas[i][0]) || (numeroJogadores==1 && pressionada(VK_UP))) dx=-1;
                else if(pressionada(teclas[i][1]) || (numeroJogadores==1 && pressionada(VK_DOWN))) dx=1;
                else if(pressionada(teclas[i][2]) || (numeroJogadores==1 && pressionada(VK_LEFT))) dy=-1;
                else if(pressionada(teclas[i][3]) || (numeroJogadores==1 && pressionada(VK_RIGHT))) dy=1;
                if(dx || dy) {
                    j.ultimoPasso = agora;
                    int nx=j.x+dx, ny=j.y+dy;
                    bool ocupado = numeroJogadores==2 && jogadores[1-i].x==nx && jogadores[1-i].y==ny;
                    bool dentro = nx>=0 && nx<LINHAS && ny>=0 && ny<COLUNAS;
                    bool livre = dentro &&
                        (m[nx][ny]==CAMINHO || m[nx][ny]==EXPLOSAO || m[nx][ny]==PORTAL ||
                         (j.atravessa && m[nx][ny]==PAREDE_DESTRUTIVEL));
                    if(livre && !ocupado) {
                        j.x=nx; j.y=ny;
                        registro.movimentos++; registro.pontuacao--;
                    }
                }
            }
            if(!receberDano(j,efeitos[j.x][j.y].ativo,tocaInimigo(j.x,j.y),agora)) {
                perdeu=true; break;
            }
            if(!efeitos[j.x][j.y].ativo && itens[j.x][j.y]!=SEM_BONUS) {
                aplicarBonus(j,itens[j.x][j.y]);
                itens[j.x][j.y]=SEM_BONUS;
            }
            bool colocar=pressionada(teclas[i][4]), detonar=pressionada(teclas[i][5]);

            if(modoAutomatico && i == 0) {
                bool botColocar = false;
                int antesX = j.x, antesY = j.y;
                movimentoBot(m, j, bombas, efeitos, inimigos, chefe, botColocar, agora);
                if(j.x != antesX || j.y != antesY) {
                    registro.movimentos++;
                    registro.pontuacao--;
                }
                colocar = botColocar;
                detonar = false;
            }

            if(detonar && !j.detonarAnterior)
                for(int k=0; k<static_cast<int>(bombas.size()); k++)
                    if(bombas[k].ativa && bombas[k].dono==i && bombas[k].remota)
                        detonarBomba(k,m,bombas,efeitos,registro,agora);
            int ativas=0;
            for(const BombaJogador& bomba : bombas)
                if(bomba.ativa && bomba.dono==i) ativas++;
            if(colocar && !j.colocarAnterior && !portalAtivo && ativas<j.capacidade &&
               m[j.x][j.y]==CAMINHO && !efeitos[j.x][j.y].ativo) {
                BombaJogador bomba = {j.x,j.y,i,j.alcance,j.relogio,j.pierce,true,agora};
                bombas.push_back(bomba);
                m[j.x][j.y]=BOMBA;
                registro.bombas++; registro.pontuacao-=5;
            }
            j.colocarAnterior=colocar; j.detonarAnterior=detonar;
        }
        verificarInimigosAtingidos(m,inimigos);
        chefeAtingidoPelaExplosao(chefe,efeitos[chefe.x][chefe.y].ativo,registro,agora);
        int mortos=0;
        for(int i=0; i<quantidadeInimigos; i++) if(!inimigos[i].vivo) mortos++;
        registro.pontuacao+=(mortos-contabilizados)*100;
        contabilizados=mortos;
        bool perigos=false;
        for(const BombaJogador& bomba : bombas) if(bomba.ativa) perigos=true;
        for(int x=0; x<LINHAS; x++)
            for(int y=0; y<COLUNAS; y++)
                if(efeitos[x][y].ativo) perigos=true;
        if(!portalAtivo && mortos==quantidadeInimigos && !chefeVivo(chefe) && !perigos) {
            criarPortal(m,jogadores[0].x,jogadores[0].y);
            portalAtivo=true;
        }
        int tela[LINHAS][COLUNAS];
        for(int x=0; x<LINHAS; x++)
            for(int y=0; y<COLUNAS; y++) {
                tela[x][y]=m[x][y];
                if(m[x][y]==CAMINHO && itens[x][y]!=SEM_BONUS)
                    tela[x][y]=10+static_cast<int>(itens[x][y]);
            }
        if(chefeVivo(chefe) && m[chefe.x][chefe.y]==BOSS && agora<chefe.invulneravelAte)
            tela[chefe.x][chefe.y]=BOSS_FERIDO;
        imprimirJogo(tela,jogadores[0].x,jogadores[0].y,coord,
                     numeroJogadores==2 ? jogadores[1].x : -1,
                     numeroJogadores==2 ? jogadores[1].y : -1);
        long long segundosFase = chrono::duration_cast<chrono::seconds>(agora-inicioFase).count();
        cout << "Fase " << fase << "/" << TOTAL_FASES << " (" << nomeDificuldade() << ")"
             << " | Tempo: " << formatarTempo(segundosFase)
             << " | Pontos: " << registro.pontuacao
             << " | Movimentos: " << registro.movimentos
             << " | Bombas usadas: " << registro.bombas << "          \n";
        cout << "Inimigos: " << quantidadeInimigos - mortos << "/" << quantidadeInimigos << " restantes          \n";
        for(int i=0; i<numeroJogadores; i++) {
            const Jogador& j=jogadores[i];
            cout << VERDE << "P" << i+1 << RESETAR_COR
                 << " Vidas:" << textoHud(j.vidas) << " Alcance:" << textoHud(j.alcance)
                 << " Bombas:" << textoHud(j.capacidade)
                 << " | Remota:" << textoHud(j.relogio) << " Escudo:" << textoHud(j.escudo)
                 << " Pierce:" << textoHud(j.pierce) << " Atravessa:" << textoHud(j.atravessa) << "       \n";
        }
        if(chefe.ativo) {
            int vidaVisivel = limitar(chefe.vida, 0, vidaBoss);
            cout << "BOSS: " << string(vidaVisivel,'#') << string(vidaBoss-vidaVisivel,'-')
                 << " (" << vidaVisivel << "/" << vidaBoss << ")" << (chefe.vida>0 ? "  Ele segue voce!" : "  Derrotado!") << "        \n";
        }
        cout << SIMBOLO_ALCANCE << " alcance | " << SIMBOLO_BOMBAS << " bombas | " << SIMBOLO_VIDA << " vida | "
             << SIMBOLO_REMOTA << " remota | " << SIMBOLO_ESCUDO << " escudo | " << SIMBOLO_PIERCE << " pierce | "
             << SIMBOLO_ATRAVESSA << " atravessa\n";
        if(modoAutomatico)
            cout << "MODO AUTOMATICO: o computador controla o P1.\n";
        else
            cout << "Detonar: P1 E / P2 Ctrl direito (sozinha apos " << TEMPO_MAXIMO_REMOTA_SEGUNDOS << "s). Bonus resetam a cada fase.\n";
        cout << (portalAtivo ? "Entre no portal [] para avancar.                  " :
                                "Elimine os inimigos e o boss.                      ") << "\n";
        if(perdeu) {
            cout << "MORREU\n";
            pararMusica();
            tocarEfeito("faaah.mp3");
            Sleep(TEMPO_MENSAGEM_MILISSEGUNDOS);
            while(_kbhit()) _getch();
            return false;
        }
        for(int i=0; i<numeroJogadores; i++) {
            Jogador& j=jogadores[i];
            if(!receberDano(j,efeitos[j.x][j.y].ativo,tocaInimigo(j.x,j.y),agora)) {
                cout << "MORREU\n";
                pararMusica();
                tocarEfeito("faaah.mp3");
                Sleep(TEMPO_MENSAGEM_MILISSEGUNDOS);
                while(_kbhit()) _getch();
                return false;
            }
            if(portalAtivo && m[j.x][j.y]==PORTAL) {
                cout << "FASE CONCLUIDA!\n";
                while(_kbhit()) _getch();
                return true;
            }
        }
        bombas.erase(remove_if(bombas.begin(),bombas.end(),
            [](const BombaJogador& b) { return !b.ativa; }),bombas.end());
        while(_kbhit()) _getch();
        Sleep(10);
    }
}

void mostrarCursor() {
    CONSOLE_CURSOR_INFO cursorInfo = {100, true};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

char perguntarOpcao(const string& validas) {
    char tecla;
    do {
        tecla = static_cast<char>(toupper(_getch()));
    } while(validas.find(tecla) == string::npos);
    return tecla;
}

char telaFimDeJogo(bool vitoria, int fase, const RegistroRanking& registro) {
    system("cls");
    if(vitoria) {
        cout << "====== PARABENS! VOCE ZEROU O JOGO ======\n\n";
    }
    else {
        cout << "============== FIM DE JOGO ==============\n\n";
        cout << "  Voce perdeu na fase " << fase << " de " << TOTAL_FASES << ".\n\n";
    }
    cout << "  Jogador:    " << registro.jogador << '\n'
         << "  Pontuacao:  " << registro.pontuacao << '\n'
         << "  Bombas:     " << registro.bombas << '\n'
         << "  Movimentos: " << registro.movimentos << "\n\n";
    if(!vitoria) {
        cout << "  C - Continuar: tenta a fase " << fase << " de novo\n"
             << "      (a pontuacao volta ao valor do inicio da fase)\n";
    }
    cout << "  R - Reiniciar: salva esta partida e comeca de novo na fase 1\n";
    cout << "  M - Voltar ao menu: salva esta partida\n\n";
    cout << "  Escolha: ";
    return perguntarOpcao(vitoria ? "RM" : "CRM");
}

void gravarPartida(const RegistroRanking& registro) {
    cout << "\n";
    if(salvarRanking(registro)) cout << "Partida salva em ranking.txt.\n";
    else cout << "Erro ao salvar ranking.txt. Verifique a permissao de escrita na pasta.\n";
    Sleep(1000);
}

void jogarCampanha(int numeroJogadores = 1, bool modoAutomatico = false) {
    system("cls");
    mostrarCursor();
    RegistroRanking registro;
    cout << (numeroJogadores == 2 ? "Nome da dupla (ranking compartilhado):\n" : "");
    registro.jogador = pedirNome();
    if(registro.jogador.empty()) return;

    bool jogarDeNovo = true;
    while(jogarDeNovo) {
        jogarDeNovo = false;
        registro.pontuacao = 0;
        registro.bombas = 0;
        registro.movimentos = 0;

        int fase = 1;
        bool venceuTudo = false;
        bool encerrar = false;
        while(fase <= TOTAL_FASES && !encerrar) {
            RegistroRanking inicioFase = registro;
            bool venceu = jogar(fase, registro, numeroJogadores, modoAutomatico);
            pararMusica();
            mostrarCursor();

            if(venceu) {
                if(fase < TOTAL_FASES) {
                    cout << "\nFase " << fase << " concluida! Pressione qualquer tecla para a proxima...";
                    _getch();
                }
                else {
                    venceuTudo = true;
                }
                fase++;
            }
            else {
                char opcao = telaFimDeJogo(false, fase, registro);
                if(opcao == 'C') {
                    registro = inicioFase;
                }
                else {
                    encerrar = true;
                    jogarDeNovo = (opcao == 'R');
                }
            }
        }

        if(venceuTudo) {
            jogarDeNovo = (telaFimDeJogo(true, TOTAL_FASES, registro) == 'R');
        }
        gravarPartida(registro);
    }
}

int main() {
    aplicarDificuldade(dificuldadeAtual);

    while(true) {
        system("cls");
        cout << "=============================\n";
        cout << "          BOMBERMAN\n";
        cout << "=============================\n\n";
        cout << "  Dificuldade: " << nomeDificuldade() << "\n\n";
        cout << "  1 - Jogar sozinho\n";
        cout << "  2 - Jogar com 2 jogadores (cooperativo)\n";
        cout << "  3 - Como jogar\n";
        cout << "  4 - Pontuacao (como os pontos sao calculados)\n";
        cout << "  5 - Dificuldade\n";
        cout << "  6 - Ranking\n";
        cout << "  7 - Jogar no modo automatico (computador)\n";
        cout << "  0 - Sair\n\n";
        cout << "  Escolha: ";

        int opcao = _getch();

        if(opcao == '1') {
            jogarCampanha();
        }
        else if(opcao == '2') {
            jogarCampanha(2);
        }
        else if(opcao == '3') {
            telaComoJogar();
        }
        else if(opcao == '4') {
            telaPontuacao();
        }
        else if(opcao == '5') {
            telaDificuldade();
        }
        else if(opcao == '6') {
            telaRanking();
        }
        else if(opcao == '7') {
            jogarCampanha(1, true);
        }
        else if(opcao == '0') {
            return 0;
        }
    }
    return 0;
}
