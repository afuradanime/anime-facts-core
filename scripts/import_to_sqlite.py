#!/usr/bin/env python3
"""
Import anime-offline-database JSONL into SQLite database.
"""

import json
import sqlite3
from pathlib import Path
from typing import Any, Dict, List, Optional
import sys

def create_tables(conn: sqlite3.Connection) -> None:
    """Create SQLite tables based on the schema."""

    cursor = conn.cursor()

    # Drop existing tables
    cursor.execute("DROP TABLE IF EXISTS anime")
    cursor.execute("DROP TABLE IF EXISTS anime_season")
    cursor.execute("DROP TABLE IF EXISTS synonyms")
    cursor.execute("DROP TABLE IF EXISTS studios")
    cursor.execute("DROP TABLE IF EXISTS producers")
    cursor.execute("DROP TABLE IF EXISTS related_anime")
    cursor.execute("DROP TABLE IF EXISTS tags")
    cursor.execute("DROP TABLE IF EXISTS description")
    cursor.execute("DROP TABLE IF EXISTS language")

    # Main anime table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS anime (
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
        )
    """)
    
    # Anime season table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS anime_season (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            season TEXT NOT NULL,
            year INTEGER,
            FOREIGN KEY (anime_id) REFERENCES anime(id)
        )
    """)
    
    # Synonyms table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS synonyms (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            synonym TEXT NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id)
        )
    """)
    
    # Studios table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS studios (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            studio TEXT NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id)
        )
    """)
    
    # Producers table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS producers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            producer TEXT NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id)
        )
    """)
    
    # Related anime table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS related_anime (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            related_url TEXT NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id)
        )
    """)
    
    # Tags lookup table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            tag TEXT NOT NULL UNIQUE
        )
    """)
    
    # Anime tags junction table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS anime_tags (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            tag_id INTEGER NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id),
            FOREIGN KEY (tag_id) REFERENCES tags(id)
        )
    """)

    # Language table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS language (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            language TEXT NOT NULL
        )
    """)

    cursor.execute("""
        INSERT INTO language (language)
        VALUES (?)
    """, ("English",))

    cursor.execute("""
        INSERT INTO language (language)
        VALUES (?)
    """, ("Portuguese",))

    # Description table
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS description (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            anime_id INTEGER NOT NULL,
            description TEXT NOT NULL,
            language INTEGER NOT NULL,
            FOREIGN KEY (anime_id) REFERENCES anime(id),
            FOREIGN KEY (language) REFERENCES language(id)
        )
    """)
    
    conn.commit()


