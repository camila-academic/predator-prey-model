# Projeto Final - Ecossistema de Zumbis em C

Este projeto e um exemplo completo para o trabalho de Estrutura de Dados em C.
Ele implementa uma simulacao em tabuleiro com humanos, zumbis e soldados,
alem de obstaculos, movimentacao aleatoria, combate, reproducao e condicoes
de parada.

## Estrutura

```text
.
+-- include/
|   +-- board.h
|   +-- entities.h
|   +-- simulation.h
|   +-- types.h
|   +-- utils.h
+-- src/
|   +-- board.c
|   +-- entities.c
|   +-- main.c
|   +-- simulation.c
|   +-- utils.c
+-- docs/
|   +-- relatorio.md
+-- Makefile
+-- build.bat
+-- README.md
```

## Como compilar

## Versao jogavel no navegador

Abra o arquivo `jogo.html` com duplo clique para executar a versao jogavel.
Essa versao nao precisa de compilador e possui tabuleiro visual, botoes de
controle, placar e historico de eventos.

Controles disponiveis:

- `Iniciar`: roda a simulacao automaticamente.
- `Pausar`: interrompe a simulacao.
- `Rodada`: executa uma rodada por vez.
- `Reiniciar` ou `Novo mapa`: cria uma nova partida.

## Jogo em C em arquivo unico

O arquivo `jogo.c` contem uma versao completa em C para terminal. Ele junta em
um unico codigo:

- tabuleiro dinamico;
- humanos, zumbis e soldados;
- arvores e pedras como obstaculos;
- movimentacao aleatoria;
- combate;
- reproducao;
- energia dos zumbis;
- menu para executar uma rodada, 10 rodadas ou ate o fim.

Para compilar quando houver GCC instalado:

```bash
gcc jogo.c -o jogo
./jogo
```

No Windows:

```bat
gcc jogo.c -o jogo.exe
jogo.exe
```

## Jogo em C++ para compilador online

Se o seu compilador usa o arquivo `main.cpp`, use o arquivo `main.cpp` deste
projeto. Ele tem a mesma ideia do jogo, mas escrito em C++ valido.

Compile assim:

```bash
g++ main.cpp -o jogo
./jogo
```

## Checklist do enunciado

O arquivo `docs/checklist_requisitos_c.md` mostra como a versao em C atende as
orientacoes do projeto.

## Versao em C

No Windows, com GCC instalado:

```bat
build.bat
ecosystem.exe
```

Em Linux/macOS ou terminal com `make`:

```bash
make
./ecosystem
```

Tambem e possivel compilar diretamente:

```bash
gcc -Wall -Wextra -pedantic -std=c11 -Iinclude src/main.c src/utils.c src/entities.c src/board.c src/simulation.c -o ecosystem
```

## Regras implementadas

- O usuario escolhe o tamanho do tabuleiro.
- O menor tabuleiro permitido e 5x5.
- O programa calcula automaticamente a quantidade de entidades:
  - Humanos: 12% das celulas.
  - Zumbis: 12% das celulas.
  - Soldados: 5% das celulas.
  - Obstaculos: 10% das celulas.
- Todos os elementos sao posicionados aleatoriamente.
- Obstaculos nao se movem e bloqueiam passagem.
- Entidades se movem para cima, baixo, esquerda ou direita.
- Zumbis eliminam humanos.
- Soldados eliminam zumbis.
- Entidades iguais podem gerar novas entidades.
- Zumbis perdem energia por rodada e recuperam energia ao atacar humanos.

## Estruturas de dados usadas

- Matriz linearizada para representar obstaculos do tabuleiro.
- Vetor dinamico para armazenar entidades.
- Structs para organizar entidades, tabuleiro, posicoes e estatisticas.
- Ponteiros e alocacao dinamica com `malloc`, `calloc`, `realloc` e `free`.

## Ideias para evoluir

- Adicionar clima.
- Criar multiplos mapas.
- Fazer uma IA simples para perseguir ou fugir.
- Salvar estatisticas em arquivo.
- Usar lista encadeada para comparar desempenho com vetor dinamico.
