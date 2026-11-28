"""Verify先の追加と、提出先を混同しないための検査。"""
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import prepare_verify as verify
import update_verify_status as status

class JudgeDomainsTest(unittest.TestCase):
    def test_approved_online_judges(self):
        for host in verify.ONLINE_JUDGE_NAMES:
            with self.subTest(host=host):
                entry={'id':'x','problem_url':'https://'+host+'/problem/1','gcc_optimization':'O3'}
                self.assertFalse(verify.custom_io_enabled(entry))
                self.assertEqual(verify.gcc_optimization(entry),'O3')
                self.assertEqual(status.safe_url(entry['problem_url']),entry['problem_url'])
    def test_custom_io_remains_lc_only(self):
        for host in verify.ONLINE_JUDGE_NAMES:
            entry={'id':'x','problem_url':'https://'+host+'/problem/1','allow_custom_io':True}
            if host=='judge.yosupo.jp':self.assertTrue(verify.custom_io_enabled(entry))
            else:
                with self.assertRaises(ValueError):verify.custom_io_enabled(entry)
    def test_interactive_transcript_uses_unbuffered_input_path(self):
        entry = {'id': 'interactive', 'problem_url': 'https://yukicoder.me/problems/no/2085',
                 'interactive_transcript': True, 'libraries': []}
        self.assertTrue(verify.custom_io_enabled(entry))
        self.assertIn(b'int main() {}', verify.render(entry, driver_override='int main() {}')[0])
        with self.assertRaisesRegex(ValueError, '共通高速入出力'):
            verify.render(entry, driver_override=verify.FAST_IO_INCLUDE + '\nint main() {}')
        for invalid in ({'allow_custom_io': True}, {'interactive_transcript': 1}, {'problem_url': ''},
                        {'problem_url': 'https://judge.yosupo.jp/problem/aplusb'}):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                verify.custom_io_enabled(entry | invalid)
    def test_aoj_aliases_share_judge_but_not_other_sites(self):
        url='https://judge.u-aizu.ac.jp/onlinejudge/review.jsp?rid=1'
        self.assertEqual(status.safe_url(url,judge='onlinejudge.u-aizu.ac.jp'),url)
        with self.assertRaises(ValueError):status.safe_url(url,judge='yukicoder.me')
    def test_float_sample_uses_the_judge_specific_tolerances(self):
        sample = {'output': '1 0'}
        default = {'sample_checker': 'float'}
        strict = default | {'absolute_tolerance': 1e-9, 'relative_tolerance': 1e-9}
        self.assertTrue(verify.matches(default, sample, '1 0.00000002'))
        self.assertFalse(verify.matches(strict, sample, '1 0.00000002'))
        self.assertFalse(verify.matches(strict, sample, '1.00000002 0'))
        self.assertTrue(verify.matches(strict, sample, '1.0000000002 0.0000000002'))
    def test_url_injection_still_rejected(self):
        for url in ['http://yukicoder.me/problems/no/1','https://yukicoder.me.evil.test/1','https://u@onlinejudge.u-aizu.ac.jp/1','https://judge.u-aizu.ac.jp:444/1','https://evil.test/1']:
            with self.subTest(url=url),self.assertRaises(ValueError):status.safe_url(url)
    def test_summary_combines_aoj_hosts(self):
        data={'entries':[{'judge':host,'status':'unsubmitted'} for host in verify.ONLINE_JUDGE_NAMES],'counts':{'local_only':0}}
        text=status.readme_summary(data)
        self.assertIn('| AOJ | 2 | 0 | 0 | 2 | 0 |',text)
        self.assertIn('| Yukicoder | 1 | 0 | 0 | 1 | 0 |',text)
        self.assertIn('| AtCoder | 1 | 0 | 0 | 1 | 0 |',text)
if __name__=='__main__':unittest.main()
