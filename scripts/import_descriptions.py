#!/usr/bin/env python3
"""
Import anime descriptions from MyAnimeList CSV into SQLite database.
Matches by title and adds descriptions in English language.
"""

import csv
import sqlite3
from pathlib import Path
import sys


def import_descriptions_from_csv(csv_path: Path, db_path: Path) -> None:
    """Import descriptions from MyAnimeList CSV into SQLite database."""
    
    try:
        conn = sqlite3.connect(str(db_path))
        cursor = conn.cursor()
        
        # Get the English language ID
        cursor.execute("SELECT id FROM language WHERE language = 'English'")
        lang_result = cursor.fetchone()
        
        if not lang_result:
            print("Error: English language not found in database. Please run import_to_sqlite.py first.")
            sys.exit(1)
        
        english_lang_id = lang_result[0]
        
        # Read CSV and insert descriptions
        matched_count = 0
        not_matched_count = 0
        error_count = 0
        
        with open(csv_path, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            
            for row_num, row in enumerate(reader, 1):
                try:
                    title = row.get('title', '').strip()
                    synopsis = row.get('synopsis', '').strip()
                    
                    if not title or not synopsis:
                        continue
                    
                    # Find anime by title in database
                    cursor.execute("SELECT id FROM anime WHERE title = ?", (title,))
                    result = cursor.fetchone()
                    
                    if result:
                        anime_id = result[0]
                        
                        # Check if description already exists for this anime and language
                        cursor.execute("""
                            SELECT id FROM description 
                            WHERE anime_id = ? AND language = ?
                        """, (anime_id, english_lang_id))
                        
                        existing = cursor.fetchone()
                        
                        if not existing:
                            # Insert new description
                            cursor.execute("""
                                INSERT INTO description (anime_id, description, language)
                                VALUES (?, ?, ?)
                            """, (anime_id, synopsis, english_lang_id))
                            matched_count += 1
                        else:
                            # Update existing description
                            cursor.execute("""
                                UPDATE description 
                                SET description = ?
                                WHERE anime_id = ? AND language = ?
                            """, (synopsis, anime_id, english_lang_id))
                            matched_count += 1
                    else:
                        not_matched_count += 1
                    
                    if row_num % 500 == 0:
                        print(f"Processed {row_num} rows... ({matched_count} matched)")
                        conn.commit()
                
                except Exception as e:
                    error_count += 1
                    print(f"Error processing row {row_num}: {e}", file=sys.stderr)
        
        conn.commit()
        
        print(f"\n✓ Import complete!")
        print(f"  Descriptions matched and inserted: {matched_count}")
        print(f"  Titles not found in database: {not_matched_count}")
        print(f"  Errors encountered: {error_count}")
        
        conn.close()
        
    except FileNotFoundError:
        print(f"Error: CSV file not found: {csv_path}", file=sys.stderr)
        sys.exit(1)
    except sqlite3.Error as e:
        print(f"Database error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Fatal error: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    """Main function."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Import anime descriptions from MyAnimeList CSV into SQLite"
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=Path(__file__).parent / "data" / "myanimelist.csv",
        help="Path to CSV input file (default: data/myanimelist.csv in script directory)"
    )
    parser.add_argument(
        "--database",
        type=Path,
        default=Path(__file__).parent / "anime.db",
        help="Path to SQLite database file (default: anime.db in script directory)"
    )
    
    args = parser.parse_args()
    
    print(f"CSV file:      {args.input}")
    print(f"Database:      {args.database}")
    print()
    
    import_descriptions_from_csv(args.input, args.database)


if __name__ == "__main__":
    main()
