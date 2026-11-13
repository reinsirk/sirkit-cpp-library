#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Select a manifest edition and emit readable source with identical shared guards deduplicated."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
try:
    from .verify_distribution import code_mask, lexical_regions
except ImportError:
    from verify_distribution import code_mask, lexical_regions

PROFILES = {'normal': None, 'gcc': ('GCC', False), 'clang': ('Clang', False),
            'fast-gcc': ('GCC', True), 'fast-clang': ('Clang', True)}
GUARD = re.compile(r'SIRKIT_[A-Z0-9_]+_DEFINED\Z')
LIMIT = 512 * 1024
STANDARD_HEADERS = set('algorithm any array atomic barrier bit bitset cassert cctype cerrno cfenv cfloat charconv chrono cinttypes climits clocale cmath codecvt compare complex concepts condition_variable coroutine csetjmp csignal cstdarg cstddef cstdint cstdio cstdlib cstring ctime cuchar cwchar cwctype deque exception execution expected filesystem format forward_list fstream functional future generator initializer_list iomanip ios iosfwd iostream istream iterator latch limits list locale map mdspan memory memory_resource mutex new numbers numeric optional ostream print queue random ranges ratio regex scoped_allocator semaphore set shared_mutex source_location span spanstream sstream stack stacktrace stdexcept stop_token streambuf string string_view strstream syncstream system_error thread tuple type_traits typeindex typeinfo unordered_map unordered_set utility valarray variant vector version'.split())
COMPILER_HEADERS = {'immintrin.h', 'emmintrin.h', 'intrin.h', 'bits/stdc++.h'}


def digest(data):
    return hashlib.sha256(data).hexdigest()


def canonical_library(path):
    if not path or path.startswith('/') or any(c in path for c in '\\:\x00\r\n'):
        raise ValueError('noncanonical library path: ' + repr(path))
    if any(part in ('', '.', '..') for part in path.split('/')):
        raise ValueError('noncanonical library path: ' + path)
    if not path.endswith('.cpp') or path in ('temp.cpp', 'temp2.cpp'):
        raise ValueError('only canonical library modules are supported: ' + path)
    return path


def phase2(text):
    chars, offsets = [], []
    i = 0
    while i < len(text):
        splice = re.match(r'\\[ \t\v\f\r]*\n', text[i:]) if text[i] == '\\' else None
        if splice: i += splice.end()
        else: chars.append(text[i]); offsets.append(i); i += 1
    return ''.join(chars), offsets


def directives(text):
    logical, offsets = phase2(text)
    mask = code_mask(logical)
    if re.search(r'(?m)^[ \t\v\f\r]*%:', mask):
        raise ValueError('digraph directives are unsupported')
    records = []
    for m in re.finditer(r'(?m)^[ \t\v\f\r]*#[ \t\v\f\r]*([A-Za-z_][A-Za-z0-9_]*)([^\n]*)(?:\n|$)', mask):
        hash_at = mask.index('#', m.start(), m.end())
        name_end = m.end(1)
        arg_end = m.end(2)
        records.append(dict(kind=m.group(1), args=mask[name_end:arg_end].strip(),
                            raw_args=logical[name_end:arg_end].strip(),
                            start=offsets[m.start()], hash=offsets[hash_at],
                            end=offsets[m.end()-1]+1))
    return records


def split_frontmatter(text):
    logical, offsets = phase2(text)
    comments = {begin: end for begin,end,kind in lexical_regions(logical) if kind == 'comment'}
    at = 0
    while at < len(logical):
        if logical[at].isspace(): at += 1
        elif at in comments: at = comments[at]
        else: break
    physical = offsets[at] if at < len(offsets) else len(text)
    return text[:physical], text[physical:], 1 + text[:physical].count('\n')


def conditional_blocks(records):
    stack, blocks = [], []
    for index,d in enumerate(records):
        d['depth'] = len(stack)
        if d['kind'] in ('if','ifdef','ifndef'):
            stack.append(dict(open=index, outer_branch=False))
        elif d['kind'] in ('else','elif','elifdef','elifndef'):
            if not stack: raise ValueError('unmatched #' + d['kind'])
            stack[-1]['outer_branch'] = True
        elif d['kind'] == 'endif':
            if not stack: raise ValueError('unmatched #endif')
            frame = stack.pop();frame['close'] = index;blocks.append(frame)
    if stack: raise ValueError('unclosed conditional')
    return blocks


