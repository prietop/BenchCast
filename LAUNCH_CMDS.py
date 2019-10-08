perlbench = {
    1 : "-I./lib checkspam.pl 2500 5 25 11 150 1 1 1 1",
    2 : "-I./lib diffmail.pl 4 800 10 17 19 300",
    3 : "-I./lib splitmail.pl 6400 12 26 16 100 0"
}
gcc = {
    1 : "gcc-pp.c -O3 -finline-limit=0 -fif-conversion -fif-conversion2 -o gcc-pp.opts-O3_-finline-limit_0_-fif-conversion_-fif-conversion2.s",
    2 : "gcc-pp.c -O2 -finline-limit=36000 -fpic -o gcc-pp.opts-O2_-finline-limit_36000_-fpic.s",
    3 : "gcc-smaller.c -O3 -fipa-pta -o gcc-smaller.opts-O3_-fipa-pta.s",
    4 : "ref32.c -O5 -o ref32.opts-O5.s",
    5 : "ref32.c -O3 -fselective-scheduling -fselective-scheduling2 -o ref32.opts-O3_-fselective-scheduling_-fselective-scheduling2.s"
}
bwaves = {
    1 : "bwaves_1",
    2 : "bwaves_2",
    3 : "bwaves_3",
    4 : "bwaves_4"
}
mcf = {
    1 : "inp.in"
}
cactuBSSN = {
    1 : "spec_ref.par"
}
namd = {
    1 : "--input apoa1.input --output apoa1.ref.output --iterations 65"
}
parest = {
    1 : "ref.prm"
}
povray = {
    1 : "SPEC-benchmark-ref.ini"
}
lbm = {
    1 : "3000 reference.dat 0 0 100_100_130_ldc.of"
}
omnetpp = {
    1 : "-c General -r 0"
}
wrf = {
    1 : ""
}
xalanbmk = {
    1 : "-v t5.xml xalanc.xsl"
}
x264 = {
    1 : "--pass 1 --stats x264_stats.log --bitrate 1000 --frames 1000 -o BuckBunny_New.264 BuckBunny.yuv 1280x720",
    2 : "--pass 2 --stats x264_stats.log --bitrate 1000 --dumpyuv 200 --frames 1000 -o BuckBunny_New.264 BuckBunny.yuv 1280x720",
    3 : "--seek 500 --dumpyuv 200 --frames 1250 -o BuckBunny_New.264 BuckBunny.yuv 1280x720"
}
blender = {
    1 : "sh3_no_char.blend --render-output sh3_no_char_ --threads 1 -b -F RAWTGA -s 849 -e 849 -a"
}
cam4 = {
    1 : ""
}
deepsjeng = {
    1 : "ref.txt"
}
imagick = {
    1 : "-limit disk 0 refrate_input.tga -edge 41 -resample 181% -emboss 31 -colorspace YUV -mean-shift 19x19+15% -resize 30% refrate_output.tga"
}
leela = {
    1 : "ref.sgf"
}
nab = {
    1 : "1am0 1122214447 122"
}
exchange2 = {
    1 : "6"
}
fotonik3d = {
    1 : ""
}
roms = {
    1 : "< ocean_benchmark2.in.x"
}
xz = {
    1 : "cld.tar.xz 160 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 59796407 61004416 6",
    2 : "cpu2006docs.tar.xz 250 055ce243071129412e9dd0b3b69a21654033a9b723d874b2015c774fac1553d9713be561ca86f74e4f16f22e664fc17a79f30caa5ad2c04fbc447549c2810fae 23047774 23513385 6e",
    3 : "input.combined.xz 250 a841f68f38572a49d86226b7ff5baeb31bd19dc637a922a972b2e6d1257a890f6a544ecab967c313e370478c74f760eb229d4eef8a8d2836d233d3e9dd1430bf 40401484 41217675 7"
}
specrand_f = {
    1 : "1255432124 234923"
}
specrand_i = {
    1 : "1255432124 234923"
}
launch_cmd_rate = {
    "500.perlbench_r" : perlbench,
    "502.gcc_r" : gcc,
    "503.bwaves_r" : bwaves,
    "505.mcf_r" : mcf,
    "507.cactuBSSN_r" : cactuBSSN,
    "508.namd_r" : namd,
    "510.parest_r" : parest,
    "511.povray_r" : povray,
    "519.lbm_r" : lbm,
    "520.omnetpp_r" : omnetpp,
    "521.wrf_r" : wrf,
    "523.xalancbmk_r" : xalanbmk,
    "525.x264_r" : x264, 
    "526.blender_r" : blender,
    "527.cam4_r" : cam4,
    "531.deepsjeng_r" : deepsjeng,
    "538.imagick_r" : imagick,
    "541.leela_r" : leela,
    "544.nab_r" : nab,
    "548.exchange2_r" : exchange2,
    "549.fotonik3d_r" : fotonik3d,
    "554.roms_r" : roms,
    "557.xz_r" : xz,
    "997.specrand_fr" : specrand_f,
    "999.specrand_ir" : specrand_i
}

