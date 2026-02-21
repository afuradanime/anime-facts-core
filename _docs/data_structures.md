# As estruturas de dados

## Enumerações

## enum anime_type
O tipo de produção do anime:
```c
enum anime_type {
    TV = 1,     // Série televisiva
    OVA,        // Original Video Animation
    MOVIE,      // Filme
    SPECIAL,    // Especial
    ONA,        // Original Net Animation (lançado online)
    MUSIC,      // Vídeo musical
    UNKNOWN_TYPE
};
```

## enum anime_status
O estado de lançamento do anime:
```c
enum anime_status {
    FINISHED_AIRING = 1,    // Terminado
    CURRENTLY_AIRING,       // A ser lançado actualmente
    NOT_YET_AIRED,          // Ainda não lançado
    UNKNOWN_STATUS
};
```

## enum meteorological_season
A estação do ano em que o anime foi lançado. Definida dentro de `season_t`:
```C
enum meteorological_season {
    SPRING,     // Primavera
    SUMMER,     // Verão
    FALL,       // Outono
    WINTER,     // Inverno
    UNDEFINED
};
```

## enum language
Os idiomas disponíveis para descrições. Definida dentro de `description_t`:
```C
enum language {
    ENGLISH    = 1,
    PORTUGUESE = 2
};
```

## enum tag_type
A categoria de uma tag. Definida dentro de `tag_t`:
```c
enum tag_type {
    TAG_GENRE,          // Género (Acção, Comédia, ...)
    TAG_THEME,          // Tema (Escola, Militar, ...)
    TAG_DEMOGRAPHIC,    // Demográfico (Shounen, Seinen, ...)
    TAG_EXPLICIT_GENRE  // Género explícito
};
```

## [pageable_t](../include/pageable.h)
Uma estrutura para especificar dados de paginação: 
1. O número da página
2. O número elementos por página
```C
struct pageable {
    unsigned short page_size;
    unsigned short page_number;
};

typedef struct pageable pageable_t;
```

## [Arrays dinâmico](../include/dynamic_array.h)
Os arrays dinâmico de campos que são armazenados como listas são cinco. Como é uma declaração repetitiva tem uma macro para auxiliar a sua definição
```C
#define DEFINE_DYNAMIC_ARRAY(type_name, item_type) \
typedef struct { \
    item_type* items; \
    size_t count; \
    size_t capacity; \
} type_name;

DEFINE_DYNAMIC_ARRAY(string_array_t, char*)
DEFINE_DYNAMIC_ARRAY(tag_array_t, tag_t)
DEFINE_DYNAMIC_ARRAY(producer_array_t, producer_t)
DEFINE_DYNAMIC_ARRAY(licensor_array_t, licensor_t)
DEFINE_DYNAMIC_ARRAY(studio_array_t, studio_t)
DEFINE_DYNAMIC_ARRAY(description_array_t, description_t)
```

O seu uso é simples; as suas interfaces consistem de 3 métodos:
```C
void init_tag_array(tag_array_t* arr);                                                                          // Criar um array dinâmico
void push_tag(tag_array_t* arr, unsigned int id, const char* name, unsigned char type, const char* url);        // Adicionar um elemento
void free_tag_array(tag_array_t* arr);                                                                          // Libertar um array dinâmico
```

Aqui exemplifica-se tag arrays, mas os outros funcionam da mesma maneira.

## [anime_t](../include/anime.h)
A estrutura com todos os dados pertinentes a um anime, ao contrário de um `partial_anime_t`[^1] um `anime_t` inclui listas de tags (géneros), produtores, licenciadores, estúdios, sinónimos e descrições
```c
struct anime {
    unsigned int id;        // Id na base de dados
    char* url;              // URL no My Anime List
    char* title;            // Título principal
    
    string_array_t synonyms;            // Variações do título
    description_array_t descriptions;   // Descrições do anime
    
    enum anime_type type;               // Tipo de anime
    
    char* source;           // Fonte original ("Original", "Manga", "Light novel", ...)
    unsigned int episodes;  // Número de episódios
    
    enum anime_status status;   // Estado de lançamento ("A sair", "Acabado", ...)
    
    bool airing;            // Currently airing flag
    char* duration;         // Duration string "24 min per ep")
    
    char* start_date;       // Data de começo de lançamento (formato ISO 8601)
    char* end_date;         // Data de fim de lançamento (formato ISO 8601)
    season_t season;        // Season em que lançou
    
    broadcast_t broadcast;  // Informações de lançamento
    
    char* image_url;        // URL de uma capa de qualidade normal
    char* small_image_url;  // URL de uma capa de qualidade baixa
    char* large_image_url;  // URL de uma capa de qualidade alta
    
    char* trailer_embed_url;    // Embed code do trailer (Youtube)
    
    tag_array_t tags;               // Lista dinámica de tags
    producer_array_t producers;     // Lista dinámica de productores
    licensor_array_t licensors;     // Lista dinámica de licenciadores
    studio_array_t studios;         // Lista dinámica de estúdios
};

typedef struct anime anime_t;
```

