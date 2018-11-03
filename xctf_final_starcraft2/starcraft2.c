#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define Protoss 1
#define Terran  2
#define Zerg    3
#define Marine    4
#define Zergling    5
#define Stalker    6

struct Unit{
    long int type;
    unsigned long number;
};

struct Race{
    int type;
    unsigned long unitnum;
    unsigned long gas;
    unsigned long mineral;
    struct Unit** troop;
    struct Unit** team;
};

char id[0x10]="Ne0"; 

void* base;
char* cheat;
struct Race* race;
int is_signin;

void handler(int i){
   printf("Sorry %s,you are too slow...\n",id);
   exit(0);
}

void initialize(){
    setvbuf(stdin,0,2,0);
    setvbuf(stderr,0,2,0);
    setvbuf(stdout,0,2,0);
    alarm(30);
    signal(SIGALRM,handler);
    long int addr=0;
    
    int fd=open("/dev/urandom",0);
    if(fd==-1){
        exit(0);
    }
    read(fd,&addr,6);
    base = mmap((void*)addr, 0x1000, PROT_WRITE|PROT_READ, 34, -1, 0);
    if(((long int)base&0xFFF)!=0){
        exit(0);
    }
    fd=open("flag",0);
    if(fd==-1){
        exit(0);
    }
    read(fd,base,0x100);
    cheat=(char*)((long int)base+0x100);
    race=(struct Race*)((long int)base+0x200);
    race->troop=(struct Unit**)((long int)base+0x280);
    race->team=(struct Unit**)((long int)base+0x400);
    for(int i=0;i<10;i++){
        race->team[i]=(struct Unit*)((long int)base+0x500+i*sizeof(struct Unit));
    }
}

void check(){
   void* res;
   res=dlsym((void*)-1,"__malloc_hook");
   if(res){
       exit(0);
   }
   res=dlsym((void*)-1,"__free_hook");
   if(res){
       exit(0);
   }
}

int readn(char* name,unsigned int size){
    for(unsigned int i=0;i<size;i++){
        read(0,&name[i],1);
        if(name[i]=='\n'){
            name[i]='\0';
            return i;
        }
    }
    return size;
}

void signin(){
    puts("Input your name:");
    readn(id,0x10);
    puts("You have logged in!Welcome!");
    is_signin=1;
}

void changename(){
    if(!is_signin){
        puts("Login first!");
        return;
    }
    puts("Input your new name:");
    readn(id,strlen(id));
    puts("Your name has changed to --------");
    puts(id);
    puts("---------------------------------");
}

void banner(){
    puts("#     #           ###     ###                    #####           #####\n"
         "##    #  ######  #   #    ###     ####          #     #   ####  #     #\n"
         "# #   #  #      # #   #    #     #              #        #    #       #\n"
         "#  #  #  #####  #  #  #   #       ####           #####   #       #####\n"
         "#   # #  #      #   # #               #               #  #      #\n"
         "#    ##  #       #   #           #    #         #     #  #    # #\n"
         "#     #  ######   ###             ####           #####    ####  #######\n");
    puts("sucks....");
}

void menu(){    
    puts("1. Play");
    puts("2. Change name");
    puts("3. Make a map");
    puts("4. Report a bug");
    puts("5. Sign in");
    puts(">> ");
}

void select_race_menu(){
    puts("1. Select a race");
    puts("2. Build unit");
    puts("3. Add unit into a team");
    puts("4. Msg");
    puts("5. Showinfo");
    puts("6. Back");
    puts(">> ");
}

void select_race(){
    char racetype[0x100];
    memset(racetype,0,0x100);
    puts("Input your race:");
    readn(racetype,0x100);
    if(!strcmp(racetype,"Protoss")){
        puts("P imba!");
        race->type=Protoss;
        race->mineral=0x1000;
        race->gas=0x1000;
    }else if(!strcmp(racetype,"Terran")){
        puts("T is good!");
        race->type=Terran;
        race->mineral=0x10000;
        race->gas=0x10000;
    }else if(!strcmp(racetype,"Zerg")){
        puts("Z imba!");
        race->type=Zerg;
        race->mineral=0x1000;
        race->gas=0x1000;
    }else{
        puts("Are you kidding me?");
        return;
    }
    puts("Done!");
}

void build_unit(){
    if(!race->type){
        puts("You have to select a race first!");
        return;
    }
    char unitname[0x100];
    puts("Input the unit's name:");
    readn(unitname,0x100);
    int tmptype=0,tmp_m=0,tmp_g=0;
    if(!strcmp(unitname,"Marine")&&(race->type==Terran)){
        tmp_g=0;
        tmp_m=50; 
        tmptype=Marine;
    }else if(!strcmp(unitname,"Stalker")&&(race->type==Protoss)){
        tmp_g=25;
        tmp_m=125; 
        tmptype=Stalker;
    }else if(!strcmp(unitname,"Zergling")&&(race->type==Zerg)){
        tmp_g=0;
        tmp_m=25; 
        tmptype=Zergling;
    }
    if(tmptype==0){
        puts("No such unit,Cloud player!");
        return;
    }
    puts("Input the number:");
    unsigned long unum;
    scanf("%lu",&unum);
    if(race->gas<unum*tmp_g||race->mineral<unum*tmp_m){
        puts("You don't have enough resouces!");
        return;
    }
    race->gas-=unum*tmp_g;    
    race->mineral-=unum*tmp_m;    
    race->troop[race->unitnum]=(struct Unit*)malloc(sizeof(struct Unit));
    race->troop[race->unitnum]->type=tmptype;
    race->troop[race->unitnum]->number=unum;
    race->unitnum++;
    puts("Done!");
}

