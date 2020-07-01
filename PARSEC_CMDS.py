blackscholes = {
    1 : "1 in_10M.txt prices.txt"
}
bodytrack = {
    1 : "sequenceB_261 4 261 4000 5 0 1"
}
facesim = {
    1 : "-timing -threads 1 -lastframe 100"
}
ferret = {
    1 : "TODO"
}
fluidanimate = {
    1 : "1 500 in_500K.fluid out.fluid"
}
freqmine = {
    1 : "webdocs_250k.dat 11000"
}
raytrace = {
    1 : "thai_statue.obj -automove -nthreads 1 -frames 200 -res 1920 1080"
}
swaptions = {
    1 : "-ns 128 -sm 1000000 -nt 1"
}
vips = {
    1 : "im_benchmark orion_18000x18000.v output.v"
}
x264 = {
    1 : "--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads 1 -o eledream.264 eledream_1920x1080_512.y4m"
}
canneal = {
    1 : "1 15000 2000 2500000.nets 6000"
}
dedup = {
    1 : "-c -p -v -t 1 -i FC-6-x86_64-disc1.iso -o output.dat.ddp"
}
streamcluster = {
    1 : "10 20 128 1000000 200000 5000 none output.txt 1"
}

parsec_cmd = {
    "blackscholes": blackscholes,
    "bodytrack": bodytrack,
    "facesim": facesim,
    "ferret": ferret,
    "fluidanimate": fluidanimate,
    "freqmine": freqmine,
    "raytrace": raytrace,
    "swaptions": swaptions,
    "vips": vips,
    "x264": x264,
    "canneal": canneal,
    "dedup": dedup,
    "streamcluster": streamcluster
}