# SPDX-License-Identifier: CC0-1.0
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
SCRIPTS=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(SCRIPTS))
from bundle_libraries import LIMIT,digest,directives,optimize_modules,split_frontmatter,canonical_library,load_modules
A='#ifndef SIRKIT_A_DEFINED\n#define SIRKIT_A_DEFINED\nint a;\n#endif\n'
class BundleTests(unittest.TestCase):
    def module(self,text,name='a.cpp'):
        return dict(library='Math/'+name,source='Math/'+name,data=text.encode())
    def render(self,*texts):
        return optimize_modules([self.module(t,str(i)+'.cpp')for i,t in enumerate(texts)])
    def test_identical_top_guard(self):
        result,meta=self.render(A,A);self.assertEqual(len(meta['removals']),1);self.assertEqual(result.count(b'int a;'),1);self.assertIn(b'#ifndef SIRKIT_A_DEFINED',result)
    def test_first_guard_retained_with_external_definition(self):
        result,meta=self.render('#define SIRKIT_A_DEFINED\n'+A,A);self.assertEqual(len(meta['removals']),1);self.assertIn(b'#define SIRKIT_A_DEFINED',result)
    def test_different_guard_body_keeps_every_copy(self):
        result,meta=self.render(A,A.replace('int a;','int different;'),A);self.assertEqual(len(meta['removals']),0);self.assertEqual(result.count(b'int a;'),2)
    def test_different_whitespace_kept(self):
        _,meta=self.render(A,A.replace('int a;','int  a;'));self.assertFalse(meta['removals'])
    def test_unknown_outer_condition(self):
        result,meta=self.render('#if MYSTERY\n'+A+'#endif\n',A);self.assertFalse(meta['removals']);self.assertEqual(result.count(b'int a;'),2)
    def test_unknown_second_condition(self):
        _,meta=self.render(A,'#if MYSTERY\n'+A+'#endif\n');self.assertFalse(meta['removals'])
    def test_else_on_outer_guard(self):
        branch=A.replace('#endif',' #else\nint alternate;\n#endif');_,meta=self.render(branch,branch);self.assertFalse(meta['removals'])
    def test_nested_else_allowed(self):
        nested=A.replace('int a;','#if FEATURE\nint a;\n#else\nint b;\n#endif');_,meta=self.render(nested,nested);self.assertEqual(len(meta['removals']),1)
    def test_first_define_must_be_unconditional(self):
        nested=A.replace('#define SIRKIT_A_DEFINED','#if FEATURE\n#define SIRKIT_A_DEFINED\n#endif');_,meta=self.render(nested,nested);self.assertFalse(meta['removals'])
    def test_first_directive_must_define_guard(self):
        wrong=A.replace('#define SIRKIT_A_DEFINED','#include <vector>\n#define SIRKIT_A_DEFINED');_,meta=self.render(wrong,wrong);self.assertFalse(meta['removals'])
    def test_code_before_define_kept(self):
        wrong=A.replace('#define','int before;\n#define',1);_,meta=self.render(wrong,wrong);self.assertFalse(meta['removals'])
    def test_undef_anywhere_disables_guard(self):
        for suffix in ['#undef SIRKIT_A_DEFINED\n','#if NEVER\n#undef SIRKIT_A_DEFINED\n#endif\n']:
            _,meta=self.render(A,A+suffix);self.assertFalse(meta['removals'])
    def test_undef_other_guard_not_disable(self):
        _,meta=self.render(A,A+'#undef SIRKIT_OTHER_DEFINED\n');self.assertEqual(len(meta['removals']),1)
    def test_pragma_push_pop_disable_all(self):
        for pragma in ['push_macro','pop_macro']:
            _,meta=self.render(A,A+'#pragma '+pragma+'("SIRKIT_A_DEFINED")\n');self.assertFalse(meta['removals'])
    def test_dynamic_pragma_disable_all(self):
        _,meta=self.render(A,A+'#define RESTORE _Pragma("pop_macro(\\"SIRKIT_A_DEFINED\\")")\n');self.assertFalse(meta['removals'])
    def test_spliced_dynamic_pragma_disable_all(self):
        _,meta=self.render(A,A+'#define RESTORE _Pra\\\n'+'gma("pop_macro(\\\"SIRKIT_A_DEFINED\\\")")\n');self.assertFalse(meta['removals'])
    def test_digraph_directives_rejected(self):
        with self.assertRaises(ValueError):self.render(A,'%:undef SIRKIT_A_DEFINED\n'+A)

    def test_macro_looking_raw_string_and_comments_ignored(self):
        misleading='const char*s=R"x(\n#undef SIRKIT_A_DEFINED\n)x";\n// #pragma pop_macro("SIRKIT_A_DEFINED")\n';_,meta=self.render(misleading+A,A);self.assertEqual(len(meta['removals']),1)
    def test_continuation_directive(self):
        split=A.replace('ifndef','ifn\\\ndef').replace('#define SIRKIT_A_DEFINED','#de\\\nfine SIRKIT_A_DEFINED');_,meta=self.render(split,split);self.assertEqual(len(meta['removals']),1)
    def test_continuation_line_comment_hides_undef(self):
        _,meta=self.render(A,'// continued \\\n#undef SIRKIT_A_DEFINED\n'+A);self.assertEqual(len(meta['removals']),1)
    def test_include_lines_all_retained(self):
        source='#include <vector>\n'+A;result,meta=self.render(source,source);self.assertEqual(result.count(b'#include <vector>'),2);self.assertEqual(len(meta['removals']),1)
    def test_external_dependencies_rejected(self):
        for include in ['"file.hpp"','HEADER','<atcoder/modint>','<custom/library.hpp>']:
            with self.subTest(include=include),self.assertRaises(ValueError):self.render('#include '+include+'\n'+A)
    def test_unbalanced_condition_rejected(self):
        for text in ['#if A\n', '#endif\n', '#else\n']:
            with self.assertRaises(ValueError):self.render(text)
    def test_prefix_preserved_before_any_include(self):
        first='// SPDX-License-Identifier: CC0-1.0\n// API FIRST\n#include <vector>\n'+A
        second='/* LICENSE SECOND */\n// API SECOND\n#include <string>\n'+A
        result,_=self.render(first,second);text=result.decode();self.assertLess(text.index('API SECOND'),text.index('#include <vector>'));self.assertIn('LICENSE SECOND',text);self.assertIn('#include <string>',text)
    def test_leading_comment_hash_not_removed(self):
        text='/* # misleading */ '+A;_,meta=self.render(text,text);self.assertFalse(meta['removals'])
    def test_non_component_macro_kept(self):
        text=A.replace('SIRKIT_A_DEFINED','APP_GUARD');_,meta=self.render(text,text);self.assertFalse(meta['removals'])
    def test_line_origin_preserved(self):
        result,meta=self.render('// API\n'+A,'// API\n'+A+'int line=__LINE__;\n');self.assertIn('#line '+str(meta['removals'][0]['after_line']),result.decode());self.assertIn('__LINE__',result.decode())
    def test_no_optimization_flag(self):
        result,meta=optimize_modules([self.module(A),self.module(A)],deduplicate=False);self.assertFalse(meta['removals']);self.assertEqual(result.count(b'int a;'),2)
    def test_cli_limit_no_overwrite(self):
        with tempfile.TemporaryDirectory()as tmp:
            root=Path(tmp);source=root/'Math/A.cpp';source.parent.mkdir();source.write_text('// '+('x'*(LIMIT+1))+'\n'+A);manifest=root/'Variants/manifest.json';manifest.parent.mkdir();manifest.write_text(json.dumps(dict(entries=[dict(source='Math/A.cpp',source_sha256=digest(source.read_bytes()),path='Variants/GCC/Math/A.cpp',compiler='GCC',fast=False)])));output=root/'out.cpp';output.write_text('sentinel');report=root/'report.json';cmd=[sys.executable,str(SCRIPTS/'bundle_libraries.py'),'--root',str(root),'--library','Math/A.cpp','--output',str(output),'--report',str(report)];r=subprocess.run(cmd,capture_output=True,text=True);self.assertEqual(r.returncode,2,r.stderr);self.assertEqual(output.read_text(),'sentinel');self.assertFalse(json.loads(report.read_text())['accepted']);r=subprocess.run(cmd+['--allow-oversize'],capture_output=True,text=True);self.assertEqual(r.returncode,0,r.stderr);self.assertGreater(output.stat().st_size,LIMIT)
    def test_cli_protected_source_not_overwritten(self):
        with tempfile.TemporaryDirectory()as tmp:
            root=Path(tmp);source=root/'Math/A.cpp';source.parent.mkdir();source.write_text(A);manifest=root/'Variants/manifest.json';manifest.parent.mkdir();manifest.write_text(json.dumps(dict(entries=[dict(source='Math/A.cpp',source_sha256=digest(source.read_bytes()),path='Variants/GCC/Math/A.cpp',compiler='GCC',fast=False)])));r=subprocess.run([sys.executable,str(SCRIPTS/'bundle_libraries.py'),'--root',str(root),'--library','Math/A.cpp','--output',str(source)],capture_output=True,text=True);self.assertEqual(r.returncode,1);self.assertEqual(source.read_text(),A)

    def test_cpp23_alternate_branches_preserved(self):
        for directive in ('elifdef ENABLE', 'elifndef DISABLE'):
            branch=A.replace('#endif', '#'+directive+'\nint alternate;\n#endif')
            result,meta=self.render(branch,branch)
            self.assertFalse(meta['removals'])
            self.assertEqual(result.count(b'int alternate;'),2)
    def test_cpp23_nested_branches_structural(self):
        branch=A.replace('int a;', '#if FIRST\nint a;\n#elifdef ENABLE\nint b;\n#elifndef DISABLE\nint c;\n#endif')
        _,meta=self.render(branch,branch)
        self.assertEqual(len(meta['removals']),1)
    def test_cpp23_whitespace_spliced_undef(self):
        _,meta=self.render(A,'#un\\ \t\ndef SIRKIT_A_DEFINED\n'+A)
        self.assertFalse(meta['removals'])
    def test_cpp23_whitespace_spliced_pragma(self):
        _,meta=self.render(A,'#define RESTORE _Pra\\ \t\ngma("pop_macro(\\\"SIRKIT_A_DEFINED\\\")")\n'+A)
        self.assertFalse(meta['removals'])
    def test_cpp23_spliced_frontmatter_comment(self):
        prefix='// license \\ \t\n#include "not_a_dependency.hpp"\n'
        text=prefix+'#include <vector>\n'+A
        head,body,line=split_frontmatter(text)
        self.assertEqual(head,prefix)
        self.assertTrue(body.startswith('#include <vector>'))
        self.assertEqual(line,3)
        result,meta=self.render(text,text)
        self.assertEqual(len(meta['removals']),1)
        self.assertEqual(result.count(b'not_a_dependency.hpp'),2)
    def test_raw_string_with_splice_rejected(self):
        with self.assertRaises(ValueError):self.render('const char*s=R"x(a)x";\n// splice \\\n'+A)
    def test_templates_and_noncanonical_paths_rejected(self):
        for path in ('temp.cpp','temp2.cpp','../Math/A.cpp','Math/../A.cpp','/Math/A.cpp','Math/A.hpp'):
            with self.subTest(path=path),self.assertRaises(ValueError):canonical_library(path)
    def test_manifest_edition_and_escape_rejected(self):
        for path in ('../outside.cpp','Variants/Clang/Math/A.cpp','/tmp/A.cpp'):
            with tempfile.TemporaryDirectory()as tmp:
                root=Path(tmp);(root/'Variants').mkdir()
                entry=dict(source='Math/A.cpp',path=path,compiler='GCC',fast=False)
                (root/'Variants/manifest.json').write_text(json.dumps(dict(entries=[entry])))
                with self.assertRaises(ValueError):load_modules(root,'normal',['Math/A.cpp'])


    def test_token_paste_disables_all(self):
        for operator in ('##', '%:%:'):
            _,meta=self.render('#define CAT(a,b) a'+operator+'b\n'+A,A)
            self.assertFalse(meta['removals'])
            self.assertTrue(meta['macro_stack_disables_all'])


    def test_directive_horizontal_whitespace(self):
        for space in ('\v', '\f', '\r'):
            _,meta=self.render(A,'#'+space+'undef SIRKIT_A_DEFINED\n'+A)
            self.assertFalse(meta['removals'])


    def test_fast_origin_protected(self):
        with tempfile.TemporaryDirectory()as tmp:
            root=Path(tmp)
            entry=dict(source='Math/A.cpp',origin='Fast/Source/Math/A.cpp',path='Fast/GCC/Math/A.cpp',compiler='GCC',fast=True,sha256=digest(A.encode()),source_sha256=digest(A.encode()))
            for field in ('source','origin','path'):
                target=root/entry[field];target.parent.mkdir(parents=True,exist_ok=True);target.write_text(A)
            (root/'Variants').mkdir();(root/'Variants/manifest.json').write_text(json.dumps(dict(entries=[entry])))
            for flag in ('--output','--report'):
                command=[sys.executable,str(SCRIPTS/'bundle_libraries.py'),'--root',str(root),'--profile','fast-gcc','--library','Math/A.cpp','--output',str(root/'out.cpp')]
                command.extend([flag,str(root/entry['origin'])])
                result=subprocess.run(command,capture_output=True,text=True)
                self.assertEqual(result.returncode,1,result.stderr)
                self.assertEqual((root/entry['origin']).read_text(),A)

if __name__=='__main__':unittest.main(verbosity=2)
