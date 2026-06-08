import os
import signal
import sys

print("Content-Type: text/html\r\n\r\n", end="")
print("Je vais recevoir un signal SIGSEGV...")
sys.stdout.flush()
os.kill(os.getpid(), signal.SIGSEGV);