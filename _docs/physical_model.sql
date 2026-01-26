DROP TABLE IF EXISTS anime_tags;
DROP TABLE IF EXISTS description;
DROP TABLE IF EXISTS related_anime;
DROP TABLE IF EXISTS producers;
DROP TABLE IF EXISTS studios;
DROP TABLE IF EXISTS synonyms;
DROP TABLE IF EXISTS anime_season;
DROP TABLE IF EXISTS tags;
DROP TABLE IF EXISTS language;
DROP TABLE IF EXISTS anime;

-- Anime table
CREATE TABLE anime (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sources TEXT,
    title TEXT NOT NULL,
    type TEXT NOT NULL,
    episodes REAL NOT NULL,
    status TEXT NOT NULL,
    picture TEXT,
    thumbnail TEXT,
    duration_value REAL,
    duration_unit TEXT,
    rating TEXT,
    color TEXT
);

-- Anime season table
CREATE TABLE anime_season (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    season TEXT NOT NULL,
    year INTEGER,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE
);

-- Synonyms table
CREATE TABLE synonyms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    synonym TEXT NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE
);

-- Studios table
CREATE TABLE studios (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    studio TEXT NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE
);

-- Producers table
CREATE TABLE producers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    producer TEXT NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE
);

-- Related anime table
CREATE TABLE related_anime (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    related_url TEXT NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE
);

-- Tags lookup table
CREATE TABLE tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tag TEXT NOT NULL UNIQUE
);

-- Anime tags auxiliary table
CREATE TABLE anime_tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    tag_id INTEGER NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE,
    UNIQUE(anime_id, tag_id)
);

-- Language table
CREATE TABLE language (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    language TEXT NOT NULL UNIQUE
);

-- Description table
-- Descriptions can be user inserted in out current model
-- Since we don't have description data for all animes, we allow users to insert them
CREATE TABLE description (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    anime_id INTEGER NOT NULL,
    description TEXT NOT NULL,
    language INTEGER NOT NULL,
    FOREIGN KEY (anime_id) REFERENCES anime(id) ON DELETE CASCADE,
    FOREIGN KEY (language) REFERENCES language(id) ON DELETE RESTRICT
);

-- Title search index
CREATE INDEX idx_anime_title ON anime(title);

-- Foreign key indexes for JOIN performance
CREATE INDEX idx_anime_season_anime_id ON anime_season(anime_id);
CREATE INDEX idx_synonyms_anime_id ON synonyms(anime_id);
CREATE INDEX idx_studios_anime_id ON studios(anime_id);
CREATE INDEX idx_producers_anime_id ON producers(anime_id);
CREATE INDEX idx_related_anime_anime_id ON related_anime(anime_id);
CREATE INDEX idx_anime_tags_anime_id ON anime_tags(anime_id);
CREATE INDEX idx_anime_tags_tag_id ON anime_tags(tag_id);
CREATE INDEX idx_description_anime_id ON description(anime_id);
CREATE INDEX idx_description_language ON description(language);

-- Filter search indexes
CREATE INDEX idx_anime_type ON anime(type);
CREATE INDEX idx_anime_status ON anime(status);
CREATE INDEX idx_anime_season_year ON anime_season(year);
CREATE INDEX idx_tags_tag ON tags(tag);

-- Bootstrap

INSERT INTO language (language) VALUES ('English');
INSERT INTO language (language) VALUES ('Portuguese');