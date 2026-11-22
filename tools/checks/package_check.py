"""Check archive contents, hashes, manifest references and accidental private paths."""
from pathlib import Path
from hashlib import sha256
import json,struct,zipfile

ROOT=Path(__file__).resolve().parents[2];dist=ROOT/'dist'
manifest=json.loads((ROOT/'swofty.json').read_text(encoding='utf-8'))
expected=json.loads((dist/'PACKAGE_CONTENTS.json').read_text(encoding='utf-8'))
archive=dist/('MyTeam-Macarena-RedBull-Style-'+manifest['version']+'.zip')
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    assert set(z.namelist())==set(expected)
    assert json.loads(z.read('swofty.json'))==manifest
    assert all(not n.endswith(('.cpp','.hpp','.h','.obj','.pdb','.py','.exe','.svg')) for n in z.namelist())
    assert z.read('native/myteam-wing-animation.dll')==(ROOT/'bin/myteam-wing-animation.dll').read_bytes()
    for entry in manifest['operations']['native']:assert entry['dll'] in z.namelist()
    png=z.read(manifest['icon']);assert png[:8]==b'\x89PNG\r\n\x1a\n' and struct.unpack('>II',png[16:24])==(512,512)
    for name in z.namelist():
        if name.endswith(('.json','.md','.txt')):
            data=z.read(name).lower()
            assert b'c:/users/' not in data and b'c:\\users\\' not in data,name
for line in (dist/'SHA256SUMS.txt').read_text(encoding='utf-8').splitlines():
    digest,name=line.split('  ',1);assert sha256((dist/name).read_bytes()).hexdigest()==digest
assert archive.read_bytes()==archive.with_suffix('.swoftyaddon').read_bytes()
print('PASS: explicit package contents, references, PNG, compiled DLL, versions, hashes and private-path checks.')
