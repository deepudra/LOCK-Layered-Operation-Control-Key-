from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer
import argparse
import shutil
import os

custom_dir = "ftp_C_drive"
windows_structure = [
    "Program Files",
    "Program Files (x86)",
    "tmp",
    "Windows",
    "PerfLogs",
    "Users\\Admin\\Documents",
    "Users\\Admin\\Downloads",
    "Users\\Admin\\Desktop"
]

fake_files = {
    "Program Files": ["app1.exe", "config.ini"],
    "Program Files (x86)": ["app2.exe", "settings.json"],
    "tmp": ["temp1.tmp", "temp2.log"],
    "Windows": ["system32.dll", "explorer.exe"],
    "PerfLogs": ["log1.txt", "log2.txt"],
    "Users\\Admin\\Documents": ["resume.docx", "notes.txt"],
    "Users\\Admin\\Downloads": ["installer.msi", "game.zip"],
    "Users\\Admin\\Desktop": ["shortcut.lnk", "readme.txt"]
}

shortcut_command = "powershell -Command \"$ws = New-Object -ComObject WScript.Shell; $ws.Popup('You are hacked !', 5, 'Hacker', 64)\""

def clear_directory(directory):
    if os.path.exists(directory):
        shutil.rmtree(directory)
    os.makedirs(directory)

def create_custom_files(base_dir):
    for folder in windows_structure:
        folder_path = os.path.join(base_dir, folder)
        os.makedirs(folder_path, exist_ok=True)
        if folder in fake_files:
            for file_name in fake_files[folder]:
                with open(os.path.join(folder_path, file_name), "w") as f:
                    f.write(f"Fake content of {file_name}")

def start_ftp_server(username, password, port):
    create_custom_files(custom_dir)
    authorizer = DummyAuthorizer()
    authorizer.add_user(username, password, custom_dir, perm="elradfmw")
    authorizer.add_anonymous(custom_dir, perm="elr")

    handler = FTPHandler
    handler.authorizer = authorizer
    server = FTPServer(("0.0.0.0", port), handler)
    
    print(f"FTP Server started on port {port} with user '{username}'")
    server.serve_forever()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Start a custom FTP server.")
    parser.add_argument("--username", type=str, required=True, help="FTP username")
    parser.add_argument("--password", type=str, required=True, help="FTP password")
    parser.add_argument("--port", type=int, required=True, help="FTP server port")
    args = parser.parse_args()
    start_ftp_server(args.username, args.password, args.port)