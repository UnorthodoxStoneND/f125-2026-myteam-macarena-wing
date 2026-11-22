"""Fetch the hash-pinned upstream hook sources into ignored .deps/."""
from pathlib import Path
from hashlib import sha256
import json,urllib.request

ROOT=Path(__file__).resolve().parents[2]
def prepare():
    lock=json.loads((ROOT/'tools/checks/dependencies.json').read_text(encoding='utf-8'))
    for f in lock['files']:
        target=(ROOT/f['target']).resolve()
        if not target.is_relative_to((ROOT/'.deps').resolve()):raise ValueError('Dependency target must stay in .deps')
        if target.is_file() and sha256(target.read_bytes()).hexdigest()==f['sha256']:continue
        url=f"https://raw.githubusercontent.com/{lock['repository']}/{lock['commit']}/{f['path']}"
        print('Fetching',f['path'],flush=True)
        with urllib.request.urlopen(url,timeout=60) as response:data=response.read(32*1024*1024)
        if sha256(data).hexdigest()!=f['sha256']:raise ValueError('Upstream dependency checksum mismatch: '+f['path'])
        target.parent.mkdir(parents=True,exist_ok=True)
        temp=target.with_suffix(target.suffix+'.download');temp.write_bytes(data);temp.replace(target)
    return lock

if __name__=='__main__':
    lock=prepare();print('Dependencies verified:',lock['tag'],lock['commit'])
