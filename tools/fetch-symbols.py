"""Fetch the exact Microsoft PDB for local shell modules, without loading it."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import struct
import urllib.request
import uuid

parser = argparse.ArgumentParser()
parser.add_argument('--cache', required=True, type=Path)
parser.add_argument('--source', required=True, type=Path)
args = parser.parse_args()
args.cache.mkdir(parents=True, exist_ok=True)
for binary in [Path(os.environ['SystemRoot']) / 'System32/twinui.pcshell.dll',
               Path(os.environ['SystemRoot']) / 'explorer.exe']:
    data = binary.read_bytes()
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    sections = struct.unpack_from('<H', data, pe + 6)[0]
    opt_size = struct.unpack_from('<H', data, pe + 20)[0]
    opt = pe + 24
    if struct.unpack_from('<H', data, opt)[0] != 0x20b:
        raise RuntimeError('This checker requires x64 Windows.')
    debug_rva, debug_size = struct.unpack_from('<II', data, opt + 112 + 6 * 8)

    def offset(rva):
        for n in range(sections):
            at = opt + opt_size + n * 40
            vsize, va, size, raw = struct.unpack_from('<IIII', data, at + 8)
            if va <= rva < va + max(vsize, size):
                return raw + rva - va
        raise ValueError('RVA outside PE sections')

    for at in range(offset(debug_rva), offset(debug_rva) + debug_size, 28):
        _, _, _, _, kind, size, _, raw = struct.unpack_from('<IIHHIIII', data, at)
        if kind != 2 or data[raw:raw+4] != b'RSDS':
            continue
        guid = uuid.UUID(bytes_le=data[raw+4:raw+20]).hex.upper()
        age = struct.unpack_from('<I', data, raw + 20)[0]
        name = data[raw+24:raw+size].split(b'\0')[0].decode().replace('\\', '/').split('/')[-1]
        key = f'{guid}{age:X}'
        target_dir = args.cache / name / key
        target_dir.mkdir(parents=True, exist_ok=True)
        target = target_dir / name
        url = f'https://msdl.microsoft.com/download/symbols/{name}/{key}/{name}'
        print(f'BINARY: {binary}\nSHA256: {hashlib.sha256(data).hexdigest()}\nPDB: {url}', flush=True)
        if not target.exists():
            temporary = target.with_suffix('.download')
            with urllib.request.urlopen(url, timeout=45) as response, temporary.open('wb') as output:
                while chunk := response.read(1024 * 1024):
                    output.write(chunk)
            temporary.replace(target)
        break
    else:
        raise RuntimeError('No PDB record found')

source = args.source.read_text(encoding='utf-8')
win11 = source.split('if (g_winVersion == WinVersion::Win11) {', 1)[1].split('} else if (g_winVersion', 1)[0]
start = source.split('namespace StartMenu {', 1)[1]
# Optional legacy Alt+Tab hooks are deliberately excluded from the native Win11 check.
win11 = win11.split('// For the old Win10', 1)[0]
symbols = re.findall(r'LR"\(([^\n]*?)\)"', win11 + start)
if len(symbols) != 14:
    raise RuntimeError(f'Expected 14 required Win11 symbols, got {len(symbols)}; update the checker.')
(args.cache / 'required.txt').write_text('\n'.join(symbols) + '\n', encoding='utf-8')
print(f'Ready: {len(symbols)} required symbols across two modules.', flush=True)
