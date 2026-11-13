# SPDX-License-Identifier: CC0-1.0
"""非一意な解の出力を独立に検査する。"""
from collections import deque
from functools import lru_cache

def check_scc(entry, sample, output):
    try:
        data=list(map(int,sample['input'].split()));n,m=data[:2];arcs=list(zip(data[2::2],data[3::2]));values=list(map(int,output.split()));k=values[0];at=1;ids=[-1]*n;groups=[]
        if not 0<=k<=n:return False
        for c in range(k):
            length=values[at];at+=1
            if length<=0 or at+length>len(values):return False
            group=values[at:at+length];at+=length
            for v in group:
                if not 0<=v<n or ids[v]!=-1:return False
                ids[v]=c
            groups.append(group)
        if at!=len(values)or any(i<0 for i in ids):return False
        g=[[]for _ in range(n)];r=[[]for _ in range(n)]
        for u,v in arcs:
            if ids[u]>ids[v]:return False
            g[u].append(v);r[v].append(u)
        for graph in[g,r]:
            seen=[False]*n
            for c,group in enumerate(groups):
                seen[group[0]]=True;stack=[group[0]];count=0
                while stack:
                    v=stack.pop();count+=1
                    for u in graph[v]:
                        if ids[u]==c and not seen[u]:seen[u]=True;stack.append(u)
                if count!=len(group):return False
        return True
    except (ValueError,IndexError,TypeError):return False

def check_two_sat(entry,sample,output):
    try:
        a=output.split();x=sample['input'].split();n,m=map(int,x[2:4]);expected=sample['output'].split()
        if a==['s','UNSATISFIABLE']:return expected[:2]==a
        if len(a)!=n+4 or a[:3]!=['s','SATISFIABLE','v']or a[-1]!='0':return False
        assignment=list(map(int,a[3:-1]))
        if any(abs(v)!=i+1 for i,v in enumerate(assignment)):return False
        for pos in range(4,len(x),3):
            u,v=int(x[pos]),int(x[pos+1]);left=(assignment[abs(u)-1]>0)==(u>0);right=(assignment[abs(v)-1]>0)==(v>0)
            if not(left or right):return False
        return True
    except(ValueError,IndexError,TypeError):return False

def check_max_flow_grid(entry,sample,output):
    try:
        x=sample['input'].split();h,w=map(int,x[:2]);original=x[2:];a=output.split();count=int(a[0]);grid=a[1:]
        if count!=int(sample['output'].split()[0])or len(grid)!=h or any(len(row)!=w for row in grid):return False
        placed=0
        for i in range(h):
            for j in range(w):
                c=grid[i][j]
                if original[i][j]=='#':
                    if c!='#':return False
                    continue
                if c=='.':continue
                if c=='>':
                    if j+1>=w or grid[i][j+1]!='<':return False
                    placed+=1
                elif c=='v':
                    if i+1>=h or grid[i+1][j]!='^':return False
                    placed+=1
                elif c=='<':
                    if j==0 or grid[i][j-1]!='>':return False
                elif c=='^':
                    if i==0 or grid[i-1][j]!='v':return False
                else:return False
        return placed==count
    except(ValueError,IndexError,TypeError):return False


