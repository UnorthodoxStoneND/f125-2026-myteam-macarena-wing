"""Build the native addon and run its actual-DLL tests (Visual Studio C++ x64)."""
from pathlib import Path
from hashlib import sha256
import argparse,importlib.util,json,os,subprocess

ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
mode=parser.add_mutually_exclusive_group()
mode.add_argument('--build-only',action='store_true',help='Build shared CI artifacts without running tests')
mode.add_argument('--tests-only',action='store_true',help='Run the already-built actual-DLL fixture')
args=parser.parse_args()
OUT=ROOT/'.verify/native';OUT.mkdir(parents=True,exist_ok=True)
(ROOT/'bin').mkdir(exist_ok=True)
plugin=ROOT/'bin/myteam-wing-animation.dll';fixture=OUT/'wing-fixture.dll';runner=OUT/'test-native.exe'

if not args.tests_only:
    spec=importlib.util.spec_from_file_location('prepare_deps',ROOT/'tools/checks/prepare_deps.py')
    deps=importlib.util.module_from_spec(spec);spec.loader.exec_module(deps);deps.prepare()
    hook=ROOT/'.deps/safetyhook';sdk=ROOT/'native/sdk'
    vswhere=Path(os.environ.get('ProgramFiles(x86)',r'C:\Program Files (x86)'))/'Microsoft Visual Studio/Installer/vswhere.exe'
    install=subprocess.check_output([str(vswhere),'-latest','-products','*','-requires',
        'Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip()
    if not install:raise SystemExit('Install Visual Studio Build Tools with Desktop development with C++.')
    dev=Path(install)/'Common7/Tools/VsDevCmd.bat'
    result=subprocess.run(f'cmd.exe /d /s /c "call "{dev}" -arch=x64 -host_arch=x64 >nul && set"',
                          capture_output=True,text=True,check=True)
    env={k.upper():v for k,v in os.environ.items()}
    env.update((line.split('=',1)[0].upper(),line.split('=',1)[1]) for line in result.stdout.splitlines() if '=' in line and not line.startswith('='))
    cl=Path(env['VCTOOLSINSTALLDIR'])/'bin/Hostx64/x64/cl.exe'
    flags=[str(cl),'/nologo','/std:c++latest','/EHsc','/O2','/MT','/W4','/I'+str(sdk),'/I'+str(hook),'/I'+str(ROOT/'native')]
    def run(command):subprocess.run(command,env=env,cwd=OUT,check=True)
    run(flags+['/c',str(hook/'safetyhook.cpp'),'/Fo'+str(OUT/'safetyhook.obj')])
    run([str(cl),'/nologo','/O2','/MT','/c',str(hook/'Zydis.c'),'/I'+str(hook),'/Fo'+str(OUT/'Zydis.obj')])
    for test,dst in [(False,plugin),(True,fixture)]:
        stem='wing-fixture' if test else 'myteam-wing-animation'
        run(flags+(['/DWING_TEST'] if test else [])+['/LD',str(ROOT/'native/wing.cpp'),'/Fo'+str(OUT/(stem+'.obj')),
            str(OUT/'safetyhook.obj'),str(OUT/'Zydis.obj'),'/link','/Brepro','bcrypt.lib','/OUT:'+str(dst),'/IMPLIB:'+str(OUT/(stem+'.lib'))])
    run(flags+[str(ROOT/'native/test_native.cpp'),'/Fo'+str(OUT/'test-native.obj'),'/link','/Brepro','/OUT:'+str(runner)])

if not args.build_only:
    test=subprocess.run([str(runner),str(fixture),str(plugin)],cwd=OUT,capture_output=True,text=True,timeout=30)
    print(test.stdout);print(test.stderr);test.check_returncode()
    report={'status':'pass','actual_dll':True,'release_sha256':sha256(plugin.read_bytes()).hexdigest(),
            'output':test.stdout.strip(),'in_game_validation':'Offline host only; record game results separately.'}
    (OUT/'validation-native.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
