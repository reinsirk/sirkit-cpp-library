from pathlib import Path
import struct,random,json,hashlib
p=Path(__file__).resolve().parent;data=p/'data';data.mkdir(exist_ok=True);r=random.Random(1826374)
primes=[1000000007,998244353,1000000009,4294967311,2305843009213693951,18446744069414584321,18446744073709551557,17798225727368200193,10346175718953582593];rows=[]
for shape,prime in enumerate(primes):
 patterns=[0,1,2]+([3]if shape in [0,5,6,7]else[])+([4]if shape in [0,5]else[])
 for pattern in patterns:
  values=[]
  for i in range(1024):
   if pattern==3:a=1 if i%2==0 else prime+1
   elif pattern==4:a=0 if i%2==0 else prime
   else:
    qr=pattern==0 or(pattern==2 and i%2==0)
    while True:
     a=r.randrange(prime)
     if qr:a=a*a%prime
     if a in [0,1]:continue
     if qr or pow(a,(prime-1)//2,prime)==prime-1:break
   values.append((a,prime,0))
  path=data/f'sqrt-{shape}-{pattern}.bin';path.write_bytes(b''.join(struct.pack('<QQQ',*v)for v in values));rows.append({'id':len(rows),'kind':0,'shape':shape,'pattern':pattern,'prime':prime,'file':str(path),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'classification':'all QR nontrivial'if pattern==0 else'all non-QR'if pattern==1 else'alternating QR/non-QR'if pattern==2 else'all normalize to1'if pattern==3 else'all normalize to0'})
for shape,mod in enumerate([1000000007,18446744069414584321,18446744073709551614,18446744073709551557]):
 values=[(r.getrandbits(64),mod,r.getrandbits(64))for _ in range(1024)];path=data/f'power-{shape}.bin';path.write_bytes(b''.join(struct.pack('<QQQ',*v)for v in values));rows.append({'id':len(rows),'kind':1,'shape':shape,'pattern':12,'prime':mod,'file':str(path),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'classification':'ModPow preservation control'})
(p/'benchmark_cases.json').write_text(json.dumps({'oracle':'independent Python builtin pow for QR/non-QR; nontrivial QR excludes0/1. Binary data contains uint64 a,modulus,exponent, not code.','queries_per_process':8192,'cases':rows},indent=2)+'\n');print('benchmark cases',len(rows))