def include_policy(records):
    for d in records:
        if d['kind'] in ('include_next','import'): raise ValueError('unsupported include directive: ' + d['kind'])
        if d['kind'] != 'include': continue
        include = re.fullmatch(r'<([^>]+)>', d['raw_args'])
        # A trailing comment is harmless, and literals elsewhere must not be directives.
        if include is None:
            plain = re.sub(r'/\*.*?\*/|//.*', '', d['raw_args']).strip()
            include = re.fullmatch(r'<([^>]+)>', plain)
        if include is None:
            raise ValueError('quoted/macro include is unsupported in a self-contained bundle: ' + d['raw_args'])
        name = include.group(1)
        if name not in STANDARD_HEADERS | COMPILER_HEADERS:
            raise ValueError('external header is unsupported; supply Mint separately if required: ' + name)


def optimize_modules(modules, *, deduplicate=True):
    """Pure operation on trusted module bytes; each file must balance its conditionals."""
    parsed = []
    disabled, disable_all = set(), False
    for module in modules:
        text = module['data'].decode('utf-8')
        logical,_ = phase2(text)
        if re.search(r'\\[ \t\v\f\r]*\n', text):
            for candidate in (text,logical):
                if any(kind == 'literal' and candidate.startswith('R"',begin)
                       for begin,end,kind in lexical_regions(candidate)):
                    raise ValueError('raw-string files with physical line splicing are unsupported')
        prefix,body,line = split_frontmatter(text)
        records = directives(text)
        blocks = conditional_blocks(records)
        include_policy(records)
        for d in records:
            if d['kind'] == 'undef' and GUARD.fullmatch(d['args']): disabled.add(d['args'])
            # Token pasting can synthesize _Pragma or __pragma indirectly.
            if d['kind'] == 'define' and re.search(r'##|%:%:', d['args']): disable_all = True
            if d['kind'] == 'pragma' and re.search(r'\b(?:push_macro|pop_macro)\b', d['args']): disable_all = True
            if d['kind'] == 'line': raise ValueError('existing #line directives are unsupported')
        if re.search(r'\b(?:_Pragma|__pragma)\b', code_mask(logical)): disable_all = True
        parsed.append(dict(module, text=text, prefix=prefix, body=body, body_start=len(prefix), body_line=line,
                           records=records, blocks=blocks))
    # A macro with differing candidate bodies is preserved everywhere.
    definitions = {}
    for module in parsed:
        records = module['records']
        for frame in module['blocks']:
            opening,closing = records[frame['open']],records[frame['close']]
            guard = opening['args']
            if opening['depth'] != 0 or opening['kind'] != 'ifndef' or not GUARD.fullmatch(guard) or frame['outer_branch']:
                continue
            inner = records[frame['open']+1:frame['close']]
            if not inner or inner[0]['depth'] != 1 or inner[0]['kind'] != 'define' or inner[0]['args'] != guard:
                continue
            block = module['text'][opening['start']:closing['end']]
            definitions.setdefault(guard,set()).add(block)
    conflicting = sorted(guard for guard,bodies in definitions.items() if len(bodies)>1)
    disabled.update(conflicting)
    observed = {}
    removals = []
    for module in parsed:
        records,spans = module['records'],[]
        for frame in sorted(module['blocks'], key=lambda f: f['open']):
            opening,closing = records[frame['open']],records[frame['close']]
            guard = opening['args']
            if opening['depth'] != 0 or opening['kind'] != 'ifndef' or not GUARD.fullmatch(guard) or frame['outer_branch']:
                continue
            if module['text'][opening['start']:opening['hash']].strip(): continue
            inner = records[frame['open']+1:frame['close']]
            if not inner or inner[0]['depth'] != 1 or inner[0]['kind'] != 'define' or inner[0]['args'] != guard:
                continue
            if code_mask(module['text'][opening['end']:inner[0]['start']]).strip(): continue
            block = module['text'][opening['start']:closing['end']]
            if not deduplicate or disable_all or guard in disabled: continue
            if guard in observed and observed[guard]['block'] == block:
                spans.append((opening['start'],closing['end']))
                removals.append(dict(guard=guard, library=module['library'], source=module['source'],
                                     first_source=observed[guard]['source'], bytes=len(block.encode()), sha256=digest(block.encode()),
                                     first_line=1+module['text'][:opening['start']].count('\n'),
                                     after_line=1+module['text'][:closing['end']].count('\n')))
            elif guard not in observed:
                observed[guard] = dict(block=block, source=module['source'])
            # Differing definitions are kept and never substituted as provenance.
        module['remove_spans'] = spans
    pieces = ['// Readable module bundle; component guards and source provenance are retained.\n']
    for module in parsed:
        pieces.append('// Library: ' + module['library'] + '\n')
        pieces.append(module['prefix'])
        if not module['prefix'].endswith('\n'): pieces.append('\n')
    pieces.append('\n')
    for module in parsed:
        pieces.append('// Definitions: ' + module['library'] + '\n')
        filename = json.dumps(module['source'], ensure_ascii=False)
        pieces.append(f'#line {module["body_line"]} {filename}\n')
        cursor = module['body_start']
        for begin,end in module['remove_spans']:
            pieces.append(module['text'][cursor:begin])
            line = 1 + module['text'][:end].count('\n')
            pieces.append(f'// Identical guarded component omitted.\n#line {line} {filename}\n')
            cursor = end
        pieces.append(module['text'][cursor:])
        if not module['text'].endswith('\n'): pieces.append('\n')
    result = ''.join(pieces).encode()
    return result,dict(removals=removals, disabled_guards=sorted(disabled), macro_stack_disables_all=disable_all, conflicting_guards=conflicting,
                       libraries=[dict(library=m['library'], source=m['source'], sha256=digest(m['data'])) for m in parsed])


