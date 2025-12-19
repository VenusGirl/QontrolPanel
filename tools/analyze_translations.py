#!/usr/bin/env python3
import json
import xml.etree.ElementTree as ET
from pathlib import Path
import sys
import subprocess
from datetime import datetime

def analyze_ts_file(ts_file_path):
    try:
        tree = ET.parse(ts_file_path)
        root = tree.getroot()
       
        total_messages = 0
        unfinished_messages = 0
       
        for message in root.findall('.//message'):
            translation = message.find('translation')
            if translation is None:
                continue
               
            translation_type = translation.get('type', '').lower()
           
            if translation_type == 'vanished':
                continue
               
            total_messages += 1
           
            if translation_type == 'unfinished':
                unfinished_messages += 1
                   
        finished_messages = total_messages - unfinished_messages
       
        if total_messages == 0:
            percentage = 0
        else:
            percentage = int((finished_messages / total_messages) * 100)
           
        return {
            'total_messages': total_messages,
            'translated_messages': finished_messages,
            'percentage': percentage
        }
       
    except ET.ParseError as e:
        print(f"Error parsing {ts_file_path}: {e}")
        return None
    except Exception as e:
        print(f"Error processing {ts_file_path}: {e}")
        return None

def extract_language_code(filename):
    if filename.startswith('QontrolPanel_') and filename.endswith('.ts'):
        return filename[len('QontrolPanel_'):-len('.ts')]

    return filename

def get_last_commit_info(file_path):
    """Get the last commit hash for a specific file"""
    try:
        result = subprocess.run(
            ['git', 'log', '-1', '--format=%H %ci', '--', str(file_path)],
            capture_output=True,
            text=True,
            cwd=file_path.parent.parent
        )
        if result.returncode == 0 and result.stdout.strip():
            parts = result.stdout.strip().split(' ', 1)
            commit_hash = parts[0]
            date_str = parts[1] if len(parts) > 1 else None
            return commit_hash, date_str
        return None, None
    except Exception as e:
        print(f"  Warning: Could not get git commit info: {e}")
        return None, None

def count_ts_files_in_commit(commit_hash, i18n_dir):
    """Count how many .ts files were modified in a given commit"""
    try:
        result = subprocess.run(
            ['git', 'show', '--name-only', '--format=', commit_hash],
            capture_output=True,
            text=True,
            cwd=i18n_dir.parent
        )
        if result.returncode == 0:
            modified_files = result.stdout.strip().split('\n')
            ts_count = sum(1 for f in modified_files if f.startswith('i18n/') and f.endswith('.ts'))
            return ts_count
        return 0
    except Exception as e:
        print(f"  Warning: Could not count files in commit: {e}")
        return 0

def main():
    script_dir = Path(__file__).parent
    i18n_dir = script_dir.parent / 'i18n'
    compiled_dir = i18n_dir / 'compiled'

    compiled_dir.mkdir(exist_ok=True)

    output_file = compiled_dir / 'translation_progress.json'

    # Load existing data to preserve dates when multiple files are updated
    existing_progress = {}
    if output_file.exists():
        try:
            with open(output_file, 'r', encoding='utf-8') as f:
                existing_progress = json.load(f)
            print(f"Loaded existing translation progress from {output_file}")
        except Exception as e:
            print(f"Warning: Could not load existing progress: {e}")

    print("Analyzing translation files...")
    print(f"Looking for .ts files in: {i18n_dir}")

    ts_files = list(i18n_dir.glob('*.ts'))

    if not ts_files:
        print("No .ts files found in i18n directory!")
        sys.exit(1)

    print(f"Found {len(ts_files)} translation files")

    translation_progress = {}
   
    for ts_file in ts_files:
        print(f"Analyzing {ts_file.name}...")

        language_code = extract_language_code(ts_file.name)

        # Get last commit info
        commit_hash, commit_date = get_last_commit_info(ts_file)

        # Determine if we should update the date
        should_update_date = False
        if commit_hash:
            ts_count_in_commit = count_ts_files_in_commit(commit_hash, i18n_dir)
            if ts_count_in_commit == 1:
                should_update_date = True
                print(f"  Single .ts file in commit {commit_hash[:7]} - will update date")
            else:
                print(f"  {ts_count_in_commit} .ts files in commit {commit_hash[:7]} - preserving old date")

        # Decide which date to use
        if should_update_date:
            last_updated = commit_date
        else:
            # Preserve old date if it exists
            old_data = existing_progress.get(language_code, {})
            last_updated = old_data.get('last_updated')
            if not last_updated:
                # If no old date exists, use None instead of current commit date
                last_updated = None

        # Special case for English - always 100%
        if language_code.lower() in ['en', 'english', 'en_us', 'en_gb']:
            translation_progress[language_code] = {
                'percentage': 100,
                'last_updated': last_updated
            }
            print(f"  {language_code}: 100% (English - always complete)")
            if last_updated:
                print(f"    Last updated: {last_updated}")
            continue

        stats = analyze_ts_file(ts_file)
        if stats is None:
            print(f"Skipping {ts_file.name} due to errors")
            continue

        translation_progress[language_code] = {
            'percentage': stats['percentage'],
            'last_updated': last_updated
        }

        print(f"  {language_code}: {stats['translated_messages']}/{stats['total_messages']} = {stats['percentage']}%")
        if last_updated:
            print(f"    Last updated: {last_updated}")
   
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(translation_progress, f, indent=2, ensure_ascii=False)
       
        print(f"\nTranslation progress saved to: {output_file}")
        print("Contents:")
        print(json.dumps(translation_progress, indent=2))
       
    except Exception as e:
        print(f"Error saving JSON file: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()