## [partial_anime_t](../include/anime.h)
Uma versão reduzida de `anime_t` sem os arrays dinâmicos. Usado em todas as funções que retornam listas de animes para evitar os JOINs necessários para popular as listas[^1].
```c
struct partial_anime {
    unsigned int id;
    char* url;              // URL no My Anime List
    char* title;            // Título principal

    enum anime_type type;

    char* source;           // Fonte original ("Original", "Manga", "Light novel", ...)
    unsigned int episodes;  // Número de episódios

    enum anime_status status;

    bool airing;
    char* duration;         // "24 min per ep"

    char* start_date;       // Formato ISO 8601
    char* end_date;         // Formato ISO 8601 (pode ser NULL)
    season_t season;

    broadcast_t broadcast;

    char* image_url;
    char* small_image_url;
    char* large_image_url;

    char* trailer_embed_url;
};

typedef struct partial_anime partial_anime_t;
```

## [anime_filter_t](../include/anime.h)
Estrutura de filtros para pesquisas. Todos os campos são opcionais (nullable). Um campo `NULL` é simplesmente ignorado na query.
```c
struct anime_filter {
    enum anime_type*   type;            // Filtrar por tipo de anime
    enum anime_status* status;          // Filtrar por estado de lançamento

    time_t* start_date;                 // Animes com início após esta data
    time_t* end_date;                   // Animes com fim antes desta data
    season_t* season;                   // Filtrar por estação e ano

    char* name;                         // Pesquisa por título (LIKE %name%)
    unsigned int* min_episodes;         // Número mínimo de episódios (exclusivo)
    unsigned int* max_episodes;         // Número máximo de episódios (exclusivo)
};

typedef struct anime_filter anime_filter_t;
```

Um filtro vazio (todos os campos a `NULL`) retorna resultados sem restrições, ordenados por `quality_score` descendente.

## Tipos auxiliares

## `season_t`
```c
typedef struct season {
    enum meteorological_season season;
    unsigned short year;
} season_t;
```

## `broadcast_t`
Informação sobre o horário de transmissão semanal:
```c
typedef struct broadcast {
    char* day;       // Ex: "Saturdays"
    char* time;      // Ex: "01:00"
    char* timezone;  // Ex: "Asia/Tokyo"
} broadcast_t;
```

## `description_t`
```c
typedef struct description {
    enum language language;
    char* description;
} description_t;
```

## `producer_t` / `licensor_t`
```c
typedef struct producer {
    unsigned int id;
    char* name;
    char* type;  // Ex: "Producer", "Studio"
    char* url;   // URL no My Anime List
} producer_t;

// licensor_t tem a mesma estrutura
```

## `studio_t`
```c
typedef struct studio {
    unsigned int id;
    char* name;
    char* url;  // URL no My Anime List
} studio_t;
```

## `tag_t`
```c
typedef struct tag {
    unsigned int id;
    char* name;
    enum tag_type type;
    char* url;  // URL no My Anime List
} tag_t;
```

# Notas de rodapé
[^1] A query de um anime por completo, com todos os JOINS que precisa para as suas listas, tornou-se muito lenta e inconveniente, portanto foi criada a distinção entre "anime parcial" e "anime", um anime parcial não possui nenhum dos arrays dinâmicos que um anime completo possui, estes são retornados sempre que pesquisas por listas (até porque geralmente quando listamos animes esses dados são irrelevantes) e quando pesquisas por um anime individual (`fetch_anime_by_id`, `fetch_random_anime`) é retornado um anime completo.