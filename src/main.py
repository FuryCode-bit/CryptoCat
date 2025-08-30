import os
import subprocess
import re
import time
import sys
from collections import defaultdict
from datetime import datetime
import pyfiglet
from rich.console import Console
from rich.panel import Panel
from rich.text import Text
from rich.prompt import Prompt
from rich.columns import Columns

# --- CONFIGURATION ---

TABLE_EXECUTABLE = "./bin/table"
GUESS_EXECUTABLE = "./bin/guess"
DATA_DIR = "./data"
 
LOG_LEVELS = {"info": 2, "error": 1, "none": 0}
LOG_LEVEL = LOG_LEVELS["none"]
LOG_FILE_HANDLER = None

SIZE_PARAMETERS = {"4": "16", "5": "18", "6": "22", "7": "26", "8": "30"}
ASSIGNMENT_RESULTS_FILE = os.path.join(DATA_DIR, "assignment_cracked_passwords.txt")
TARGETS_BY_LENGTH = {
    4: ["f8340c836d41f77cd92708bbd5443cbe", "a5da39d04c817287740f53e6bdc13b5c", "5321abb3b57f535e0e3186b7a3204dff", "626e33f13574402935a30b2d200ebe53", "ff293b3c17785c6eafcc1d0f0615f445", "212549071c902ae28bc239ce6f1eec49", "c1fdbe6dc8eb0ed6aeea0e877ba836dd", "727578e6768a6750ada716f8dbb0e47c", "c77e40f417dc4ed074b1df3b916f85c9", "11cd649a72075e28384bce23c72c253d"],
    5: ["2a06b95a264d30c1282c0eacc8fd4eff", "f367b5557726a7a7e3c412e2e2b8b67d", "0b912f372ae901469920803051d95b6c", "b6bb4144918a257b256293b7dfe080ae", "7f150f7990a25b72ff2881afe4b88b62", "258f7ebad296c6b14dcd7d11fd21fb57", "30ac5f6e343fcfa96718c465e69ea0e0", "0593bd4d2a40db0ede8c55bf62ee8646", "801c580fd61a3a36feea0f027b990c22", "7f0398784a0692bd301804aeb9436d76"],
    6: ["02188670c5291c33fe5176f9e47b5576", "986247dad9d3ae53fa971e0d8531612c", "8453086621fac9a8e4db75708a053b48", "aa926e31b4a1d349e1e8e0687be19b99", "48051a930667ac6d0ff9234c4aaf8fe3", "caf8601b997161d9d8b467a24fd50204", "211862bfd5b5fdad68474edf8ce59d1c", "5eb0ccb1e0cd2a5a3a853223113e124f", "272f3fe7cb9ad55d2be1ecdb9c836f67", "f967085d623bd170584d4e80211f38b0"],
    7: ["89c6a9638646d4d53080a304f23c7f4e", "c3ae09893fe58f0f399561fa96a6cb41", "8b91e9967cf6d5182b2a2861e639d39b", "ffa76fa867d0d075f2f77d99741c283b", "69120c685003f05595208316ba4579b4", "92b180339869ae0e74ff71ee24364059", "8da0500ff759af3268e93fd73ebaa7bc", "d847d60611ed814c33e8f7275e43d8bc", "2aafba15313c0c80e02d928a33c50755", "904041327eae3675f36efaeda1f1d3c1"],
    8: ["6d4cc6104ee5752052905ae366de1788", "25563e8dd502e79f89c02433f011eb9b", "763ef3ff10f5954eed462610bae034b5", "e40a7d82d7d3606a564849809ea8e882", "2e22961e4b41067df857aba6e50fd78a", "04dc6ed7007be8f32f2c27ddbbd00d2a", "a5f9d6ba84524791063a50bb1ecfd0b0", "523c034fc8c58b7a6a1e182bb23e015c", "d771d6576c6aee69a9778fef4fb345bc", "368dc45539edd49052642ee6360dd768"]
}

# From https://www.asciiart.eu/animals/cats
CAT_ART = """  /\_/\  (
 ( ^.^ ) _)
   \"/  (
 ( | | )
(__d b__)


"""

console = Console()

