from pwn import *
import sys

context.log_level=True

libc=ELF('/lib/x86_64-linux-gnu/libc-2.27.so')

ru = lambda x : p.recvuntil(x,timeout=2)
sn = lambda x : p.send(x)
rl = lambda   : p.recvline(timeout=2)
sl = lambda x : p.sendline(x) 
rv = lambda x : p.recv(x,timeout=2)
sa = lambda a,b : p.sendafter(a,b,timeout=2)
sla = lambda a,b : p.sendlineafter(a,b,timeout=2)

def lg(s,addr):
    print('\033[1;31;40m%20s-->0x%x\033[0m'%(s,addr))

def raddr(a=6):
    if(a==6):
        return u64(rv(a).ljust(8,'\x00'))
    else:
        return u64(rl().strip('\n').ljust(8,'\x00'))

def choice(idx):
    sla(">> ",str(idx))

def play():
    choice(1)

def change_name(name):
    choice(2)
    sa("name:\n",name)
    
def signin(name):
    choice(5)
    sa("name:\n",name)

def createmap(mapsize,maplist):
    choice(3)
    sla("size:\n",str(mapsize))
    siz=int(rl().strip('\n'),16)
    for i,j,k in maplist:
        ru("c):\n")
        sl("%s,%s,%s"%(str(i),str(j),k))
    sl("0,0,\n")
    return siz

def select_race(race):
    choice(1)
    sla("race:\n",race)

def build_unit(unit,num):
    choice(2)
    sla("name:\n",unit)
    sla("number:\n",str(num))

def maketeam(uidx,teamidx):
    choice(3)
    sla(":\n",str(uidx))
    sla(":\n",str(teamidx))

def msg(msg):
    choice(4)
    sla("msg:\n",str(msg))

def report_bug(bug):
    choice(4)
    sla('report:\n',str(len(bug)+1))
    sla(":\n",bug)

def showinfo():
    choice(5)

def back():
    choice(6)


def patch_check():
    origin_file = open("./check/starcraft2",'rb')
    serverfile = open("./starcraft2",'rb')
    a=origin_file.read()
    b=serverfile.read()
    # check whether the file length is the same
    if(len(a)!=len(b)):
        return {"status":"down","msg":"file length not correct!"}
    
    diff_counter=0
    i=0
    while i < len(a):
        if(a[i]!=b[i]):
            diff_counter+=1
        i+=1

    if(diff_counter>0x30):
        return {"status":"down","msg":"patch too many bytes!"}
    
    ## check whether plt and got have been changed
    plt_start=0x9e0
    plt_size=0x120
    got_start=0x03f38
    got_size=0xc8
    
    if a[plt_start:plt_start+plt_size]!=b[plt_start:plt_start+plt_size]:
        return {"status":"down","msg":"can't patch plt"}

    if a[got_start:got_start+got_size]!=b[got_start:got_start+got_size]:
        return {"status":"down","msg":"can't patch got"}

    return {"status":"up","msg":"good"}

def functionality_check(ip,port):
    global p
    p=remote(ip,port)
    try:
        signin('A'*0x3+'\n')
        a="You have logged in!Welcome!\n"
        if ru(a)!=a:
            raise
        change_name("B"*3+'1'+'\n')
        a=p.clean()
        if 'BBB1' in a or 'Add unit into a team' not in a:
            raise
        sl('6')
        a=cyclic(0x40)
        maplist=[]
        for i in range(8):
            for j in range(8):
                maplist.append((i,j,a[i*8+j]))
        sz=createmap(0x8,maplist) 
        ru("(Due to the limited resource, only part of your map is shown)")
        a="Your map is:\n"
        if ru(a)!=a:
            raise
        a=ru("oaaapaaa\n")
        a=''.join(a.split("\n"))
        if a!=cyclic(0x40):
            raise
        #lg("Base size",sz)
        if(createmap(0x8,maplist)!=sz):
            raise
        if(createmap(0x10,maplist)!=(0x50+sz)):
            raise
        if((createmap(0x300,maplist)&0xFFF)!=0x10):
            raise
        play()
        select_race("Terran")
        for i in range(10):
            build_unit("Marine",0x1)
        for i in range(9):
            maketeam(0,0)
        showinfo()
        a=ru("5. Showinfo").split('\n')
        if "====> Terran"!=a[2] or "Unit num 1, mineral fe0c , gas 10000" !=a[3] or "9 marines"!=a[6] or "1 marines" !=a[9]:
            raise
        msg("show me the money")
        if rl()!='You know how to cheat in starcraft!\n':
            raise
        showinfo()
        a=ru("5. Showinfo").split('\n')
        for i in range(len(a)):
            print(str(i)+":"+a[i])
        if 'Unit num 1, mineral 1000000000fe0c , gas 10000000010000'!=a[3]:
            raise
        back()
        ksz=createmap(0x1,[])
        if (ksz-sz)!=0x220:
            raise
        report_bug("12345")
        if "12345" not in p.clean():
            raise
        p.close()
    except:
        return {"status":"down","msg":"something wrong"}
    return {"status":"up","msg":"good"}

def checker(ip,port):
    a=functionality_check('127.0.0.1',12345)
    if a["status"]=='down':
        return a
    return patch_check()

if __name__ == '__main__':
    #ip=sys.argv[1]
    #port=sys.argv[2]
    ip='127.0.0.1'
    port=12345
    print checker(ip,port)
