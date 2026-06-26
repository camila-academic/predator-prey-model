#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #define CLEAR "cls"
  #define SLEEP(ms) Sleep(ms)
#else
  #include <unistd.h>
  #define CLEAR "clear"
  #define SLEEP(ms) usleep((ms)*1000)
#endif

#define GRID   10
#define MAXENT 64
#define SHEEP  1
#define WOLF   2
#define HUNTER 3
#define HOUSE  4
#define TREE   5

typedef struct { unsigned char type, x, y, alive; } Ent;

Ent E[MAXENT];
unsigned char ecnt=0, turn=0;
signed char over=0;

#define HSIZE 9
char hist[HSIZE][64];
unsigned char hcnt=0;

void log_push(const char *msg) {
    if(hcnt<HSIZE) strncpy(hist[hcnt++],msg,63);
    else { memmove(hist[0],hist[1],63*(HSIZE-1)); strncpy(hist[HSIZE-1],msg,63); }
}
void log_clear(void) { hcnt=0; memset(hist,0,sizeof(hist)); }

int rnd(int n)            { return rand()%n; }
int count(int t)          { int c=0; for(int i=0;i<ecnt;i++) c+=(E[i].alive&&E[i].type==t); return c; }
int at(int x,int y,int t) { for(int i=0;i<ecnt;i++) if(E[i].alive&&E[i].x==x&&E[i].y==y&&(t<0||E[i].type==t)) return 1; return 0; }
int cnt_at(int x,int y)   { int c=0; for(int i=0;i<ecnt;i++) c+=(E[i].alive&&E[i].x==x&&E[i].y==y); return c; }
int mdist(int ax,int ay,int bx,int by) { return abs(ax-bx)+abs(ay-by); }

void add(int t,int x,int y) {
    if(ecnt<MAXENT){ E[ecnt].type=t; E[ecnt].x=x; E[ecnt].y=y; E[ecnt].alive=1; ecnt++; }
}

void compact(void) {
    unsigned char w=0;
    for(unsigned char r=0;r<ecnt;r++) if(E[r].alive) E[w++]=E[r];
    ecnt=w;
}

void spawn(int t, int n) {
    for(int p=0,tries=0; p<n&&tries<300; tries++) {
        int x=rnd(GRID), y=rnd(GRID);
        if((x==4&&y==4)||at(x,y,-1)) continue;
        add(t,x,y); p++;
    }
}

void init(void) {
    memset(E,0,sizeof(E)); ecnt=turn=0; over=0; log_clear();
    for(int n=0,tries=0; n<8&&tries<300; tries++) {
        int x=rnd(GRID), y=rnd(GRID);
        if((x==4&&y==4)||at(x,y,TREE)) continue;
        add(TREE,x,y); n++;
    }
    add(HOUSE,4,4);
    spawn(WOLF,2); spawn(SHEEP,2); spawn(HUNTER,1);
}

char glyph(int x, int y) {
    int pri[]={HUNTER,WOLF,SHEEP,HOUSE,TREE};
    for(int p=0;p<5;p++)
        for(int i=0;i<ecnt;i++)
            if(E[i].alive&&E[i].x==x&&E[i].y==y&&E[i].type==pri[p])
                return "SWHCT"[pri[p]-1];
    return '.';
}

void render(void) {
    system(CLEAR);
    printf("=== FLORESTA SELVAGEM | Turno %d/10 ===\n", turn);
    printf("S=ovelha  W=lobo  H=cacador  C=casa  T=arvore\n\n");
    for(int y=0;y<GRID;y++) {
        for(int x=0;x<GRID;x++)
            printf(cnt_at(x,y)>1 ? "[%c]" : " %c ", glyph(x,y));
        printf("\n");
    }
    printf("\nS:%d  W:%d  H:%d\n", count(SHEEP),count(WOLF),count(HUNTER));
    printf("----------------------------------------\n");
    if(!hcnt) printf("  (nenhum evento ainda)\n");
    for(int i=0;i<hcnt;i++) printf("  %s\n", hist[i]);
    printf("----------------------------------------\n");
}

Ent *nearest(int x,int y,int t) {
    Ent *best=NULL; int bd=9999;
    for(int i=0;i<ecnt;i++) {
        if(!E[i].alive||E[i].type!=t) continue;
        int d=mdist(x,y,E[i].x,E[i].y);
        if(d<bd){bd=d;best=&E[i];}
    }
    return best;
}

