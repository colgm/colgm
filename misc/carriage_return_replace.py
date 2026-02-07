import os
import argparse

def replace_carriage_returns_in_file(file_path, create_backup=False):
    """
    Replace ^M characters (\r) in the file and save the modified content.

    :param file_path: File path
    :param create_backup: Whether to create a backup file (default False)
    """
    try:
        # Create backup file (if needed)
        if create_backup:
            backup_path = file_path + '.bak'
            with open(file_path, 'rb') as original, open(backup_path, 'wb') as backup:
                backup.write(original.read())
            print(f"Backup file created: {backup_path}")

        # Read file content and replace ^M characters
        with open(file_path, 'r', encoding='utf-8', newline='') as f:
            content = f.read()

        # Replace all \r characters
        if '\r' not in content:
            return
        cleaned_content = content.replace('\r', '')

        # Write back to original file
        with open(file_path, 'w', encoding='utf-8', newline='') as f:
            f.write(cleaned_content)

        print(f"File processed: {file_path}")
    except Exception as e:
        print(f"Failed to process file: {file_path}, error message: {e}")

def process_directory(directory, extensions, create_backup=False):
    """
    Traverse directory and process files with specified extensions.

    :param directory: Directory path
    :param extensions: List of file extensions to process (e.g., ['.txt', '.log'])
    :param create_backup: Whether to create backup files
    """
    for root, _, files in os.walk(directory):
        for file in files:
            # Check if file extension matches
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                replace_carriage_returns_in_file(file_path, create_backup)

if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="Replace ^M characters in files")
    parser.add_argument(
        "directory",
        help="Directory path to process"
    )
    parser.add_argument(
        "-e", "--extensions",
        nargs="+",
        default=[
            ".md", ".yml",
            ".h", ".c", ".cpp",
            ".nas", ".py", ".colgm",
            ".result", ".gitignore", ".gitattributes"
        ],
        help="List of file extensions to process, default is ['.txt']"
    )
    parser.add_argument(
        "-b", "--backup",
        action="store_true",
        help="Whether to create backups of original files (default not to create)"
    )

    args = parser.parse_args()

    # Execute processing logic
    print(f"Starting to process directory: {args.directory}")
    print(f"Target file extensions: {args.extensions}")
    print(f"Create backup: {'Yes' if args.backup else 'No'}")

    process_directory(args.directory, args.extensions, args.backup)

    print("Processing completed!")