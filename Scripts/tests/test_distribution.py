# SPDX-License-Identifier: CC0-1.0
import copy
import json
import pathlib
import struct
import tempfile
import unittest
import warnings
import zipfile
from Scripts.verify_distribution import (EDITIONS, digest, expand_reference, includes, markdown_links,
                           node_example_adapter, validate_archive)


class DistributionTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self.directory.name) / 'root'
        self.root.mkdir()
        self.libraries = ['DataStructure/Node.cpp', 'Math/Foo.cpp', 'Math/Bar.cpp']
        self.examples = {'node_example.cpp': self.libraries[0], 'math_example.cpp': self.libraries[1]}
        self.node_example = '#include "../DataStructure/Node.cpp"\nint main(){auto entry=make_node(); auto raw=entry.get();}\n// 所有は残る\n// 共有所有の循環を切ってから解放する\n'
        self.math_example = '#include "../Math/Foo.cpp"\n#include <atcoder/modint>\nint main(){return bar;}\n'
        self.bar = '#ifndef BAR\n#define BAR\nint bar=3;\n#endif\n'
        self.foo = '#include "Bar.cpp"\nint foo=4;\n'
        self.write('Scripts/package_revised_libraries.py', 'LIBRARIES='+repr(self.examples)+'\n')
        self.write('Scripts/library_layout.json', json.dumps({'DataStructure':['Node.cpp'],'Math':['Foo.cpp','Bar.cpp']}))
        self.write(self.libraries[0], '// NORMAL\nstruct Node{};\n')
        self.write(self.libraries[1], self.foo)
        self.write(self.libraries[2], self.bar)
        self.write('Examples/node_example.cpp', self.node_example)
        self.write('Examples/math_example.cpp', self.math_example)
        self.write('Verify/current/vendor/atcoder/modint.hpp', '#ifndef ACL_M\n#define ACL_M\n#include "atcoder/internal_math"\nint acl_mod=7;\n#endif\n')
        self.write('Verify/current/vendor/atcoder/internal_math.hpp', '#ifndef ACL_I\n#define ACL_I\nint acl_internal=9;\n#endif\n')
        self.write('Readme.md', '# fixture\n[Docs](Document/README.md)\n')
        self.write('Document/README.md', '[Foo](../Math/Foo.cpp)\n[Audit](../Audit/evidence.json)\n')
        self.write('Audit/evidence.json', '{}\n')
        self.write('AGENTS.md', 'fixture\n'); self.write('LICENSE','CC0\n')
        self.write('temp.cpp','int main(){}\n'); self.write('temp2.cpp','int main(){}\n')
        self.normal_driver='Verify/current/drivers/node_main.cpp'; self.write(self.normal_driver,'int main(){auto entry=make_node();auto p=entry.get();}\n')
        self.fast_driver='Fast/Drivers/node_main.cpp';self.write(self.fast_driver,'int main(){auto entry=make_node();auto p=entry;}\n')
        normal_code = self.read(self.libraries[0]) + self.read(self.normal_driver)
        self.write('Verify/current/submissions/node.cpp', normal_code)
        self.normal=[dict(id='node',libraries=[self.libraries[0]],source_hashes={self.libraries[0]:digest(self.read(self.libraries[0]).encode())},driver=self.normal_driver,driver_sha256=digest(self.read(self.normal_driver).encode()),submission='Verify/current/submissions/node.cpp',submission_sha256=digest(normal_code.encode()),bytes=len(normal_code.encode()))]
        variants=[];verifies=[]
        for edition,(compiler,fast) in list(EDITIONS.items())[1:]:
            prefix=('Fast/'if fast else'Variants/')+compiler
            for source in self.libraries:
                data=(f'// {edition}\n'+self.read(source)).encode();self.write(prefix+'/'+source,data)
                variants.append(dict(source=source,compiler=compiler,fast=fast,path=prefix+'/'+source,sha256=digest(data),source_sha256=digest(self.read(source).encode()),bytes=len(data)))
            driver=self.fast_driver if fast else self.normal_driver
            data=(self.read(prefix+'/'+self.libraries[0])+self.read(driver)).encode();target=prefix+'/Verify/current/submissions/node.cpp';self.write(target,data)
            verifies.append(dict(id='node',compiler=compiler,fast=fast,path=target,sha256=digest(data),bytes=len(data),source_paths={self.libraries[0]:prefix+'/'+self.libraries[0]},source_hashes={self.libraries[0]:digest(self.read(prefix+'/'+self.libraries[0]).encode())},driver=driver,normal_driver=self.normal_driver,driver_adapted=fast,driver_sha256=digest(self.read(driver).encode())))
        self.write('Variants/manifest.json',json.dumps(dict(entries=variants)))
        self.write('Variants/verify_manifest.json',json.dumps(dict(entries=verifies)))
        self.write('Verify/current/manifest.json',json.dumps(dict(entries=self.normal)))

    def tearDown(self):
        self.directory.cleanup()

    def write(self,name,data):
        target=self.root/name;target.parent.mkdir(parents=True,exist_ok=True)
        target.write_bytes(data if isinstance(data,bytes)else data.encode())

    def read(self,name):
        return (self.root/name).read_text()

    def contents(self,edition):
        variants=json.loads(self.read('Variants/manifest.json'))['entries'];verifies=json.loads(self.read('Variants/verify_manifest.json'))['entries']
        if edition=='full':
            contents={p.relative_to(self.root).as_posix():p.read_bytes()for p in self.root.rglob('*')if p.is_file()}
            sources={n:self.read(n)for n in self.libraries}
        else:
            compiler,fast=EDITIONS[edition]
            contents={e['source']:(self.root/e['path']).read_bytes()for e in variants if(e['compiler'],e['fast'])==(compiler,fast)}
            sources={n:contents[n].decode()for n in self.libraries}
            selected=[]
            for entry in verifies:
                if(entry['compiler'],entry['fast'])!=(compiler,fast):continue
                record=copy.deepcopy(entry);record['path']='Verify/current/submissions/'+entry['id']+'.cpp';selected.append(record);contents[record['path']]=(self.root/entry['path']).read_bytes()
            contents['Verify/current/manifest.json']=json.dumps(dict(entries=selected)).encode()
            for name in ['Readme.md','Document/README.md','Audit/evidence.json','LICENSE']:contents[name]=(self.root/name).read_bytes()
        for name,library in self.examples.items():
            source=self.read('Examples/'+name)
            if edition.startswith('fast_') and library==self.libraries[0]:
                source=source.replace('entry.get()','entry').replace('所有は残る','参照は残る').replace('共有所有の循環を切ってから解放する','循環を切る。メモリは解放しない')
            contents['Examples/'+name]=source.encode()
            if library==self.libraries[0]:
                standalone=source.replace('#include "../'+library+'"',sources[library])
            else:
                body=sources[library].replace('#include "Bar.cpp"',sources['Math/Bar.cpp'])
                acl=self.read('Verify/current/vendor/atcoder/modint.hpp').replace('#include "atcoder/internal_math"','// ACL: atcoder/internal_math\n'+self.read('Verify/current/vendor/atcoder/internal_math.hpp'))
                standalone=source.replace('#include "../'+library+'"',body).replace('#include <atcoder/modint>','// ACL: atcoder/modint\n'+acl)
            contents['StandaloneExamples/'+name]=standalone.encode()
        return contents

    def archive(self,edition='gcc',change=None,extra=None):
        entries=self.contents(edition)
        if change:change(entries)
        target=pathlib.Path(self.directory.name)/'fixture.zip'
        with warnings.catch_warnings():
            warnings.simplefilter('ignore',UserWarning)
            with zipfile.ZipFile(target,'w',compression=zipfile.ZIP_STORED)as z:
                for name,data in entries.items():z.writestr(name,data)
                for name,data in(extra or[]):z.writestr(name,data)
        return target

    def assert_error(self,code,edition='gcc',change=None,extra=None):
        report=validate_archive(self.root,self.archive(edition,change,extra),edition)
        self.assertFalse(report['ok']);self.assertIn(code,{e['code']for e in report['errors']},report['errors']);return report

    def test_all_five_editions(self):
        for edition in EDITIONS:
            with self.subTest(edition=edition):
                report=validate_archive(self.root,self.archive(edition),edition);self.assertTrue(report['ok'],report['errors'])

    def test_source_wrong_edition(self):
        self.assert_error('edition_source_sha',change=lambda c:c.__setitem__(self.libraries[0],self.read(self.libraries[0]).encode()))

    def test_source_missing(self):
        self.assert_error('missing_entry',change=lambda c:c.pop(self.libraries[0]))

    def test_submission_wrong_bytes(self):
        self.assert_error('verify_sha',change=lambda c:c.__setitem__('Verify/current/submissions/node.cpp',b'bad'))

    def test_submission_limit(self):
        self.assert_error('verify_size',change=lambda c:c.__setitem__('Verify/current/submissions/node.cpp',b'x'*(512*1024+1)))

    def test_verify_exact_limit(self):
        report=self.assert_error('verify_sha',change=lambda c:c.__setitem__('Verify/current/submissions/node.cpp',b'x'*(512*1024)))
        self.assertNotIn('verify_size',{e['code']for e in report['errors']})

    def test_missing_example_dep(self):
        self.assert_error('example_dependency',change=lambda c:c.__setitem__('Examples/node_example.cpp',b'#include "../missing.cpp"\n'))

    def test_standalone_unexpanded_own(self):
        self.assert_error('standalone_unexpanded',change=lambda c:c.__setitem__('StandaloneExamples/node_example.cpp',self.node_example.encode()))

    def test_standalone_unexpanded_acl(self):
        self.assert_error('standalone_unexpanded',change=lambda c:c.__setitem__('StandaloneExamples/math_example.cpp',b'#include <atcoder/modint>\n'))

    def test_standalone_wrong_acl(self):
        self.assert_error('standalone_source_sync',change=lambda c:c.__setitem__('StandaloneExamples/math_example.cpp',c['StandaloneExamples/math_example.cpp'].replace(b'acl_mod=7',b'acl_mod=8')))

    def test_fast_node_adapter(self):
        self.assert_error('example_edition_adapter','fast_gcc',change=lambda c:c.__setitem__('Examples/node_example.cpp',self.node_example.encode()))

    def test_adapter_keeps_literal(self):
        text='const char*s="entry.get()";\n// entry.get()\nint main(){auto p=entry.get();}\n'
        self.assertEqual(node_example_adapter(text),'const char*s="entry.get()";\n// entry.get()\nint main(){auto p=entry;}\n')

    def test_missing_markdown(self):
        self.assert_error('broken_markdown_link',change=lambda c:c.pop('Audit/evidence.json'))

    def test_unknown_verify_id(self):
        self.assert_error('manifest_verify_ids',change=lambda c:c.__setitem__('Verify/current/manifest.json',b'{"entries":[]}'))

    def test_manifest_parse(self):
        self.assert_error('manifest_parse',change=lambda c:c.__setitem__('Verify/current/manifest.json',b'not json'))

    def test_flat_manifest_repaired_source_paths(self):
        def repair(c):
            d=json.loads(c['Verify/current/manifest.json']);d['entries'][0]['source_paths']={self.libraries[0]:self.libraries[0]};c['Verify/current/manifest.json']=json.dumps(d).encode()
        report=validate_archive(self.root,self.archive(change=repair),'gcc');self.assertTrue(report['ok'],report['errors']);self.assertEqual(report['warnings'],[])

    def test_current_manifest_library_mismatch(self):
        target=self.root/'Variants/GCC/DataStructure/Node.cpp';target.write_text('changed')
        self.assert_error('current_manifest_source_sha')

    def test_current_verify_library_mismatch(self):
        d=json.loads(self.read('Variants/verify_manifest.json'));d['entries'][0]['source_hashes'][self.libraries[0]]='0'*64;self.write('Variants/verify_manifest.json',json.dumps(d));self.assert_error('current_verify_library_sha')

    def test_unsafe_paths(self):
        for name in ['../bad.cpp','/abs.cpp','a//b','a/./b','C:/x','a\\b','a/../../x']:
            with self.subTest(path=name):self.assert_error('unsafe_entry',extra=[(name,b'bad')])

    def test_duplicate_path(self):
        self.assert_error('duplicate_entry',extra=[('Readme.md',b'duplicate')])

    def test_casefold_collision(self):
        self.assert_error('casefold_collision',extra=[('readme.md',b'case')])

    def test_file_directory_collision(self):
        self.assert_error('file_directory_collision',extra=[('collision',b'file'),('collision/child',b'child')])

    def test_symlink(self):
        name=zipfile.ZipInfo('link');name.create_system=3;name.external_attr=(0o120777<<16)
        self.assert_error('symlink_entry',extra=[(name,b'../../escape')])

    def test_crc(self):
        path=self.archive();raw=bytearray(path.read_bytes())
        with zipfile.ZipFile(path)as z:info=z.getinfo(self.libraries[0]);at=info.header_offset;name_len,extra_len=struct.unpack_from('<HH',raw,at+26);data_at=at+30+name_len+extra_len
        raw[data_at]^=1;path.write_bytes(raw);report=validate_archive(self.root,path,'gcc');self.assertIn('crc_or_read',{e['code']for e in report['errors']})

    def test_budget(self):
        report=validate_archive(self.root,self.archive(),'gcc',max_total=1);self.assertIn('archive_budget',{e['code']for e in report['errors']})

    def test_comment_raw_string_and_splice_include(self):
        text='/* #include "wrong" */\nconst char*s=R"d(\n#include "bad"\n)d";\n// ignored \\\n#include "wrong"\n#include \\\n"right"\n'
        self.assertEqual([e['name']for e in includes(text)],['right'])

    def test_hash_inside_leading_comment(self):
        text='/* # ignored */ #include "real"\n'
        directive=includes(text)[0]
        self.assertEqual(text[directive['begin']:directive['end']], '#include "real"')

    def test_digit_separator(self):
        text='int x=1\'000\'000;\n#include "right"\n'
        self.assertEqual([e['name']for e in includes(text)],['right'])

    def test_reference_cycles(self):
        with self.assertRaises(ValueError):expand_reference('#include "a"','example',{'a'},lambda n:'#include "a"',lambda n:'')

    def test_markdown_nested_parentheses_and_fences(self):
        source='[P](../Math/prime(Miller).cpp)\n[A](<file with space.md>)\n`[ignore](missing)`\n```cpp\n[x](also_missing)\n```\n[id]: here.md\n'
        self.assertEqual(markdown_links(source),['../Math/prime(Miller).cpp','file with space.md','here.md'])



