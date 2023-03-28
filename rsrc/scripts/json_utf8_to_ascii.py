#!/usr/bin/env python3
# Author: Sean Pesce

import json
import sys


def json_convert_utf8_to_ascii(in_fpath, out_fpath=None, include_encoding=False):
    b = b''
    with open(in_fpath, 'rb') as f:
        b = f.read()
    j = json.loads(b)
    if include_encoding:
        j['encoding'] = 'ascii'  # If you want to include encoding metadata
    if out_fpath is not None:
        with open(out_fpath, 'w') as f:
            json.dump(j, f, indent=2, ensure_ascii=True)
    else:
        print(json.dumps(j, indent=2, ensure_ascii=True))
    return j


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f'Usage:\n\t{sys.argv[0]} <UTF-8 JSON input file> [ASCII JSON output file]')
        sys.exit()
    
    OUT_FILE = None
    if len(sys.argv) > 2:
        OUT_FILE = sys.argv[2]
    json_convert_utf8_to_ascii(sys.argv[1], out_fpath=OUT_FILE, include_encoding=True)