# --- CORE LOGIC ---

def format_time(seconds_float):
    """Formats a float representing seconds into a human-readable string (e.g., 12.3s, 5m 30s, 2h 15m)."""
    if seconds_float < 60:
        return f"{seconds_float:.1f}s"
    minutes, seconds = divmod(int(seconds_float), 60)
    if minutes < 60:
        return f"{minutes}m {seconds}s"
    hours, minutes = divmod(minutes, 60)
    return f"{hours}h {minutes}m"

def log(message, level="info"):
    """Prints messages to the console with Rich formatting and optionally writes them to a log file."""
    global LOG_LEVEL, LOG_LEVELS, LOG_FILE_HANDLER
    message_level_map = {"info": 2, "error": 1, "success": 2}
    message_level = message_level_map.get(level, 2)

    if message_level <= LOG_LEVEL:
        console_message = message
        if level == "success":
            console_message = f"🎉 {message}"
        elif level == "error":
            console_message = f"[bold red]ERROR:[/] {message}"

        if level == "error":
            console.print(console_message)
        else:
            console.print(console_message)

        if LOG_FILE_HANDLER:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            log_entry = f"[{timestamp}] [{level.upper()}] {message}\n"
            LOG_FILE_HANDLER.write(log_entry)
            LOG_FILE_HANDLER.flush()

def run_command(command_list):
    """Executes a shell command, measures its execution time, and captures its standard output and error."""
    global LOG_LEVEL, LOG_LEVELS

    verbose_commands = list(command_list)
    if LOG_LEVEL == LOG_LEVELS["info"]:
        verbose_commands.append("--verbose")

    log(f"Running: {' '.join(verbose_commands)}", level="info")

    start_time = time.monotonic()
    try:
        process = subprocess.run(verbose_commands, capture_output=True, text=True, encoding='utf-8', check=False)
        elapsed_time = time.monotonic() - start_time

        if process.stdout:
            log(f"STDOUT:\n---\n{process.stdout.strip()}\n---", level="info")
        if process.stderr:
            log(f"STDERR:\n---\n{process.stderr.strip()}\n---", level="error")

        return process.stdout, elapsed_time

    except Exception as e:
        log(f"FATAL: {e}", level="error")
        sys.exit(1)

def crack_engine(hashes_to_crack, length_str, size_param, num_threads_str):
    """The main automated cracking engine. It iteratively generates rainbow tables and attempts to
    crack a given list of hashes until all are solved or no new hashes are found."""
    unsolved_hashes = list(hashes_to_crack)
    cracked_passwords = {}
    table_file = f"./bin/rainbow_l{length_str}_session.dat"
    num_chains = str(2**int(size_param))

    attempt = 1
    while unsolved_hashes:
        log(f"--- [Attempt #{attempt}] Cracking {len(unsolved_hashes)} remaining hash(es) of length {length_str} ---", level="info")

        table_cmd = [TABLE_EXECUTABLE, length_str, num_chains, table_file]
        if num_threads_str != "1": table_cmd.extend(["--threads", num_threads_str])
        _, table_time = run_command(table_cmd)
        log(f"Table generation finished in {format_time(table_time)}.", "info")

        newly_solved = []
        for a_hash in unsolved_hashes:
            log(f"-> Guessing hash: {a_hash}", level="info")
            guess_cmd = [GUESS_EXECUTABLE, table_file, a_hash]
            if num_threads_str != "1": guess_cmd.extend(["--threads", num_threads_str])
            output, guess_time = run_command(guess_cmd)
            log(f"Guess attempt finished in {format_time(guess_time)}.", "info")

            if "Password found" in output:
                match = re.search(r"Password found: (\S+)", output)
                if match:
                    password = match.group(1)
                    log(f"SUCCESS! Hash: {a_hash}, Password: {password}", level="success")
                    cracked_passwords[a_hash] = password
                    newly_solved.append(a_hash)

        if newly_solved:
            unsolved_hashes = [h for h in unsolved_hashes if h not in newly_solved]
        else:
            log("No new passwords found in this attempt. Regenerating table...", level="info")
        attempt += 1

    if os.path.exists(table_file): os.remove(table_file)
    return cracked_passwords