def check_pruefer_completion(entry, sample, output):
    """出力順や辺の選び方によらず、次数・森・連結を照合する。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, m = data[:2]
        if n <= 0 or len(data) != 2 + n + 2 * m:
            return False
        degrees = data[2:2 + n]
        edges = list(zip(data[2 + n::2], data[3 + n::2]))
        parent = list(range(n))
        size = [1] * n
        used = [0] * n

        def leader(v):
            while parent[v] != v:
                parent[v] = parent[parent[v]]
                v = parent[v]
            return v

        def add(u, v):
            u -= 1; v -= 1
            if not (0 <= u < n and 0 <= v < n):
                return False
            used[u] += 1; used[v] += 1
            a, b = leader(u), leader(v)
            if a == b:
                return False
            if size[a] < size[b]: a, b = b, a
            parent[b] = a; size[a] += size[b]
            return True

        forest = all(add(u, v) for u, v in edges)
        residual = [a - b for a, b in zip(degrees, used)]
        roots = {leader(i) for i in range(n)}
        budgets = {root: 0 for root in roots}
        for i, value in enumerate(residual): budgets[leader(i)] += value
        feasible = (forest and all(value >= 0 for value in residual) and
                    sum(residual) == 2 * (len(roots) - 1) and
                    (len(roots) == 1 or all(value > 0 for value in budgets.values())))
        tokens = list(map(int, output.split()))
        if tokens == [-1]:
            return not feasible
        if not feasible or len(tokens) != 2 * (n - 1 - m):
            return False
        for u, v in zip(tokens[::2], tokens[1::2]):
            if not add(u, v): return False
        return used == degrees and size[leader(0)] == n
    except (ValueError, IndexError, TypeError, KeyError):
        return False


def check_component_sets(entry,sample,output):
    def parse(text,n):
        a=list(map(int,text.split()));k=a[0];pos=1;groups=[]
        if k<0:return None
        for _ in range(k):
            length=a[pos];pos+=1
            if length<=0 or pos+length>len(a):return None
            group=a[pos:pos+length];pos+=length
            if len(set(group))!=length or any(v<0 or v>=n for v in group):return None
            groups.append(tuple(sorted(group)))
        return sorted(groups)if pos==len(a)else None
    try:
        n=int(sample['input'].split()[0]);actual=parse(output,n);expected=parse(sample['output'],n);return actual is not None and actual==expected
    except(ValueError,IndexError,TypeError):return False

def check_eulerian(entry,sample,output,directed):
    try:
        inp=list(map(int,sample['input'].split()));pos=1;a=output.split();expected=sample['output'].split();at=0;et=0
        for _ in range(inp[0]):
            n,m=inp[pos:pos+2];pos+=2;edges=[tuple(inp[pos+2*i:pos+2*i+2])for i in range(m)];pos+=2*m
            expected_yes=expected[et]=='Yes';et+=1
            if expected_yes:et+=(m+1 if n else 0)+m
            state=a[at];at+=1
            if state=='No':
                if expected_yes:return False
                continue
            if state!='Yes' or not expected_yes:return False
            length=m+1 if n else 0;vertices=list(map(int,a[at:at+length]));at+=length;ids=list(map(int,a[at:at+m]));at+=m
            if len(vertices)!=length or len(ids)!=m or sorted(ids)!=list(range(m))or any(v<0 or v>=n for v in vertices):return False
            for i,e in enumerate(ids):
                u,v=edges[e]
                if not((u,v)==(vertices[i],vertices[i+1])or(not directed and(v,u)==(vertices[i],vertices[i+1]))):return False
        return at==len(a)
    except(ValueError,IndexError,TypeError):return False

def check_eulerian_trail_directed(entry,sample,output):return check_eulerian(entry,sample,output,True)
def check_eulerian_trail_undirected(entry,sample,output):return check_eulerian(entry,sample,output,False)
def check_min_cost_flow_grid(entry,sample,output):
    try:
        inp=list(map(int,sample['input'].split()));n,k=inp[:2];values=inp[2:];a=output.split();score=int(a[0]);grid=a[1:]
        if score!=int(sample['output'].split()[0])or len(grid)!=n or any(len(row)!=n for row in grid):return False
        columns=[0]*n;total=0
        for i,row in enumerate(grid):
            count=0
            for j,c in enumerate(row):
                if c=='X':count+=1;columns[j]+=1;total+=values[i*n+j]
                elif c!='.':return False
            if count>k:return False
        return all(c<=k for c in columns)and total==score
    except(ValueError,IndexError,TypeError):return False


def check_bipartite_matching(entry, sample, output):
    """小さいsampleを右頂点集合の部分集合DPでも独立に判定する。"""
    try:
        data=list(map(int,sample['input'].split()));left,right,m=data[:3]
        if len(data)!=3+2*m or not(0<=right<=20):return False
        edges=set(zip(data[3::2],data[4::2]));actual=list(map(int,output.split()))
        if not actual or actual[0]<0 or len(actual)!=1+2*actual[0]:return False
        used_left=set();used_right=set()
        for u,v in zip(actual[1::2],actual[2::2]):
            if not(0<=u<left and 0<=v<right)or(u,v)not in edges or u in used_left or v in used_right:return False
            used_left.add(u);used_right.add(v)
        reachable={0}
        for u in range(left):
            following=set(reachable)
            for state in reachable:
                for v in range(right):
                    if(u,v)in edges and not(state>>v&1):following.add(state|(1<<v))
            reachable=following
        return actual[0]==max(state.bit_count()for state in reachable)
    except(ValueError,IndexError,TypeError):return False


def check_linear_recurrence(entry, sample, output):
    """小標本の全漸化式制約と、独立な掃き出し法による最短次数を検査する。"""
    try:
        data = list(map(int, sample["input"].split()))
        n, sequence = data[0], data[1:]
        if not (0 <= n <= 64) or len(sequence) != n:
            return False
        answer = list(map(int, output.split()))
        if not answer or not 0 <= answer[0] <= n or len(answer) != answer[0] + 1:
            return False
        mod = 998244353
        degree, coefficients = answer[0], answer[1:]
        if any(value < 0 or value >= mod for value in coefficients):
            return False
        if any((sequence[i] - sum(coefficients[j] * sequence[i - j - 1]
                                 for j in range(degree))) % mod for i in range(degree, n)):
            return False

        def consistent(d):
            rows = [[sequence[i - j - 1] % mod for j in range(d)] + [sequence[i] % mod]
                    for i in range(d, n)]
            rank = 0
            for column in range(d):
                pivot = next((i for i in range(rank, len(rows)) if rows[i][column]), None)
                if pivot is None:
                    continue
                rows[rank], rows[pivot] = rows[pivot], rows[rank]
                inverse = pow(rows[rank][column], mod - 2, mod)
                rows[rank] = [x * inverse % mod for x in rows[rank]]
                for i in range(rank + 1, len(rows)):
                    factor = rows[i][column]
                    if factor:
                        rows[i] = [(x - factor * y) % mod for x, y in zip(rows[i], rows[rank])]
                rank += 1
            return all(any(row[:d]) or row[d] == 0 for row in rows)

        return degree == next(d for d in range(n + 1) if consistent(d))
    except (ValueError, IndexError, TypeError, StopIteration):
        return False


def check_longest_common_substring(entry, sample, output):
    """小標本の最長長を独立DPで求め、異なる位置を返す有効解も受理する。"""
    try:
        first, second = sample["input"].split()
        positions = list(map(int, output.split()))
        if len(positions) != 4 or len(first) * len(second) > 1_000_000:
            return False
        a, b, c, d = positions
        if not (0 <= a <= b <= len(first) and 0 <= c <= d <= len(second)):
            return False
        if b - a != d - c or first[a:b] != second[c:d]:
            return False
        longest = 0
        before = [0] * (len(second) + 1)
        for x in first:
            current = [0] * (len(second) + 1)
            for j, y in enumerate(second):
                if x == y:
                    current[j + 1] = before[j] + 1
                    longest = max(longest, current[j + 1])
            before = current
        return b - a == longest
    except (ValueError, IndexError, TypeError):
        return False


def check_matrix_linear_system(entry, sample, output):
    """小標本の解・核基底・基底の独立性・不能判定を独立な掃き出し法で検査する。"""
    try:
        values = list(map(int, sample['input'].split()))
        n, m = values[:2]
        if not (0 <= n <= 64 and 0 <= m <= 64) or len(values) != 2 + n*m + n:
            return False
        mod = 998244353
        matrix = [values[2+i*m:2+(i+1)*m] for i in range(n)]
        rhs = values[2+n*m:]
        def rank(rows, columns):
            rows = [[x % mod for x in row] for row in rows]
            result = 0
            for column in range(columns):
                pivot = next((i for i in range(result, len(rows)) if rows[i][column]), None)
                if pivot is None:
                    continue
                rows[result], rows[pivot] = rows[pivot], rows[result]
                inverse = pow(rows[result][column], mod-2, mod)
                rows[result] = [x*inverse % mod for x in rows[result]]
                for i in range(result+1, len(rows)):
                    factor = rows[i][column]
                    if factor:
                        rows[i] = [(x-factor*y) % mod for x,y in zip(rows[i],rows[result])]
                result += 1
                if result == len(rows):
                    break
            return result
        coefficient_rank = rank(matrix, m)
        augmented_rank = rank([row + [b] for row,b in zip(matrix,rhs)], m+1)
        actual = list(map(int, output.split()))
        if actual == [-1]:
            return coefficient_rank != augmented_rank
        if coefficient_rank != augmented_rank or not actual:
            return False
        dimension = actual[0]
        if dimension != m-coefficient_rank or len(actual) != 1+m*(dimension+1):
            return False
        if any(x < 0 or x >= mod for x in actual[1:]):
            return False
        particular = actual[1:1+m]
        basis = [actual[1+m+i*m:1+m+(i+1)*m] for i in range(dimension)]
        def apply(vector):
            return [sum(x*y for x,y in zip(row,vector)) % mod for row in matrix]
        return (apply(particular) == [x % mod for x in rhs]
                and all(all(x == 0 for x in apply(vector)) for vector in basis)
                and rank(basis,m) == dimension)
    except (ValueError, IndexError, TypeError):
        return False


def validate_binary_linear_system(
    input_text: str, output_text: str
) -> tuple[bool, str]:
    try:
        tokens = input_text.split()
        n = int(tokens[0])
        m = int(tokens[1])
        rows = tokens[2 : 2 + n]
        rhs = tokens[2 + n]
        if (
            n < 1
            or m < 1
            or n > 128
            or m > 128
            or len(tokens) != n + 3
            or len(rows) != n
            or len(rhs) != n
            or any(len(row) != m or set(row) - {"0", "1"} for row in rows)
            or set(rhs) - {"0", "1"}
        ):
            return False, "invalid checker input"

        a = [sum((c == "1") << i for i, c in enumerate(row)) for row in rows]
        b = [int(c) for c in rhs]

        def rank(vectors, width):
            values = vectors[:]
            r = 0
            for col in range(width):
                pivot = next(
                    (j for j in range(r, len(values)) if (values[j] >> col) & 1),
                    None,
                )
                if pivot is None:
                    continue
                values[pivot], values[r] = values[r], values[pivot]
                for j in range(r + 1, len(values)):
                    if (values[j] >> col) & 1:
                        values[j] ^= values[r]
                r += 1
            return r

        coefficient_rank = rank(a, m)
        augmented_rank = rank([x | (y << m) for x, y in zip(a, b)], m + 1)
        exists = coefficient_rank == augmented_rank
        out = output_text.split()
        if out == ["-1"]:
            return not exists, "no-solution claim" if exists else ""
        if not exists:
            return False, "inconsistent system"
        if not out:
            return False, "empty output"
        dimension = int(out[0])
        if dimension != m - coefficient_rank or len(out) != 2 + dimension:
            return False, "dimension/output length"
        strings = out[1:]
        if any(len(s) != m or set(s) - {"0", "1"} for s in strings):
            return False, "binary vector length"
        vectors = [sum((c == "1") << i for i, c in enumerate(s)) for s in strings]
        if any(
            ((row & vectors[0]).bit_count() & 1) != value
            for row, value in zip(a, b)
        ):
            return False, "particular"
        if any((row & v).bit_count() & 1 for row in a for v in vectors[1:]):
            return False, "kernel"
        if rank(vectors[1:], m) != dimension:
            return False, "dependent basis"
        return True, ""
    except (IndexError, TypeError, ValueError):
        return False, "malformed output"


def check_binary_linear_system(entry: dict, sample: dict, output: str) -> bool:
    return validate_binary_linear_system(sample["input"], output)[0]


def validate_general_matching(input_text, output_text):
    try:
        data = list(map(int, input_text.split()))
        if len(data) < 2:
            return False, "input header"
        n, m = data[:2]
        if not 0 <= n <= 20 or m < 0 or len(data) != 2 + 2 * m:
            return False, "input size"
        neighbors = [0] * n
        edges = set()
        for u, v in zip(data[2::2], data[3::2]):
            if not (0 <= u < n and 0 <= v < n):
                return False, "input vertex"
            if u != v:
                edges.add((min(u, v), max(u, v)))
                neighbors[u] |= 1 << v
                neighbors[v] |= 1 << u

        actual = list(map(int, output_text.split()))
        if not actual or not 0 <= actual[0] <= n // 2:
            return False, "matching size"
        if len(actual) != 1 + 2 * actual[0]:
            return False, "output size"
        used = set()
        for u, v in zip(actual[1::2], actual[2::2]):
            if not (0 <= u < n and 0 <= v < n) or u == v:
                return False, "output vertex"
            if u in used or v in used:
                return False, "reused vertex"
            if (min(u, v), max(u, v)) not in edges:
                return False, "nonedge"
            used.update((u, v))

        @lru_cache(maxsize=None)
        def best(mask):
            if not mask:
                return 0
            bit = mask & -mask
            vertex = bit.bit_length() - 1
            remaining = mask ^ bit
            answer = best(remaining)
            candidates = remaining & neighbors[vertex]
            while candidates:
                other = candidates & -candidates
                answer = max(answer, 1 + best(remaining ^ other))
                candidates ^= other
            return answer

        if actual[0] != best((1 << n) - 1):
            return False, "not maximum"
        return True, ""
    except (ValueError, TypeError, IndexError):
        return False, "parse"


def check_general_matching(entry, sample, output):
    return validate_general_matching(sample["input"], output)[0]


def check_modular_square_root(entry, sample, output):
    """素数法を前提とするLC形式。任意の標準剰余の根と不能を独立に確認する。"""
    try:
        data = list(map(int, sample['input'].split()))
        count = data[0]
        answers = list(map(int, output.split()))
        if not 0 <= count <= 100000 or len(data) != 1 + 2*count or len(answers) != count:
            return False
        for index, root in enumerate(answers):
            value, prime = data[1+2*index:3+2*index]
            if not 2 <= prime <= (1 << 64)-1 or (prime != 2 and prime % 2 == 0):
                return False
            value %= prime
            if root == -1:
                if prime == 2 or value == 0 or pow(value, (prime-1)//2, prime) != prime-1:
                    return False
            elif not 0 <= root < prime or root*root % prime != value:
                return False
        return True
    except (ValueError, TypeError, IndexError):
        return False
