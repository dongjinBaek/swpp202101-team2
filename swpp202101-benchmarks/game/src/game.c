#include <stdlib.h>
#include <inttypes.h>

uint64_t read();
void write(uint64_t);

extern uint64_t *Yroot;
extern uint64_t R, C, P, Q, U, V;
extern uint64_t K;

uint64_t gcd2(uint64_t X, uint64_t Y) {
    uint64_t tmp;
    while (X != Y && Y != 0) {
        tmp = X;
        X = Y;
        Y = tmp % Y;
    }
    return X;
}

void init(uint64_t R_, uint64_t C_) {
 
    for(R=1 ; R<R_ ; R<<=1);
    for(C=1 ; C<C_ ; C<<=1);
    Yroot=malloc(3 * sizeof(uint64_t));
    *((uint64_t **)Yroot)=NULL;
    *((uint64_t **)Yroot+1)=NULL;
    *((uint64_t **)Yroot+2)=NULL;
}
 
uint64_t Xcalculate(uint64_t *now) {
 
    uint64_t value=0;
 
    if(Q<=*((uint64_t *)now+2) && *((uint64_t *)now+3)<=V) return *((uint64_t *)now+4);
    else {
        if(*((uint64_t **)now)!=NULL && Q<=*(*((uint64_t **)now)+3) && *(*((uint64_t **)now)+2)<=V) value=Xcalculate(*((uint64_t **)now));
        if(*((uint64_t **)now+1)!=NULL && Q<=*(*(((uint64_t **)now)+1)+3) && *(*(((uint64_t **)now)+1)+2)<=V) value=gcd2(value,Xcalculate(*((uint64_t **)now+1)));
        return value;
    }
}
 
uint64_t Ycalculate(uint64_t l, uint64_t r, uint64_t *now) {
 
    uint64_t m=(l+r)>>1;
    uint64_t value=0;
 
    if(P<=l && r<=U) {
        if(*((uint64_t **)now+2)!=NULL) value=Xcalculate(*((uint64_t **)now+2));
        return value;
    }
    else {
        if(P<=m && *((uint64_t **)now)!=NULL) value=Ycalculate(l,m,*((uint64_t **)now));
        if(m+1<=U && *((uint64_t **)now+1)!=NULL) value=gcd2(value,Ycalculate(m+1,r,*((uint64_t **)now+1)));
        return value;
    }
}
 
void Xupdate(uint64_t *now) {
 
    uint64_t m=(*((uint64_t *)now+2)+(*((uint64_t *)now+3)))>>1, x, y;
    uint64_t *temp;
 
    if(*((uint64_t *)now+2)==Q && Q==*((uint64_t *)now+3)) *((uint64_t *)now+4)=K;
    else {
        if(Q<=m) {
            if(*((uint64_t **)now)==NULL) {
                *((uint64_t **)now)=malloc(5 * sizeof(uint64_t));
                **((uint64_t ***)now)=NULL;
                *(*((uint64_t ***)now)+1)=NULL;
                *(*((uint64_t **)now)+2)=Q;
                *(*((uint64_t **)now)+3)=Q;
                *(*((uint64_t **)now)+4)=0;
            }
            if(Q<*(*((uint64_t **)now)+2) || *(*((uint64_t **)now)+3)<Q) {
                temp=*((uint64_t **)now);
                *((uint64_t **)now)=malloc(5 * sizeof(uint64_t));
                **((uint64_t ***)now)=NULL;
                *(*((uint64_t ***)now)+1)=NULL;
                *(*((uint64_t **)now)+2)=*((uint64_t *)now+2);
                *(*((uint64_t **)now)+3)=m;
                *(*((uint64_t **)now)+4)=0;
                if(Q<*((uint64_t *)temp+2)) *(*((uint64_t ***)now)+1)=temp;
                else **((uint64_t ***)now)=temp;
                while(1) {
                    x=(*(*((uint64_t **)now)+3)-(*(*((uint64_t **)now)+2))+1)>>1;
                    y=(*(*((uint64_t **)now)+2)+(*(*((uint64_t **)now)+3)))>>1;
                    if(!(Q&x) && !(*((uint64_t *)temp+2)&x)) *(*((uint64_t **)now)+3)=y;
                    else if(Q&x && *((uint64_t *)temp+2)&x) *(*((uint64_t **)now)+2)=y+1;
                    else break;
                }
            }
            Xupdate(*((uint64_t **)now));
        }
        else {
            if(*((uint64_t **)now+1)==NULL) {
                *((uint64_t **)now+1)=malloc(5 * sizeof(uint64_t));
                **(((uint64_t ***)now)+1)=NULL;
                *(*(((uint64_t ***)now)+1)+1)=NULL;
                *(*(((uint64_t **)now)+1)+2)=Q;
                *(*(((uint64_t **)now)+1)+3)=Q;
                *(*(((uint64_t **)now)+1)+4)=0;
            }
            if(Q<*(*(((uint64_t **)now)+1)+2) || *(*(((uint64_t **)now)+1)+3)<Q) {
                temp=*((uint64_t **)now+1);
                *((uint64_t **)now+1)=malloc(5 * sizeof(uint64_t));
                **(((uint64_t ***)now)+1)=NULL;
                *(*(((uint64_t ***)now)+1)+1)=NULL;
                *(*(((uint64_t **)now)+1)+2)=m+1;
                *(*(((uint64_t **)now)+1)+3)=*((uint64_t *)now+3);
                *(*(((uint64_t **)now)+1)+4)=0;
                if(Q<*((uint64_t *)temp+2)) *(*(((uint64_t ***)now)+1)+1)=temp;
                else **(((uint64_t ***)now)+1)=temp;
                while(1) {
                    x=(*(*(((uint64_t **)now)+1)+3)-(*(*(((uint64_t **)now)+1)+2))+1)>>1;
                    y=(*(*(((uint64_t **)now)+1)+2)+(*(*(((uint64_t **)now)+1)+3)))>>1;
                    if(!(Q&x) && !(*((uint64_t *)temp+2)&x)) *(*(((uint64_t **)now)+1)+3)=y;
                    else if(Q&x && *((uint64_t *)temp+2)&x) *(*(((uint64_t **)now)+1)+2)=y+1;
                    else break;
                }
            }
            Xupdate(*((uint64_t **)now+1));
        }
        if(*((uint64_t **)now+1)==NULL) *((uint64_t *)now+4)=*(*((uint64_t **)now)+4);
        else if(*((uint64_t **)now)==NULL) *((uint64_t *)now+4)=*(*(((uint64_t **)now)+1)+4);
        else *((uint64_t *)now+4)=gcd2(*(*((uint64_t **)now)+4),*(*(((uint64_t **)now)+1)+4));
    }
}
 