def handle_single_hash_target(num_threads_str):
    """Handles the cracking of a single user-provided hash. Prompts the user for the hash
    and its expected length, then uses the crack_engine to attempt recovery."""
    log("\n-- Target: Single Hash --", level="info")
    hash_to_crack = Prompt.ask("Enter the hash").strip().lower()
    length_str = Prompt.ask("Enter the password length").strip()
    size_param = SIZE_PARAMETERS.get(length_str)
    if not size_param: log(f"No default size for length {length_str}.", level="error"); return

    cracked = crack_engine([hash_to_crack], length_str, size_param, num_threads_str)
    if cracked:
        log("Cracking finished successfully.", "info")
    else:
        log("Could not crack the hash with the current configuration.", "info")

def handle_file_target(num_threads_str):
    """Handles the cracking of multiple hashes read from a specified file."""
    log("\n-- Target: Hashes from File --", level="info")
    filename = Prompt.ask(f"Enter filename from '{DATA_DIR}/'").strip()
    filepath = os.path.join(DATA_DIR, filename)
    if not os.path.exists(filepath): log(f"File not found.", level="error"); return
    length_str = Prompt.ask("Enter password length for all hashes in file").strip()
    size_param = SIZE_PARAMETERS.get(length_str)
    if not size_param: log(f"No default size for length {length_str}.", level="error"); return

    with open(filepath, 'r') as f:
        hashes = [line.strip().lower() for line in f if len(line.strip()) == 32]

    log(f"Found {len(hashes)} hashes to crack.", "info")
    results_file = os.path.join(DATA_DIR, f"cracked_{filename}")
    cracked = crack_engine(hashes, length_str, size_param, num_threads_str)

    if cracked:
        with open(results_file, 'a', encoding='utf-8') as f:
            for h, p in cracked.items(): f.write(f"{h} | {p}\n")
        log(f"Results appended to {results_file}", "info")

def handle_assignment_target(num_threads_str):
    """Handles the full assignment target, attempting to crack pre-defined hashes."""
    log("\n-- Target: Full Assignment --", level="info")

    try:
        with open(ASSIGNMENT_RESULTS_FILE, 'r') as f:
            solved_hashes = {line.split('|')[1].strip() for line in f if '|' in line}
    except FileNotFoundError: solved_hashes = set()
    log(f"Loaded {len(solved_hashes)} previously cracked assignment hashes.", level="info")

    for length, all_hashes in TARGETS_BY_LENGTH.items():
        length_str = str(length)
        size_param = SIZE_PARAMETERS.get(length_str)
        hashes_to_crack = [h for h in all_hashes if h not in solved_hashes]

        if not hashes_to_crack:
            log(f"All {length}-char hashes already solved. Skipping.", level="info")
            continue

        log(f"\n{'='*20} PROCESSING {len(hashes_to_crack)} UNCRACKED {length}-CHAR HASHES {'='*20}", level="info")
        category_start_time = time.monotonic()
        newly_cracked = crack_engine(hashes_to_crack, length_str, size_param, num_threads_str)
        category_elapsed_time = time.monotonic() - category_start_time
        log(f"Finished processing all {length}-char hashes in {format_time(category_elapsed_time)}.", "info")

        if newly_cracked:
            log(f"Appending {len(newly_cracked)} new results to {ASSIGNMENT_RESULTS_FILE}", "info")
            with open(ASSIGNMENT_RESULTS_FILE, 'a', encoding='utf-8') as f:
                for original_hash in all_hashes:
                    if original_hash in newly_cracked:
                        f.write(f"{length} | {original_hash} | {newly_cracked[original_hash]}\n")
            solved_hashes.update(newly_cracked.keys())