abreviate_spec_name = {
    "perlbench" : "500.perlbench_r",
    "gcc" : "502.gcc_r",
    "bwaves" : "503.bwaves_r",
    "mcf" : "505.mcf_r",
    "cactuBSSN" : "507.cactuBSSN_r",
    "namd" : "508.namd_r",
    "parest" : "510.parest_r",
    "povray" : "511.povray_r",
    "lbm" : "519.lbm_r",
    "omnetpp" : "520.omnetpp_r",
    "wrf" : "521.wrf_r",
    "xalanbmk" : "523.xalancbmk_r",
    "x264" : "525.x264_r", 
    "blender" : "526.blender_r",
    "cam4" : "527.cam4_r",
    "deepsjeng" : "531.deepsjeng_r",
    "imagick" : "538.imagick_r",
    "leela" : "541.leela_r",
    "nab" : "544.nab_r",
    "exchange2" : "548.exchange2_r",
    "fotonik3d" : "549.fotonik3d_r",
    "roms" : "554.roms_r",
    "xz" : "557.xz_r",
    "specrand_f" : "997.specrand_fr",
    "specrand_i" : "999.specrand_ir"
}

perlbench_s = {
    1 : "-I./lib checkspam.pl 2500 5 25 11 150 1 1 1 1",
    2 : "-I./lib diffmail.pl 4 800 10 17 19 300",
    3 : "-I./lib splitmail.pl 6400 12 26 16 100 0"
}
gcc_s = {
    1 : "gcc-pp.c -O5 -fipa-pta -o gcc-pp.opts-O5_-fipa-pta.s",
    2 : "gcc-pp.c -O5 -finline-limit=1000 -fselective-scheduling -fselective-scheduling2 -o gcc-pp.opts-O5_-finline-limit_1000_-fselective-scheduling_-fselective-scheduling2.s",
    3 : "gcc-pp.c -O5 -finline-limit=24000 -fgcse -fgcse-las -fgcse-lm -fgcse-sm -o gcc-pp.opts-O5_-finline-limit_24000_-fgcse_-fgcse-las_-fgcse-lm_-fgcse-sm.s"
}
bwaves_s = {
    1 : "bwaves_1",
    2 : "bwaves_2"
}
mcf_s = {
    1 : "inp.in"
}
cactuBSSN_s = {
    1 : "spec_ref.par"
}
lbm_s = {
    1 : "2000 reference.dat 0 0 200_200_260_ldc.of"
}
omnetpp_s = {
    1 : "-c General -r 0"
}
wrf_s = {
    1 : ""
}
xalanbmk_s = {
    1 : "-v t5.xml xalanc.xsl"
}
x264_s = {
    1 : "--pass 1 --stats x264_stats.log --bitrate 1000 --frames 1000 -o BuckBunny_New.264 BuckBunny.yuv 1280x720",
    2 : "--pass 2 --stats x264_stats.log --bitrate 1000 --dumpyuv 200 --frames 1000 -o BuckBunny_New.264 BuckBunny.yuv 1280x720",
    3 : "--seek 500 --dumpyuv 200 --frames 1250 -o BuckBunny_New.264 BuckBunny.yuv 1280x720"
}
cam4_s = {
    1 : ""
}
pop2_s = {
    1 : ""
}
deepsjeng_s = {
    1 : "ref.txt"
}
imagick_s = {
    1 : "-limit disk 0 refspeed_input.tga -resize 817% -rotate -2.76 -shave 540x375 -alpha remove -auto-level -contrast-stretch 1x1% -colorspace Lab -channel R -equalize +channel -colorspace sRGB -define histogram:unique-colors=false -adaptive-blur 0x5 -despeckle -auto-gamma -adaptive-sharpen 55 -enhance -brightness-contrast 10x10 -resize 30% refspeed_output.tga"
}
leela_s = {
    1 : "ref.sgf"
}
nab_s = {
    1 : "3j1n 20140317 220"
}
exchange2_s = {
    1 : "6"
}
fotonik3d_s = {
    1 : ""
}
roms_s = {
    1 : "< ocean_benchmark3.in"
}
xz_s = {
    1 : "cpu2006docs.tar.xz 6643 055ce243071129412e9dd0b3b69a21654033a9b723d874b2015c774fac1553d9713be561ca86f74e4f16f22e664fc17a79f30caa5ad2c04fbc447549c2810fae 1036078272 1111795472 4",
    2 : "cld.tar.xz 1400 19cf30ae51eddcbefda78dd06014b4b96281456e078ca7c13e1c0c9e6aaea8dff3efb4ad6b0456697718cede6bd5454852652806a657bb56e07d61128434b474 536995164 539938872 8"
}
specrand_fs = {
    1 : "1255432124 234923"
}
specrand_is = {
    1 : "1255432124 234923"
}
launch_cmd_speed = {
    "600.perlbench_s" : perlbench_s,
    "602.gcc_s" : gcc_s,
    "603.bwaves_s" : bwaves,
    "605.mcf_s" : mcf_s,
    "607.cactuBSSN_s" : cactuBSSN_s,
    "619.lbm_s" : lbm_s,
    "620.omnetpp_s" : omnetpp_s,
    "621.wrf_s" : wrf_s,
    "623.xalancbmk_s" : xalanbmk_s,
    "625.x264_s" : x264_s,
    "627.cam4_s" : cam4_s,
    "628.pop2_s" : pop2_s,
    "631.deepsjeng_s" : deepsjeng_s,
    "638.imagick_s" : imagick_s,
    "641.leela_s" : leela_s,
    "644.nab_s" : nab_s,
    "648.exchange2_s" : exchange2_s,
    "649.fotonik3d_s" : fotonik3d_s,
    "654.roms_s" : roms_s,
    "657.xz_s" : xz_s,
    "996.specrand_fs" : specrand_fs,
    "998.specrand_is" : specrand_is
}

launch_cmd = {
    "rate" : launch_cmd_rate,
    "speed" : launch_cmd_speed
}
