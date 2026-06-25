# Planejamento

Programa é dividido em 4 componentes

## Engine

A engine deve conter toda a lógica do predator-prey model:

- mundo/mapa
- agentes (predadores e presas)
- regras de movimento
- reprodução
- morte
- passagem do tempo
- geração aleatória

API:
```c
struct Config {
    usize_t map_length_x;
    usize_t map_length_y;
}

struct WorldState {
    usize_t tick;
    **char map;
    *usize_t statistics;
}

create_world(config)
reset()

continue()
pause()
step()
run(usize_t n_steps)

get_state()
get_statistics()

add_cell(usize_t pos_x, usize_t pos_y, char type)

save(*char path)
load(*char path)

set_seed(seed)
```

## GUI:
Interface gráfica com sprites e etc.

## TUI:
Interface no terminal

## GUI text mode:
Interface gráfica mas simulação em modo de texto
