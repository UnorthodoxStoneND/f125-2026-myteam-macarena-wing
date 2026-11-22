"""Create the installable archive using an explicit, documented runtime allowlist."""
from pathlib import Path
from hashlib import sha256
import argparse,json,zipfile

ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output',type=Path,default=ROOT/'dist')
args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'swofty.json').read_text(encoding='utf-8'))
assert manifest['id']=='unorthodoxstonendmodding.myteam-macarena-wing'
assert manifest['icon']=='icon.png'
plugin=ROOT/'bin/myteam-wing-animation.dll'
report=json.loads((ROOT/'.verify/native/validation-native.json').read_text(encoding='utf-8'))
assert report['status']=='pass' and report['release_sha256']==sha256(plugin.read_bytes()).hexdigest(),'Build/test this DLL before packaging.'
contents={
 'swofty.json':(ROOT/'swofty.json','addon manifest and settings'),
 'icon.png':(ROOT/'icon.png','manifest icon'),
 'native/myteam-wing-animation.dll':(plugin,'manifest operations.native'),
 'registry/handling@1.1.json':(ROOT/'registry/handling@1.1.json','vehicle.handling registry records'),
 'README.md':(ROOT/'docs/RELEASE_README.md','installation, compatibility and third-party credits'),
 'CHANGELOG.md':(ROOT/'CHANGELOG.md','release history'),
 'LICENSE':(ROOT/'LICENSE','original-code licence'),
 'THIRD_PARTY_NOTICES.md':(ROOT/'THIRD_PARTY_NOTICES.md','third-party licence scope and attribution'),
 'licenses/SafetyHook-Boost.txt':(ROOT/'licenses/SafetyHook-Boost.txt','required bundled-library licence'),
 'licenses/Zydis-MIT.txt':(ROOT/'licenses/Zydis-MIT.txt','required bundled-library licence'),
 'licenses/Swofty-PolyForm-Noncommercial.txt':(ROOT/'licenses/Swofty-PolyForm-Noncommercial.txt','vendored JSON reader licence'),
 'licenses/Swofty-SDK-MIT.txt':(ROOT/'native/SDK-LICENSE.txt','Swofty SDK licence'),
}
name='MyTeam-Macarena-RedBull-Style-'+manifest['version']
archive=args.output/(name+'.zip')
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
    for rel,(src,_) in sorted(contents.items()):
        info=zipfile.ZipInfo(rel,(2026,11,22,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o644<<16
        z.writestr(info,src.read_bytes())
addon=args.output/(name+'.swoftyaddon');addon.write_bytes(archive.read_bytes())
checks=[]
for p in [archive,addon]:
    checksum=sha256(p.read_bytes()).hexdigest()+'  '+p.name+'\n'
    p.with_suffix(p.suffix+'.sha256').write_text(checksum,encoding='utf-8');checks.append(checksum)
(args.output/'SHA256SUMS.txt').write_text(''.join(checks),encoding='utf-8')
(args.output/'PACKAGE_CONTENTS.json').write_text(json.dumps({k:v[1] for k,v in contents.items()},indent=2)+'\n',encoding='utf-8')
print(archive);print(addon)
