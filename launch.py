#!/bin/python

import subprocess
import threading
import time
import os
import argparse

def run_process(command: list[str]):
    name = os.path.basename(command[0])
    if command[1] == '':
        command.pop()
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    while True:
        output = process.stdout.readline()
        if output == '' and process.poll() is not None:
            break
        if output:
            print('\033[33m' + name + '\033[0m ' + output.strip())
    rc = process.poll()
    return rc

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--no_server", help="Do not start the server",
                    action="store_true")
    args = parser.parse_args()

    commands = [
        ['build/linux/bin/npnameserver', ''],
        ['build/linux/bin/npdbserver', '']
    ]

    if not args.no_server:
        commands.append(['build/linux/bin/npserver', ''])

    threads = []
    for command in commands:
        thread = threading.Thread(target=run_process, args=(command,))
        thread.start()
        threads.append(thread)
        time.sleep(2)

    for thread in threads:
        thread.join()

if __name__ == '__main__':
    main()