void move_ent(Ent *e) {
    if(e->type==TREE||e->type==HOUSE) return;

    int dx[]={0,0,-1,1}, dy[]={-1,1,0,0};
    unsigned char vx[4],vy[4]; int nv=0;
    for(int d=0;d<4;d++) {
        int nx=e->x+dx[d], ny=e->y+dy[d];
        if(nx<0||nx>=GRID||ny<0||ny>=GRID||at(nx,ny,TREE)) continue;
        vx[nv]=nx; vy[nv]=ny; nv++;
    }
    if(!nv) return;

    int target_type = (e->type==WOLF) ? SHEEP : (e->type==HUNTER) ? WOLF : WOLF;
    int flee        = (e->type==SHEEP);
    Ent *tgt        = nearest(e->x,e->y,target_type);

    int weights[4]={0}, total=0;
    for(int i=0;i<nv;i++) {
        int d = tgt ? mdist(vx[i],vy[i],tgt->x,tgt->y) : 1;
        /*
         * Perseguidor: peso = (GRID*2 - d)^2  -> mais proximo = mais pesado
         * Fugitivo:    peso = d^2              -> mais longe   = mais pesado
         */
        int w = flee ? d*d : (GRID*2-d)*(GRID*2-d);
        weights[i] = w+1;
        total += weights[i];
    }

    int roll=rnd(total), acc=0, bi=0;
    for(int i=0;i<nv;i++) { acc+=weights[i]; if(roll<acc){bi=i;break;} }

    e->x=vx[bi]; e->y=vy[bi];
}

void resolve(int x, int y) {
    unsigned char ws[16],ss[16],hs[16],nw=0,ns=0,nh=0,house=0;
    for(int i=0;i<ecnt;i++) {
        if(!E[i].alive||E[i].x!=x||E[i].y!=y) continue;
        if(E[i].type==WOLF   && nw<16) ws[nw++]=i;
        if(E[i].type==SHEEP  && ns<16) ss[ns++]=i;
        if(E[i].type==HUNTER && nh<16) hs[nh++]=i;
        if(E[i].type==HOUSE) house=1;
    }
    char _m[64];
    if(nh) for(int i=0;i<nw;i++) { E[ws[i]].alive=0; snprintf(_m,sizeof(_m),"T%d: Cacador eliminou lobo em (%d,%d)",turn,x,y); log_push(_m); }
    int lw=0; for(int i=0;i<nw;i++) lw+=E[ws[i]].alive;
    if(lw)  for(int i=0;i<ns;i++) { E[ss[i]].alive=0; snprintf(_m,sizeof(_m),"T%d: Lobo devorou ovelha em (%d,%d)",turn,x,y); log_push(_m); }
    int ls=0; for(int i=0;i<ns;i++) ls+=E[ss[i]].alive;
    if(lw>=2) { add(WOLF,x,y);       snprintf(_m,sizeof(_m),"T%d: Dois lobos -> novo lobo em (%d,%d)",turn,x,y);          log_push(_m); }
    if(ls>=2) { add(SHEEP,x,y);      snprintf(_m,sizeof(_m),"T%d: Duas ovelhas -> nova ovelha em (%d,%d)",turn,x,y);      log_push(_m); }
    if(nh&&house){ add(HUNTER,x,y);  snprintf(_m,sizeof(_m),"T%d: Cacador na casa -> novo cacador em (%d,%d)",turn,x,y);  log_push(_m); }
}

void step(void) {
    if(over||turn>=10) return;
    turn++;
    compact();
    unsigned char ord[MAXENT],n=0;
    for(int i=0;i<ecnt;i++) if(E[i].alive&&E[i].type!=TREE&&E[i].type!=HOUSE) ord[n++]=i;
    for(int i=n-1;i>0;i--){int j=rnd(i+1);unsigned char t=ord[i];ord[i]=ord[j];ord[j]=t;}
    for(int i=0;i<n;i++) if(E[ord[i]].alive) move_ent(&E[ord[i]]);
    for(int y=0;y<GRID;y++) for(int x=0;x<GRID;x++) resolve(x,y);
    if(!count(SHEEP)) over=-1;
    else if(!count(WOLF)) over=1;
    else if(turn>=10) over=2;
}

int main(void) {
    srand((unsigned)time(NULL));
    init();
    char in[8];
    while(1) {
        render();
        if(over) {
            if(over==1)       printf("\n*** VITORIA: todos os lobos eliminados! ***\n");
            else if(over==-1) printf("\n*** DERROTA: todas as ovelhas devoradas! ***\n");
            else              printf("\n--- 10 turnos concluidos ---\n");
            printf("[r]reiniciar  [q]sair\n> ");
        } else {
            printf("\n[1]+1  [2]+2  [3]+3  [a]auto  [r]reiniciar  [q]sair\n> ");
        }
        if(!fgets(in,sizeof(in),stdin)) break;
        char c=in[0];
        if(c=='q'||c=='Q') break;
        if(c=='r'||c=='R'){init();continue;}
        if(over) continue;
        if(c=='1') step();
        else if(c=='2'){step();if(!over)step();}
        else if(c=='3'){step();if(!over)step();if(!over)step();}
        else if(c=='a'||c=='A')
            while(!over){step();render();printf("\nauto - ENTER para parar\n");fflush(stdout);SLEEP(300);}
    }
    printf("Ate logo!\n");
    return 0;
}
