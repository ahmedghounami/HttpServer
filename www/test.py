import sys
import time

# Set content type header
sys.stdout.write("Content-Type: image/png\r\n\r\n")
sys.stdout.flush()

# Send the file content (binary)
with open("/Users/mkibous/Desktop/webserver/www/image2.png", "rb") as f:
    sys.stdout.buffer.write(f.read())
    sys.stdout.flush()

# Infinite loop (optional)
# while True:
#     time.sleep(1)