void Yupdate(uint64_t l, uint64_t r, uint64_t *now) {
 
    uint64_t m=(l+r)>>1;
 
    if(l<r) {
        if(P<=m) {
            if(*((uint64_t **)now)==NULL) {
                *((uint64_t **)now)=malloc(3 * sizeof(uint64_t));
                **((uint64_t ***)now)=NULL;
                *(*((uint64_t ***)now)+1)=NULL;
                *(*((uint64_t ***)now)+2)=NULL;
            }
            Yupdate(l,m,*((uint64_t **)now));
        }
        if(m+1<=P) {
            if(*((uint64_t **)now+1)==NULL) {
                *((uint64_t **)now+1)=malloc(3 * sizeof(uint64_t));
                **(((uint64_t ***)now)+1)=NULL;
                *(*(((uint64_t ***)now)+1)+1)=NULL;
                *(*((uint64_t ***)now+1)+2)=NULL;
            }
            Yupdate(m+1,r,*((uint64_t **)now+1));
        }
        if(*((uint64_t **)now+1)==NULL) K=Xcalculate(*(*((uint64_t ***)now)+2));
        else if(*((uint64_t **)now)==NULL) K=Xcalculate(*(*((uint64_t ***)now+1)+2));
        else K=gcd2(Xcalculate(*(*((uint64_t ***)now)+2)),Xcalculate(*(*((uint64_t ***)now+1)+2)));
    }
    if(*((uint64_t **)now+2)==NULL) {
        *((uint64_t **)now+2)=malloc(5 * sizeof(uint64_t));
        **((uint64_t ***)now+2)=NULL;
        *(*((uint64_t ***)now+2)+1)=NULL;
        *(*((uint64_t **)now+2)+2)=0;
        *(*((uint64_t **)now+2)+3)=C-1;
        *(*((uint64_t **)now+2)+4)=0;
    }
    Xupdate(*((uint64_t **)now+2));
}
 
void update(uint64_t P_, uint64_t Q_, uint64_t K_) {
 
    P=P_, Q=Q_, K=K_, V=Q;
    Yupdate(0,R-1,Yroot);
}
 
uint64_t calculate(uint64_t P_, uint64_t Q_, uint64_t U_, uint64_t V_) {
 
    P=P_, Q=Q_, U=U_, V=V_;
    return Ycalculate(0,R-1,Yroot);
}

int main() {
	uint64_t R, C, N;
    uint64_t P, Q, U, V;
    uint64_t K;
    uint64_t i, type;
	uint64_t res;

    R = read();
    C = read();
    N = read();

    init(R, C);

	for (i = 0; i < N; i++) {
        type = read();
        if (type == 1) {
            P = read();
            Q = read();
            K = read();
            update(P, Q, K);
        } else { // type == 2
            P = read();
            Q = read();
            U = read();
            V = read();
            write(calculate(P, Q, U, V));
        }
	}

	return 0;
}