from Scripts.flattened_markdown_repair import repair_flattened_markdown, verify_index
class RepairTests(DistributionTests):
    def test_missing_audit_relinked_and_marked(self):
        c=self.contents('gcc');c.pop('Audit/evidence.json');changes=repair_flattened_markdown(c,self.root);self.assertEqual(len(changes),1);text=c['Document/README.md'].decode();self.assertIn('（元リポジトリ）',text);self.assertIn('/blob/main/Audit/evidence.json',text)
    def test_fast_normal_test_link_marked(self):
        self.write('Verify/current/tests/normal.cpp','fixture');c=self.contents('fast_gcc');c['Document/README.md']=b'[Test](../Verify/current/tests/normal.cpp)\n';repair_flattened_markdown(c,self.root,fast=True);self.assertIn('通常版の補足・元リポジトリ',c['Document/README.md'].decode())
    def test_code_blocks_untouched(self):
        c=self.contents('gcc');c.pop('Audit/evidence.json');c['Document/README.md']=b'`[X](../Audit/evidence.json)`\n```\n[X](../Audit/evidence.json)\n```\n';old=c['Document/README.md'];repair_flattened_markdown(c,self.root);self.assertEqual(c['Document/README.md'],old)
    def test_missing_repo_target_rejected(self):
        c=self.contents('gcc');c['Document/README.md']=b'[X](../missing.json)\n';self.assertRaises(FileNotFoundError,repair_flattened_markdown,c,self.root)
    def test_index_only_selected_and_local(self):
        value=verify_index([dict(id='yes')],'GCC',True).decode();self.assertIn('submissions/yes.cpp',value);self.assertNotIn('submissions/no.cpp',value);self.assertIn('通常版のオンライン判定を',value)
    def test_repair_then_independent_validation(self):
        def fix(c):
            c.pop('Audit/evidence.json');c['Verify/current/README.md']=verify_index([dict(id='node')],'GCC',True);d=json.loads(c['Verify/current/manifest.json']);e=d['entries'][0];e['upstream_source_paths']=dict(e['source_paths']);e['source_paths']={origin:origin for origin in e['source_paths']};c['Verify/current/manifest.json']=json.dumps(d).encode();repair_flattened_markdown(c,self.root,fast=True)
        report=validate_archive(self.root,self.archive('fast_gcc',change=fix),'fast_gcc');self.assertTrue(report['ok'],report['errors']);self.assertEqual(report['warnings'],[])
    def test_directory_and_generator_context(self):
        for owner in ('Document/fast_variants.md', 'Document/bundling.md'):
            with self.subTest(owner=owner):
                c=self.contents('gcc');c[owner]=b'# Tools\n\n[Audit](../Audit/)\n';repair_flattened_markdown(c,self.root);self.assertIn('生成コマンド',c[owner].decode());self.assertIn('元リポジトリ全体',c[owner].decode())

def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite(loader.loadTestsFromTestCase(DistributionTests))
    for name in RepairTests.__dict__:
        if name.startswith("test_"):
            suite.addTest(RepairTests(name))
    return suite

if __name__ == "__main__":
    unittest.main()