void make_team(){
    if(!race->type){
        puts("You have to select a race first!");
        return;
    }
    puts("Which unit:");
    unsigned long uidx,tidx;
    scanf("%lu",&uidx);
    if(uidx>=race->unitnum){
        puts("No such unit!");
        return;
    }
    puts("Which team:");
    scanf("%lu",&tidx);
    if(tidx>9){
        puts("No such team!");
    }
    race->team[tidx]->number+=race->troop[uidx]->number;
    race->team[tidx]->type=race->troop[uidx]->type;
    free(race->troop[uidx]);
    for(unsigned i=uidx;i<race->unitnum;i++){
        race->troop[i]=race->troop[i+1];
    }
    race->unitnum--;
    puts("Done!");
}

void makemap(){
    unsigned long mapsize;
    unsigned long x,y;
    char buff[0x30];
    puts("Input the map size:");
    scanf("%lu",&mapsize);
    char* map=(char*)malloc(mapsize*mapsize);
    sprintf(buff, "%lx", (unsigned long)map&0xFFFF);
    puts(buff);
    char c;
    while(1){
        puts("Input (x,y,c):");
        scanf("%lu,%lu,%c",&x,&y,&c);
        if(!(x<mapsize&&y<mapsize)){
            puts("How dare u!");
            exit(0);
        }        
        if(c=='\n'){
            break;
        }
        map[mapsize*x+y]=c;
    }
    puts("(Due to the limited resource, only part of your map is shown)Your map is:");
    if(mapsize>20){
        mapsize=20;
    }
    for(unsigned long i=0;i<mapsize;i++){
        write(1,map+mapsize*i,mapsize);
        write(1,"\n",1);
    }
    free(map);
}
int hash(char* buf){
    unsigned long res=0;
    if(strlen(buf)!=17){
        puts("Failed 1");
        return 0;
    }
    for(unsigned int i=0;i<strlen(buf);i++){
        res+=buf[i];
    }
    if(res!=1628||(buf[0]+buf[1])!=(buf[2]+buf[3]-11)){
        puts("Failed 2");
        return 0;
    }
    res=0;
    for(unsigned int i=0;i<strlen(buf);i++){
        res+=(unsigned)buf[i]*(unsigned)buf[i];
    }
    if(res!=171294){
        puts("Failed 3");
        return 0;
    }
    res=0;
    for(unsigned int i=0;i<strlen(buf);i++){
        res+=(unsigned)buf[i]*(unsigned)buf[i]*(unsigned)buf[i];
    }
    if(res!=18633746){
        puts("Failed 4");
        return 0;
    }
    res=0;
    for(unsigned int i=0;i<strlen(buf);i++){
        res^=(unsigned)buf[i];
    }
    if(res!=34){
        puts("Failed 5");
        return 0;
    }
    res=buf[2];
    if(res*res+buf[4]!=12353){
        puts("Failed 6");
        return 0;
    }
    return 1; 
}
void msg(){
    puts("Input your msg:");
    readn(cheat,0x100);
    if(hash(cheat)){
        puts("You know how to cheat in starcraft!");
        race->gas+=0x10000000000000;
        race->mineral+=0x10000000000000;
    }
    puts("Your msg:");
    puts(cheat);
}
void report_bug(){
    char buf[0x80];
    char size;
    char padding[3];
    puts("How long is your report:");
    scanf("%d",&size);
    if(size>0x7f){
        puts("Size too big...");
        exit(0);
    }
    puts("Please tell me about the bug:");
    readn(buf,size);
    puts("Your report is:");
    puts(buf);
    puts("Sorry, but the software written by me will never have bugs.So don't do this again. Otherwise I will take u to HK");
}

void show_unit(struct Unit* u){
    char buff[0x100];
    memset(buff,0,0x100);
    switch(u->type){
    case Marine:
        sprintf(buff,"%lx marines\n",u->number);
        break;
    case Zergling:
        sprintf(buff,"%lx zerglings\n",u->number);
        break;
    case Stalker:
        sprintf(buff,"%lx stalkers\n",u->number);
        break;
    }
    puts(buff);
}
void showinfo(){
    char buff[0x100];
    memset(buff,0,0x100);
    puts("Your race:");
    switch(race->type){
    case Protoss:
        puts("====> Protoss");
        break;
    case Terran:
        puts("====> Terran");
        break;
    case Zerg:
        puts("====> Zerg");
        break;
    }
    sprintf(buff, "Unit num %lx, mineral %lx , gas %lx\n", race->unitnum,race->mineral,race->gas);
    puts(buff);
    puts("Teams:");
    for(unsigned long i=0;i<9;i++){
        if(race->team[i]->type!=0){
            show_unit(race->team[i]);
        }
    }
    puts("Units:");
    for(unsigned long i=0;i<race->unitnum;i++){
        show_unit(race->troop[i]);
    }
}
int main(){
    initialize();
    banner();
    int choice;
    int flag=0;
    while(1){
        menu();
        scanf("%d",&choice);
        switch(choice){
        case 1:
            flag=1;
            while(flag){
                if(!is_signin){
                    break;
                }
                select_race_menu();
                scanf("%d",&choice);
                switch(choice){
                case 1:
                    select_race();
                    break;
                case 2:
                    build_unit();
                    break;
                case 3:
                    make_team();
                    break;
                case 4:
                    msg();
                    break;
                case 5:
                    showinfo();
                    break;
                case 6:
                    flag=0;
                    break;
                }
            }
            break;
        case 2:
            changename();
            break;
        case 3:
            makemap();
            break;
        case 4:
            report_bug();
            break;
        case 5:
            if(!is_signin){
                signin();
                break;
            }else{
                puts("You have already signed in!");
                break;
            }
        default:
            puts("Invalid choice!");
            break;
        }
    }
    return 0;
}
