#!/usr/bin/env python3

import base64
import sys

def swap_odd_even_positions(text):
    text_list = list(text)

    for i in range(0, len(text_list) - 1, 2):
        text_list[i], text_list[i + 1] = text_list[i + 1], text_list[i]

    return ''.join(text_list)

def main():
    if len(sys.argv) < 2:
        print("usage: base64-swap.py file-name")
        return

    input_filename = sys.argv[1]

    try:
        with open(input_filename, 'r') as input_file:
            text = input_file.read()
            encoded_text = base64.b64encode(text.encode()).decode()

            # 如果 Base64 结果长度为奇数，在最后补充一个空格
            if len(encoded_text) % 2 == 1:
                encoded_text += ' '

            swapped_encoded_text = swap_odd_even_positions(encoded_text)

            print(swapped_encoded_text, end='')

    except FileNotFoundError:
        print("file not found")
    except Exception as e:
        print("error:", e)

if __name__ == "__main__":
    main()

