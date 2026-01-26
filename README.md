# Anime facts core
O motor de pesquisa de animes do AfuradaAnime, feito para ser portátil e rápido.

Este módulo C fornece uma _interface_ para pesquisar uma base de dados [SQLite](https://sqlite.org/) directamente. Foi feito para integrar em aplicações compiladas de maneira a aceder informações detalhadas sobre animes (titulo, capa, apelidos, número de episódios, entre outros) sem precisar de se contactar um API externo ou enviar pedidos _web_ de todo.

Para a informação completa dos dados suportados, é favor ler [o modelo lógico da base de dados](_docs/svg/l_model.svg) ou o [modelo físico](_docs/physical_model.sql) para aqueles mais técnicos.

**Dependências**
1. **[SQLite3](https://sqlite.org/)** - SGBD utilizado

## Como fazer build
Para fazer build do projecto é preciso primeiro compilar o [código fonte do SQLite](https://github.com/sqlite/sqlite) para objecto, o _Makefile_ possiu um _target_ para tal. [^1] 
```bash
make sqlite
```

[1] Nota: Esta compilação só é precisa ser feita uma vez.

Dada a existència do `sqlite.o` o programa está pronto para ser _built_ como biblioteca partilhada. Para tal existe também um outro _target_:
```bash
make build_lib
```
Que vai criar um ficheiro `anime_facts.dll`. 
Este ficheiro pode ser utilizado em qualquer projecto capaz de interagir com bibliotecas dinâmicas (C, C++, Go, ...) tudo o que ele precisa é de uma base de dados bem formada.
## Como utilizar o motor de pesquisa
```c
#include <stdio.h>
#include "../anime_facts/include/anime_facts_api.h"

int main(void) {

    anime_t a;
    if (fetch_anime_by_id(15523, &a) == 0) {

        printf("[%d] %s\n", a.id, a.title);
        free_anime(&a);
    }

    return 0;
}
```

```bash
gcc main.c -L. -l anime_facts
```

Para a lista completa de cada função e as suas assinaturas, consultar a [documentação completa da API](_docs/complete_docs.md).