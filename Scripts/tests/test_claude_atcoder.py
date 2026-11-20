# SPDX-License-Identifier: CC0-1.0
from contextlib import redirect_stderr
import io
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from Scripts import claude_atcoder as launcher

APPROVED_ACCOUNT = 'approved@example.invalid'


class AccountIsolationTests(unittest.TestCase):
    def invoke(self, status, argv, env=None, *, approved_account=APPROVED_ACCOUNT, local_account=None, stderr=None):
        environment = dict(env or {})
        if approved_account is not None:
            environment[launcher.APPROVED_ACCOUNT_ENV] = approved_account
        with tempfile.TemporaryDirectory() as temporary, patch.dict(launcher.os.environ, environment, clear=True), \
             patch.object(launcher, 'PROFILE', Path(temporary)/'private'), \
             patch.object(launcher, 'LOCAL_ACCOUNT_FILE', Path(temporary)/'claude-approved-account'), \
             patch.object(launcher.subprocess, 'run') as run, redirect_stderr(stderr if stderr is not None else io.StringIO()):
            if local_account is not None:
                launcher.LOCAL_ACCOUNT_FILE.write_text(local_account, encoding='utf-8')
            run.side_effect = [subprocess.CompletedProcess([], 0, json.dumps(status)), subprocess.CompletedProcess([], 0)]
            result = launcher.main(argv)
            return result, run.call_args_list

    def good_status(self):
        return dict(loggedIn=True, authMethod='claude.ai', apiProvider='firstParty', email=APPROVED_ACCOUNT)

    def test_every_invocation_checks_account_then_uses_the_same_isolated_profile(self):
        for _ in range(2):
            args = ['-p', '--model', 'claude-opus-5-5', '--effort', 'max', 'own prompt']
            result, calls = self.invoke(self.good_status(), args)
            self.assertEqual(result, 0)
            self.assertEqual(len(calls), 2)
            self.assertEqual(calls[0].args[0], ['claude', 'auth', 'status', '--json'])
            self.assertEqual(calls[1].args[0], ['claude', *args])
            self.assertEqual(calls[0].kwargs['env'], calls[1].kwargs['env'])
            self.assertTrue(calls[0].kwargs['env']['CLAUDE_CONFIG_DIR'].endswith('/private'))
            self.assertNotIn(launcher.APPROVED_ACCOUNT_ENV, calls[0].kwargs['env'])

    def test_missing_or_invalid_approved_account_stops_before_authentication(self):
        for account in (None, '', 'invalid', 'one@example.invalid\ntwo@example.invalid'):
            with self.subTest(account=account):
                result, calls = self.invoke(self.good_status(), ['--status'], approved_account=account)
                self.assertEqual(result, 1)
                self.assertEqual(calls, [])

    def test_each_workspace_can_choose_an_account_without_logging_it(self):
        other_account = 'other@example.invalid'
        for account, expected in ((APPROVED_ACCOUNT, 1), (other_account, 0)):
            with self.subTest(expected=expected):
                log = io.StringIO()
                result, calls = self.invoke(self.good_status() | {'email': other_account}, ['--status'],
                                            approved_account=account, stderr=log)
                self.assertEqual(result, expected)
                self.assertEqual(len(calls), 1)
                self.assertNotIn(APPROVED_ACCOUNT, log.getvalue())
                self.assertNotIn(other_account, log.getvalue())

    def test_local_account_works_without_an_environment_setting(self):
        log = io.StringIO()
        result, calls = self.invoke(self.good_status(), ['--status'], approved_account=None,
                                    local_account=APPROVED_ACCOUNT + '\n', stderr=log)
        self.assertEqual(result, 0)
        self.assertEqual(len(calls), 1)
        self.assertNotIn(APPROVED_ACCOUNT, log.getvalue())

    def test_local_account_cannot_be_overridden_by_a_different_environment_setting(self):
        result, calls = self.invoke(self.good_status(), ['--status'],
                                    approved_account='other@example.invalid', local_account=APPROVED_ACCOUNT)
        self.assertEqual(result, 1)
        self.assertEqual(calls, [])

    def test_wrong_account_missing_login_or_invalid_status_cannot_launch_a_model(self):
        for value in [self.good_status() | dict(email='different@example.invalid'),
                      self.good_status() | dict(loggedIn=False),
                      self.good_status() | dict(authMethod='api_key'), [], {}]:
            with self.subTest(value=value):
                result, calls = self.invoke(value, ['-p', 'must not run'])
                self.assertEqual(result, 1)
                self.assertEqual(len(calls), 1)

    def test_other_authentication_environment_cannot_bypass_the_account_check(self):
        for name in ['ANTHROPIC_API_KEY', 'ANTHROPIC_AUTH_TOKEN', 'CLAUDE_CODE_OAUTH_TOKEN']:
            result, calls = self.invoke(self.good_status(), ['-p', 'must not run'], {name:'own unit fixture'})
            self.assertEqual(result, 1)
            self.assertEqual(calls, [])

    def test_status_only_does_not_launch_a_model(self):
        result, calls = self.invoke(self.good_status(), ['--status'])
        self.assertEqual(result, 0)
        self.assertEqual(len(calls), 1)

    def test_missing_model_and_effort_are_pinned_instead_of_using_cli_defaults(self):
        result, calls = self.invoke(self.good_status(), ['-p', 'own prompt'])
        self.assertEqual(result, 0)
        self.assertEqual(calls[1].args[0], ['claude', '--model', 'claude-opus-5-5',
                                         '--effort', 'max', '-p', 'own prompt'])

    def test_disallowed_or_ambiguous_selection_never_launches_a_model(self):
        for args in [['-p', '--model', 'claude-fable-5-2', 'own prompt'],
                     ['-p', '--model=fable', 'own prompt'],
                     ['-p', '--model=opus', 'own prompt'],
                     ['-p', '--effort=high', 'own prompt'],
                     ['-p', '--fallback-model', 'fable', 'own prompt'],
                     ['-p', '--model', 'claude-opus-5-5', '--model=claude-opus-5-5'],
                     ['-p', '--model']]:
            with self.subTest(args=args):
                result, calls = self.invoke(self.good_status(), args)
                self.assertEqual(result, 1)
                self.assertEqual(len(calls), 1)

    def test_equals_syntax_for_approved_selection_is_preserved(self):
        args = ['-p', '--model=claude-opus-5-5', '--effort=max', 'own prompt']
        result, calls = self.invoke(self.good_status(), args)
        self.assertEqual(result, 0)
        self.assertEqual(calls[1].args[0], ['claude', *args])
