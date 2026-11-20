#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""専用のClaude保存先を使い、毎回許可済みアカウントを確認してから起動する。"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PROFILE = ROOT / '.claude-code'
LOCAL_ACCOUNT_FILE = ROOT / '.sirkit-local' / 'claude-approved-account'
APPROVED_ACCOUNT_ENV = 'SIRKIT_CLAUDE_APPROVED_ACCOUNT'
APPROVED_MODEL = 'claude-opus-5-5'
APPROVED_EFFORT = 'max'


def model_arguments(args: list[str]) -> list[str]:
    # 明示指定も確認し、省略時だけCLI既定値へ任せることはしない。
    required = {'--model': APPROVED_MODEL, '--effort': APPROVED_EFFORT,
                '--fallback-model': APPROVED_MODEL}
    found: dict[str, str] = {}
    index = 0
    while index < len(args):
        key, separator, value = args[index].partition('=')
        if key in required:
            if not separator:
                index += 1
                if index == len(args):
                    raise ValueError(key + 'の値がありません。')
                value = args[index]
            if key in found or value != required[key]:
                raise ValueError('ClaudeはOpus 5.5・Maxのみ使用できます。別モデルへの切り替えは禁止です。')
            found[key] = value
        index += 1
    prefix = []
    for key in ('--model', '--effort'):
        if key not in found:
            prefix.extend((key, required[key]))
    return prefix + args


def main(argv: list[str] | None = None) -> int:
    args = list(sys.argv[1:] if argv is None else argv)
    environment = dict(os.environ, CLAUDE_CONFIG_DIR=str(PROFILE))
    # OAuthを確認しても別の資格情報で推論される構成は使わない。
    if any(environment.get(name) for name in ('ANTHROPIC_API_KEY', 'ANTHROPIC_AUTH_TOKEN', 'CLAUDE_CODE_OAUTH_TOKEN')):
        print('Claude: 別の認証用環境変数があるため、アカウントを確定できません。', file=sys.stderr)
        return 1
    approved_email = environment.pop(APPROVED_ACCOUNT_ENV, '').strip()
    try:
        local_email = LOCAL_ACCOUNT_FILE.read_text(encoding='utf-8').strip()
    except FileNotFoundError:
        pass
    except (OSError, UnicodeError):
        print('Claude: ローカルの許可済みアカウント設定を読み込めませんでした。', file=sys.stderr)
        return 1
    else:
        if approved_email and approved_email != local_email:
            print('Claude: 環境変数とローカルの許可済みアカウントが一致しません。', file=sys.stderr)
            return 1
        approved_email = local_email
    if not re.fullmatch(r'[^@\s]+@[^@\s]+', approved_email):
        print(f'Claude: ローカル設定または{APPROVED_ACCOUNT_ENV}に許可済みアカウントを指定してください。', file=sys.stderr)
        return 1
    try:
        PROFILE.mkdir(mode=0o700, parents=True, exist_ok=True)
        status = subprocess.run(['claude', 'auth', 'status', '--json'], cwd=ROOT,
                                env=environment, capture_output=True, text=True, timeout=30)
        data = json.loads(status.stdout)
    except (OSError, ValueError, subprocess.SubprocessError):
        print('Claude: 専用保存先のログイン状態を確認できませんでした。', file=sys.stderr)
        return 1
    if not isinstance(data, dict):
        print('Claude: ログイン状態の形式を確認できません。', file=sys.stderr)
        return 1
    email = data.get('email')
    print('Claude保存先: ' + str(PROFILE), file=sys.stderr)
    if (status.returncode != 0 or data.get('loggedIn') is not True
            or data.get('authMethod') != 'claude.ai' or data.get('apiProvider') != 'firstParty'
            or email != approved_email):
        print('Claude: 許可済みアカウントとの一致を確認できないため停止します。', file=sys.stderr)
        return 1
    print('Claudeアカウント: 照合済み', file=sys.stderr)
    if args == ['--status']:
        return 0
    if not args:
        print('Claude: 実行引数または--statusを指定してください。', file=sys.stderr)
        return 1
    try:
        args = model_arguments(args)
    except ValueError as error:
        print(str(error), file=sys.stderr)
        return 1
    return subprocess.run(['claude', *args], cwd=ROOT, env=environment).returncode


if __name__ == '__main__':
    raise SystemExit(main())
