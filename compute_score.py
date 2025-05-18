import sys

# parse input file

def parse_input(path):
    tokens=open(path).read().split()
    it=iter(tokens)
    N=int(next(it)); M=int(next(it)); L=int(next(it))
    S=[]; P=[]
    for _ in range(N):
        s=list(next(it))
        p=int(next(it))
        S.append(s)
        P.append(p)
    return N,M,L,S,P


def parse_output(path,M):
    tokens=open(path).read().split()
    if len(tokens)<(M*(M+1)):
        raise ValueError("output format error")
    it=iter(tokens)
    C=[]; A=[[0]*M for _ in range(M)]
    for i in range(M):
        c=next(it)
        C.append(c)
        for j in range(M):
            A[i][j]=int(next(it))
    return C,A


def mat_mul(A,B):
    n=len(A)
    C=[[0.0]*n for _ in range(n)]
    for i in range(n):
        Ai=A[i]
        for k in range(n):
            aik=Ai[k]
            if aik==0:
                continue
            Bk=B[k]
            for j in range(n):
                C[i][j]+=aik*Bk[j]
    return C


def mat_pow(mat,power):
    n=len(mat)
    result=[[float(i==j) for j in range(n)] for i in range(n)]
    base=[row[:] for row in mat]
    while power>0:
        if power & 1:
            result=mat_mul(result,base)
        base=mat_mul(base,base)
        power//=2
    return result


def compute_word_probability(word,L,C,A):
    M=len(C)
    states={}
    n=0
    for j in range(M):
        states[(0,j)]=n; n+=1
        for i in range(len(word)-1):
            if word[i]==C[j]:
                states[(i+1,j)]=n; n+=1
    X=[[0.0]*n for _ in range(n)]
    for (length,u),j in states.items():
        for v in range(M):
            next_word=word[:length]+[C[v]]
            s=0
            while next_word[s:]!=word[:len(next_word)-s]:
                s+=1
            if len(next_word)-s!=len(word):
                i=states[(len(next_word)-s,v)]
                X[i][j]+=A[u][v]/100.0
    if L<=1:
        Y=[[float(i==j) for j in range(n)] for i in range(n)]
    else:
        Y=mat_pow(X,L-1)
    init=states[(1,0)] if C[0]==word[0] else states[(0,0)]
    ret=1.0
    for i in range(n):
        ret-=Y[i][init]
    if ret<0:
        ret=0.0
    if ret>1:
        ret=1.0
    return ret


def compute_score_details(N,M,L,S,P,C,A):
    for i in range(M):
        s=sum(A[i])
        if s!=100:
            return 0,f"Sum of A[{i}] is not 100 ({s})"
    total_score=0.0
    for word,score in zip(S,P):
        prob=compute_word_probability(word,L,C,A)
        total_score+=prob*score
    return int(round(total_score)),""


def main():
    if len(sys.argv)!=3:
        print("Usage: compute_score.py <input> <output>", file=sys.stderr)
        return 1
    N,M,L,S,P=parse_input(sys.argv[1])
    C,A=parse_output(sys.argv[2],M)
    score,err=compute_score_details(N,M,L,S,P,C,A)
    if err:
        print(err, file=sys.stderr)
    print(score)
    return 0

if __name__=='__main__':
    main()
