perlbench = {"1", "2", "3"}
gcc = {"1", "2", "3", "4", "5"}
bwaves = {"1", "2", "3", "4"}
mcf = {"1"}
cactuBSSN = {"1"}
namd = {"1"}
parest = {"1"}
povray = {"1"}
lbm = {"1"}
omnetpp = {"1"}
wrf = {"1"}
xalanbmk = {"1"}
## x264 = {"1", "2", "3"}
x264 = {"1", "3"}
blender = {"1"}
cam4 = {"1"}
deepsjeng = {"1"}
imagick = {"1"}
leela = {"1"}
nab = {"1"}
exchange2 = {"1"}
fotonik3d = {"1"}
roms = {"1"}
xz = {"1", "2", "3"}

BT = {"C"}
CG = {"C"}
DC = {"A"}
EP = {"C"}
FT = {"C"}
IS = {"C"}
LU = {"C"}
MG = {"C"}
SP = {"C"}
UA = {"C"}

SPEC = {
    # "perlbench" : perlbench,
    # "gcc" : gcc,
    # "bwaves" : bwaves,
    "mcf" : mcf,
    ## "cactuBSSN" : cactuBSSN,
    "namd" : namd,
    ## "parest" : parest,
    "povray" : povray,
    "lbm" : lbm,
    "omnetpp" : omnetpp,
    "wrf" : wrf,
    # "xalancbmk" : xalanbmk,
    "x264" : x264, 
    "blender" : blender,
    "cam4" : cam4,
    "deepsjeng" : deepsjeng,
    "imagick" : imagick,
    "leela" : leela,
    "nab" : nab,
    "exchange2" : exchange2,
    "fotonik3d" : fotonik3d,
    "roms" : roms,
    "xz" : xz
}

NPB = {
    "bt": BT,
    "cg": CG,
    #"dc": DC,
    "ep": EP,
    "ft": FT,
    "is": IS,
    "lu": LU,
    "mg": MG,
    "sp": SP,
    "ua": UA
}

blackscholes = {"1"}
bodytrack = {"1"}
facesim = {"1"}
ferret = {"1"}
fluidanimate = {"1"}
freqmine = {"1"}
raytrace = {"1"}
swaptions = {"1"}
vips = {"1"}
par_x264 = {"1"}
canneal = {"1"}
dedup = {"1"}
streamcluster = {"1"}

PARSEC = {
    "blackscholes": blackscholes,
    ## "bodytrack": bodytrack,
    "facesim": facesim,
#    "ferret": ferret,
    "fluidanimate": fluidanimate,
#    "freqmine": freqmine,
#    "raytrace": raytrace,
    "swaptions": swaptions,
#    "vips": vips,
#    "x264": par_x264,
    "canneal": canneal,
    "dedup": dedup,
    "streamcluster": streamcluster
}

exp_list = {
    "SPEC": SPEC,
    "NPB": NPB,
    "PARSEC": PARSEC
}