def run_mode(mode_name, num_threads_str):
    """Provides a menu for target selection (single hash, file of hashes, or full assignment)
    within a chosen execution mode (e.g., single-threaded, multi-threaded)."""
    while True:
        console.print(Panel(
            Text.from_markup(f"""
                            [bold cyan]Please choose a target:[/bold cyan]
                            [green]1.[/green] Crack a Single Hash
                            [green]2.[/green] Crack Hashes From a File
                            [green]3.[/green] Run Full Assignment
                            [yellow]4.[/yellow] Back to Main Menu
                        """),
            title=f"[bold green]{mode_name} Mode[/bold green]",
            border_style="green"
        ))
        choice = Prompt.ask("Enter target [1-4]")
        if choice == '1': handle_single_hash_target(num_threads_str)
        elif choice == '2': handle_file_target(num_threads_str)
        elif choice == '3': handle_assignment_target(num_threads_str)
        elif choice == '4': break
        else: log("Invalid choice.", level="error")
        if choice in ['1', '2', '3']:
            Prompt.ask("\nTarget run finished. Press Enter to continue...")

def parse_arguments():
    """Parses command-line arguments to set the log level."""
    global LOG_LEVEL, LOG_LEVELS
    for arg in sys.argv[1:]:
        if arg.startswith("--log="):
            level_str = arg.split('=', 1)[1].lower()
            if level_str in LOG_LEVELS:
                LOG_LEVEL = LOG_LEVELS[level_str]
            else:
                console.print(f"[bold red]ERROR:[/] Invalid log level '{level_str}'. Valid are: {', '.join(LOG_LEVELS.keys())}")
            return

def main():
    """Main entry point for the CryptoCat Cracker application."""
    global LOG_FILE_HANDLER
    parse_arguments()

    log_level_name = [k for k, v in LOG_LEVELS.items() if v == LOG_LEVEL][0]
    console.print(f"[Config] Log level is '[bold yellow]{log_level_name.upper()}[/bold yellow]'. Use --log=info for details.")

    if LOG_LEVEL == LOG_LEVELS["info"]:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        log_filename = os.path.join(DATA_DIR, f"cryptocat_run_{timestamp}.log")
        try:
            os.makedirs(DATA_DIR, exist_ok=True)
            LOG_FILE_HANDLER = open(log_filename, 'w', encoding='utf-8')
            console.print(f"[Config] Detailed output is being logged to: [cyan]{log_filename}[/cyan]")
        except IOError as e:
            console.print(f"[bold red]FATAL:[/] Could not open log file {log_filename}: {e}")
            sys.exit(1)

    if not all(os.path.exists(p) for p in [TABLE_EXECUTABLE, GUESS_EXECUTABLE]):
        log(f"Make sure C++ executables exist. Run 'make'.", level="error"); return
    os.makedirs("./bin", exist_ok=True)

    try:
        while True:
            console.print("\n")
            title_art = pyfiglet.figlet_format("CryptoCat", font="slant")
            banner = Columns([
                Text(title_art, style="bold green", justify="left"),
                Text(CAT_ART, style="bold green"),
            ])

            console.print(banner)
            console.print(Panel(
                Text.from_markup("""
                    [bold cyan]Please choose an execution mode:[/bold cyan]
                    [green]1.[/green] Single-Threaded Mode
                    [green]2.[/green] Multi-Threaded Mode
                    [yellow]3.[/yellow] Kubernetes Mode (Coming Soon)
                    [red]4.[/red] Exit
                """),
                title="[bold magenta]Execution Mode[/bold magenta]",
                border_style="magenta"
            ))

            choice = Prompt.ask("Enter your choice [1-4]")
            if choice == '1':
                run_mode("Single-Threaded", "1")
            elif choice == '2':
                try:
                    cpu_count = os.cpu_count() or 1
                    num_threads = Prompt.ask(f"Enter number of threads (1-{cpu_count} rec)")
                    if int(num_threads) > 0:
                        run_mode(f"Multi-Threaded ({num_threads} Threads)", num_threads)
                    else: log("Must be a positive number.", level="error")
                except (ValueError, IndexError):
                    log("Invalid number.", level="error")
            elif choice == '3':
                console.print("\n[yellow]Kubernetes mode is not yet implemented.[/yellow]")
            elif choice == '4':
                console.print("Exiting program.")
                break
            else:
                log("Invalid choice.", level="error")
    finally:
        if LOG_FILE_HANDLER:
            log("Closing log file.", "info")
            LOG_FILE_HANDLER.close()

if __name__ == "__main__":
    main()