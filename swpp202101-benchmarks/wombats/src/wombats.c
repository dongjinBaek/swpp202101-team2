#include <inttypes.h>
#define N_MAX 512
#define R_MAX 5000
#define C_MAX 200
#define B_SIZE 10
#define INF 999999999

uint64_t read();
void write(uint64_t);

extern int H[R_MAX][C_MAX], V[R_MAX][C_MAX], X[C_MAX], Y[C_MAX];
extern int R, C, B, N, Idx[N_MAX<<1][C_MAX][C_MAX], D[B_SIZE+1][C_MAX];

void makeTerminal(int K) {
 
    int i, j, k, U=(K-N)*B_SIZE;
 
    for(k=0 ; k<C ; k++) {
        for(i=0 ; i<B_SIZE ; i++) for(j=0 ; j<C ; j++) D[i][j]=INF;
        D[0][k]=0;
        for(i=0 ; i<B_SIZE ; i++) {
            for(j=1 ; j<C ; j++) D[i][j]=D[i][j] < (D[i][j-1]+H[U+i][j-1]) ?
                                            D[i][j] : (D[i][j-1]+H[U+i][j-1]);
            for(j=C-2 ; j>=0 ; j--) D[i][j]=D[i][j] < (D[i][j+1]+H[U+i][j]) ?
                                            D[i][j] : (D[i][j+1]+H[U+i][j]);
            for(j=0 ; j<C ; j++) D[i+1][j]=D[i][j]+V[U+i][j];
        }
        for(i=0 ; i<C ; i++) Idx[K][k][i]=D[B_SIZE][i];
    }
}

void makeInterior(int K) {
 
    int i, j, k, P, Q, Min, Num;
 
    for(i=0 ; i<C ; i++) {
        P=0, Q=C-1-i, X[i]=C-1, k=0, Min=INF+1;
        for(j=0 ; k<=i ; j++) {
            if(Min>Idx[K<<1][P][j]+Idx[(K<<1)+1][j][Q]) {
                Min=Idx[K<<1][P][j]+Idx[(K<<1)+1][j][Q];
                Num=j;
            }
            if(j==X[k]) {
                Idx[K][P][Q]=Min, Y[k]=Num;
                j--, k++, P++, Q++, Min=INF+1;
            }
        }
        for(j=0 ; j<=i ; j++) X[j]=Y[j];
    }
    for(i=0 ; i<C-1 ; i++) {
        P=C-1-i, Q=0, X[i]=C-1, k=0, Min=INF+1;
        for(j=0 ; k<=i ; j++) {
            if(Min>Idx[K<<1][P][j]+Idx[(K<<1)+1][j][Q]) {
                Min=Idx[K<<1][P][j]+Idx[(K<<1)+1][j][Q];
                Num=j;
            }
            if(j==X[k]) {
                Idx[K][P][Q]=Min, Y[k]=Num;
                j--, k++, P++, Q++, Min=INF+1;
            }
        }
        for(j=0 ; j<=i ; j++) X[j]=Y[j];
    }
}

void init() {
    int i, j, k;
 
    B=(R-1)/B_SIZE+1;
    for(N=1 ; N<B ; N<<=1);
    for(i=R ; i<B*B_SIZE ; i++) for(j=0 ; j<C-1 ; j++) H[i][j]=INF;
    for(i=N ; i<N+B ; i++) makeTerminal(i);
    for(i=N+B ; i<(N<<1) ; i++) for(j=0 ; j<C ; j++) {
        for(k=0 ; k<C ; k++) Idx[i][j][k]=INF;
        Idx[i][j][j]=0;
    }
    for(i=N-1 ; i>=1 ; i--) makeInterior(i);
}
 
void changeH(int P, int Q, int W) {
    int K=N+P/B_SIZE;
 
    H[P][Q]=W;
    makeTerminal(K);
    K>>=1;
    while(K) {
        makeInterior(K);
        K>>=1;
    }
}
 
void changeV(int P, int Q, int W) {
    int K=N+P/B_SIZE;
 
    V[P][Q]=W;
    makeTerminal(K);
    K>>=1;
    while(K) {
        makeInterior(K);
        K>>=1;
    }
}
 
int escape(int V1, int V2) {
    return Idx[1][V1][V2];
}

int main() {
	int E, P, Q, W, V1, V2, event, i, j;

    R = read();
    C = read();
    for (i = 0; i < R; ++i)
        for (j = 0; j < C-1; ++j)
            H[i][j] = read();
    for (i = 0; i < R-1; ++i)
        for (j = 0; j < C; ++j)
            V[i][j] = read();

    init();

    E = read();
	for (i = 0; i < E; i++) {
        event = read();
        if (event == 1) {
            P = read();
            Q = read();
            W = read();
            changeH(P, Q, W);
        } else if (event == 2) {
            P = read();
            Q = read();
            W = read();
            changeV(P, Q, W);
        } else if (event == 3) {
            V1 = read();
            V2 = read();
            write(escape(V1, V2));
        }
	}

	return 0;
}
