# SPDX-License-Identifier: CC0-1.0
import unittest
from Scripts.example_dependencies import expand_example_dependencies

class ExpansionTests(unittest.TestCase):
    def render(self, text, sources=None, owner='Examples/a.cpp', **kwargs):
        sources=sources or {'Math/A.cpp':'#include <vector>\n#ifndef A\n#define A\nint a;\n#endif\n'}
        return expand_example_dependencies(text,owner,sources,lambda p:sources[p],**kwargs)
    def test_single_include_old_bytes(self):
        body='#include <vector>\n#ifndef A\n#define A\nint a;\n#endif\n'
        text='// hi\n#include "../Math/A.cpp"\nint main(){}\n'
        self.assertEqual(self.render(text),text.replace('#include "../Math/A.cpp"',body))
    def test_nested_owner_and_profiles(self):
        normal={'Math/A.cpp':'#include "B.cpp"\nA\n','Math/B.cpp':'normal\n'}
        fast=dict(normal,**{'Math/B.cpp':'fast\n'})
        self.assertIn('normal',self.render('#include "../Math/A.cpp"\n',normal))
        self.assertIn('fast',self.render('#include "../Math/A.cpp"\n',fast))
    def test_false_branch_not_deduplicated(self):
        text='#if 0\n#include "../Math/A.cpp"\n#endif\n#include "../Math/A.cpp"\n'
        self.assertEqual(self.render(text).count('int a;'),2)
    def test_cache_raw_only_repeat_retained(self):
        calls=[]
        text='#include "../Math/A.cpp"\n#include "../Math/A.cpp"\n'
        out=expand_example_dependencies(text,'Examples/x.cpp',{'Math/A.cpp'},lambda p:(calls.append(p)or'A\n'))
        self.assertEqual(calls,['Math/A.cpp']);self.assertEqual(out.count('A\n'),2)
    def test_comments_strings_raw_and_digit_separator(self):
        prefix='/*\n#include "../Math/A.cpp"\n*/\nconst char*s=R"d(\n#include "../Math/A.cpp"\n)d";\n// #include "../Math/A.cpp"\nint x=1\'000\'000;\n'
        out=self.render(prefix+'#include "../Math/A.cpp"\n')
        self.assertTrue(out.startswith(prefix));self.assertEqual(out.count('int a;'),1)
    def test_line_comment_continuation(self):
        text='// continue \\\n#include "../Math/A.cpp"\n#include "../Math/A.cpp"\n'
        self.assertEqual(self.render(text).count('int a;'),1)
    def test_unknown_macro_angle_preserved(self):
        text='#include <vector>\n#include HEADER\n#include "config.hpp"\n'
        self.assertEqual(self.render(text),text)
    def test_trailing_comments_crlf_no_final_newline(self):
        text='#include "../Math/A.cpp" // marker\r\n#include "config.hpp"'
        out=self.render(text);self.assertIn('// marker\r\n',out);self.assertTrue(out.endswith('"config.hpp"'))
    def test_unicode_relative(self):
        self.assertEqual(self.render('#include "../Math/偏角.cpp"\n',{'Math/偏角.cpp':'OK'}),'OK\n')
    def test_unicode_line_separator_in_comment_or_string(self):
        text='// comment\u2028still comment\nconst char*s="a\u2028b";\n#include "../Math/A.cpp"\n'
        out=self.render(text)
        self.assertIn('const char*s="a\u2028b";',out)
        self.assertTrue(out.endswith('\n'));self.assertEqual(out.count('int a;'),1)
    def test_forbidden_paths(self):
        for path in ['../../secret.cpp','/etc/passwd','C:/secret.cpp','..\\Math\\A.cpp','https://example.com/a.cpp']:
            with self.subTest(path=path),self.assertRaises(ValueError):self.render('#include "'+path+'"\n')
    def test_cycles(self):
        with self.assertRaises(ValueError):self.render('#include "../Math/A.cpp"\n',{'Math/A.cpp':'#include "B.cpp"\n','Math/B.cpp':'#include "A.cpp"\n'})
    def test_capacity(self):
        with self.assertRaises(ValueError):self.render('#include "../Math/A.cpp"\n',{'Math/A.cpp':'A'*100},max_output_chars=50)
    def test_known_canonical(self):
        with self.assertRaises(ValueError):self.render('x',{'Math/../A.cpp':'bad'})
    def test_comment_before_directive(self):
        self.assertIn('int a;',self.render('/* allowed */ #include "../Math/A.cpp"\n'))

if __name__=='__main__':unittest.main()
