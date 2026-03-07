# Anime facts core
O motor de pesquisa de animes do AfuradaAnime, feito para ser portátil e rápido.

Este módulo C fornece uma _interface_ para pesquisar uma base de dados [SQLite](https://sqlite.org/) directamente. Foi feito para integrar em aplicações compiladas de maneira a aceder informações detalhadas sobre animes (titulo, capa, apelidos, número de episódios, entre outros) sem precisar de se contactar um API externo ou enviar pedidos _web_ de todo.

Para a informação completa dos dados suportados, é favor ler [o modelo lógico da base de dados](_docs/svg/l_model.svg) ou o [modelo físico](_docs/physical_model.sql) para aqueles mais técnicos.

**Requisitos**
1. **[SQLite3](https://sqlite.org/)** - SGBD utilizado
1. **[CMake](https://cmake.org/)** - Sistema de build utilizado

## Como fazer build
Para fazer build do projecto é preciso primeiro criar uma pasta `build`, para depois executar o cmake de lá:
```sh
mkdir build ; cd build ; cmake ..
```
E depois é fazer build do biblioteca.
```sh
cmake --build .
```
Que vai criar um ficheiro `anime_facts.dll`/`anime_facts.so` e uma biblioteca estática do sqlite. 
Este ficheiro pode ser utilizado em qualquer projecto capaz de interagir com bibliotecas dinâmicas (C, C++, Go, ...) tudo o que ele precisa é de uma base de dados bem formada.
## Como utilizar o motor de pesquisa
```c
#include <stdio.h>
#include "../anime-facts-core/include/anime_facts_api.h"

int main(void) {

    set_database_path("../anime.db");

    anime_t a;
    if (fetch_anime_by_id(5680, &a) == 0) {

        print_anime(&a);
        free_anime(&a);
    }

    return 0;
}
```

```bash
gcc main.c -L. -l anime_facts
```

Para a lista completa de cada função e as suas assinaturas, consultar a [documentação completa da API](_docs/complete_docs.md).

## Como formar a base de dados
A base de dados consiste inteiramente de informação open source. Para criar a base de dados existem alguns scripts python e um _Makefile_ na pasta `/scripts`. A informação da base de dados é proveniente do site [MyAnimeList](https://myanimelist.net/), extraída pelo _API_ não oficial [Jikan](https://jikan.moe/).

A formação da base de dados consiste de dois passos:
1. A extração da informação.
2. O _bootstrapping_ e inserção da informação num ficheiro SQLite.

No ficheiro `/scripts/Makefile` podem ser mudadas as constantes de nomes de ficheiros caso tal seja desejado. O target `make all` irá executar todos os scripts e automaticamente resultar numa base de dados formada no caminho indicado. Esta base de dados está agora pronta para ser usada pelo API.

> [!WARNING]  
> Este processo pode demorar mais de 10 minutos.