def insert_anime(conn: sqlite3.Connection, anime_data: Dict[str, Any]) -> int:
    """Insert anime record and return the anime_id."""
    cursor = conn.cursor()
    
    # Prepare main anime fields
    sources = json.dumps(anime_data.get("sources", []))
    title = anime_data.get("title")
    anime_type = anime_data.get("type")
    episodes = anime_data.get("episodes", 0)
    status = anime_data.get("status")
    picture = anime_data.get("picture")
    thumbnail = anime_data.get("thumbnail")
    
    # Optional duration
    duration_value = None
    duration_unit = None
    if "duration" in anime_data and anime_data["duration"]:
        duration_value = anime_data["duration"].get("value")
        duration_unit = anime_data["duration"].get("unit")
    
    # Optional rating and color
    rating = anime_data.get("rating")
    color = anime_data.get("color")
    
    cursor.execute("""
        INSERT INTO anime 
        (sources, title, type, episodes, status, picture, thumbnail, 
         duration_value, duration_unit, rating, color)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (sources, title, anime_type, episodes, status, picture, thumbnail,
          duration_value, duration_unit, rating, color))
    
    return cursor.lastrowid


def insert_anime_season(conn: sqlite3.Connection, anime_id: int, 
                       anime_season: Dict[str, Any]) -> None:
    """Insert anime season record."""
    cursor = conn.cursor()
    
    season = anime_season.get("season", "UNDEFINED")
    year = anime_season.get("year")
    
    cursor.execute("""
        INSERT INTO anime_season (anime_id, season, year)
        VALUES (?, ?, ?)
    """, (anime_id, season, year))


def insert_array_data(conn: sqlite3.Connection, anime_id: int, 
                     table_name: str, column_name: str, 
                     values: List[str]) -> None:
    """Insert array data into a separate table."""
    cursor = conn.cursor()
    
    # Special handling for tags (use lookup table)
    if table_name == "tags":
        for tag in values:
            # Insert or ignore if tag already exists
            cursor.execute("INSERT OR IGNORE INTO tags (tag) VALUES (?)", (tag,))
            # Get the tag id
            cursor.execute("SELECT id FROM tags WHERE tag = ?", (tag,))
            tag_id = cursor.fetchone()[0]
            
            # Insert into junction table
            cursor.execute("""
                INSERT INTO anime_tags (anime_id, tag_id)
                VALUES (?, ?)
            """, (anime_id, tag_id))
    else:
        for value in values:
            cursor.execute(f"""
                INSERT INTO {table_name} (anime_id, {column_name})
                VALUES (?, ?)
            """, (anime_id, value))


def import_jsonl_to_sqlite(jsonl_path: Path, db_path: Path, 
                          skip_first: bool = True) -> None:
    """Import JSONL data into SQLite database."""
    
    conn = sqlite3.connect(str(db_path))
    create_tables(conn)
    
    total_records = 0
    error_count = 0
    
    try:
        with open(jsonl_path, 'r', encoding='utf-8') as f:
            for line_num, line in enumerate(f, 1):
                # Skip first line if it contains metadata
                if line_num == 1 and skip_first:
                    try:
                        meta = json.loads(line)
                        if "metadata" in meta or "version" in meta:
                            print(f"Skipping metadata line: {meta}")
                            continue
                    except json.JSONDecodeError:
                        pass
                
                try:
                    anime_data = json.loads(line.strip())
                    
                    # Insert main anime record
                    anime_id = insert_anime(conn, anime_data)
                    
                    # Insert anime season
                    if "animeSeason" in anime_data:
                        insert_anime_season(conn, anime_id, anime_data["animeSeason"])
                    
                    # Insert array fields
                    if "synonyms" in anime_data:
                        insert_array_data(conn, anime_id, "synonyms", "synonym", 
                                        anime_data["synonyms"])
                    
                    if "studios" in anime_data:
                        insert_array_data(conn, anime_id, "studios", "studio", 
                                        anime_data["studios"])
                    
                    if "producers" in anime_data:
                        insert_array_data(conn, anime_id, "producers", "producer", 
                                        anime_data["producers"])
                    
                    if "relatedAnime" in anime_data:
                        insert_array_data(conn, anime_id, "related_anime", "related_url", 
                                        anime_data["relatedAnime"])
                    
                    if "tags" in anime_data:
                        insert_array_data(conn, anime_id, "tags", "tag", 
                                        anime_data["tags"])
                    
                    total_records += 1
                    
                    if total_records % 100 == 0:
                        print(f"Processed {total_records} records...")
                        conn.commit()
                
                except json.JSONDecodeError as e:
                    error_count += 1
                    print(f"Error parsing line {line_num}: {e}", file=sys.stderr)
                except Exception as e:
                    error_count += 1
                    print(f"Error processing line {line_num}: {e}", file=sys.stderr)
        
        conn.commit()
        print(f"\n✓ Import complete!")
        print(f"  Total records inserted: {total_records}")
        print(f"  Errors encountered: {error_count}")
        
    except FileNotFoundError:
        print(f"Error: JSONL file not found: {jsonl_path}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Fatal error: {e}", file=sys.stderr)
        conn.rollback()
        sys.exit(1)
    finally:
        conn.close()


def main():
    """Main function."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Import anime-offline-database JSONL into SQLite"
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=Path(__file__).parent / "anime-offline-database.jsonl",
        help="Path to JSONL input file (default: anime-offline-database.jsonl in script directory)"
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).parent / "anime.db",
        help="Path to SQLite database file (default: anime.db in script directory)"
    )
    parser.add_argument(
        "--skip-first",
        action="store_true",
        default=True,
        help="Skip first line if it contains metadata (default: True)"
    )
    
    args = parser.parse_args()
    
    print(f"Input file:  {args.input}")
    print(f"Output DB:   {args.output}")
    print(f"Skip first line: {args.skip_first}\n")
    
    import_jsonl_to_sqlite(args.input, args.output, args.skip_first)


if __name__ == "__main__":
    main()
