import base64
import re

def extract():
    with open('data/index.html', 'r', encoding='utf-8') as f:
        html = f.read()
    
    match = re.search(r'data:image/png;base64,([^"\'\s>]+)', html)
    if match:
        img_data = base64.b64decode(match.group(1))
        with open('logo.png', 'wb') as f:
            f.write(img_data)
        print("Successfully extracted to logo.png")
    else:
        print("Image not found in index.html")

if __name__ == '__main__':
    extract()