def load_modules(root, profile, libraries):
    data = (root/'Variants/manifest.json').read_bytes()
    entries = json.loads(data)['entries']
    base = root.resolve()
    for entry in entries:
        for field in ('source', 'path', *(['origin'] if 'origin' in entry else [])):
            name = entry[field]
            if not name or name.startswith('/') or any(c in name for c in '\\:\x00\r\n') or any(part in ('', '.', '..') for part in name.split('/')):
                raise ValueError('noncanonical manifest path: ' + repr(name))
            if not (root/name).resolve().is_relative_to(base):
                raise ValueError('manifest path escapes root: ' + name)
        expected = ('Fast/' if entry['fast'] else 'Variants/') + entry['compiler'] + '/' + entry['source']
        if entry['compiler'] not in ('GCC', 'Clang') or entry['path'] != expected:
            raise ValueError('manifest edition/path mismatch: ' + entry['path'])
    protected = {(root/e['path']).resolve() for e in entries} | {(root/e['source']).resolve() for e in entries}
    protected |= {(root/e['origin']).resolve() for e in entries if 'origin' in e}
    protected |= {(root/'temp.cpp').resolve(), (root/'temp2.cpp').resolve(), (root/'Variants/manifest.json').resolve()}
    selected = {}
    for entry in entries:
        if (profile == 'normal' and not entry['fast']) or (profile != 'normal' and (entry['compiler'],entry['fast']) == PROFILES[profile]):
            if profile == 'normal':
                selected.setdefault(entry['source'], dict(path=entry['source'], sha256=entry['source_sha256']))
                if selected[entry['source']]['sha256'] != entry['source_sha256']:
                    raise ValueError('normal manifest origins disagree')
            else: selected[entry['source']] = entry
    modules = []
    for name in libraries:
        name = canonical_library(name)
        if name not in selected: raise ValueError('library is not in the selected manifest: ' + name)
        entry = selected[name];raw = (root/entry['path']).read_bytes()
        if digest(raw) != entry['sha256']: raise ValueError('source differs from current manifest: ' + entry['path'])
        modules.append(dict(library=name, source=entry['path'], data=raw))
    return modules,protected,digest(data)


def atomic_write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent, delete=False) as temporary:
        name = Path(temporary.name);temporary.write(data)
    try: os.replace(name,path)
    finally: name.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument('--profile',choices=PROFILES,default='normal')
    parser.add_argument('--library',action='append',required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--report',type=Path)
    parser.add_argument('--allow-oversize',action='store_true')
    parser.add_argument('--no-deduplicate',action='store_true')
    args = parser.parse_args()
    try:
        modules,protected,manifest_sha = load_modules(args.root,args.profile,args.library)
        if args.output.resolve() in protected or (args.report and args.report.resolve() in protected):
            raise ValueError('refusing to overwrite a library/template source')
        if args.report and args.output.resolve() == args.report.resolve():
            raise ValueError('output and report paths must differ')
        before,_ = optimize_modules(modules,deduplicate=False)
        after,details = optimize_modules(modules,deduplicate=not args.no_deduplicate)
        accepted = len(after) <= LIMIT or args.allow_oversize
        report = dict(profile=args.profile, manifest_sha256=manifest_sha, selected_source_bytes=sum(len(m["data"]) for m in modules), code_bytes_before=len(before),
                      code_bytes_after=len(after), bytes_saved=len(before)-len(after), sha256_before=digest(before),
                      sha256_after=digest(after), limit=LIMIT, oversize_allowed=args.allow_oversize, accepted=accepted, **details)
        if args.report: atomic_write(args.report,(json.dumps(report,ensure_ascii=False,indent=2)+'\n').encode())
        print(json.dumps(report,ensure_ascii=False))
        if not accepted: return 2
        atomic_write(args.output,after)
        return 0
    except (ValueError,OSError,UnicodeError,KeyError) as error:
        print(json.dumps(dict(error=str(error)),ensure_ascii=False),file=__import__('sys').stderr)
        return 1


if __name__ == '__main__': raise SystemExit(main())
