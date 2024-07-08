from SPEC_CMDS import abreviate_spec_name, launch_cmd_rate
from PARSEC_CMDS import parsec_cmd
from BENCH_CONF import PATHS

def create_bench(bench, app, conf=""):
    if bench == "SPEC":
        return SPEC(app, conf)
    elif bench == "PARSEC":
        return PARSEC(app)
    elif bench == "NPB":
        return NPB(app)
    elif bench == "llama":
        return LLAMA(app)
    else:
        return Bench(app)

class Bench:
    def __init__(self, app):
        self.PATH = "."
        self.app = app
    def getExecPath(self):
        return "%s/%s" % (self.PATH, self.app)
    def getExecCmd(self, cmd):
        return "./%s.%s" % (self.app, cmd)

class SPEC(Bench):
    def __init__(self, app, conf):
        print("config %s" % conf)
        print("app %s" % app)
        self.PATH = PATHS["SPEC"]
        self.config = conf
        self.app = app
        print("app %s" % self.app)
    def getExecPath(self):
        return "%s/benchspec/CPU/%s/run/run_base_refrate_%s-m64.0000/" % (self.PATH, abreviate_spec_name[self.app], self.config)
    def getNameApp(self, app_name):
        if ("gcc" in app_name):
            return "cpu"+app_name
        elif ("xalan" in app_name):
            return "cpuxalan_r"
        elif ("cactuBSSN" in app_name):
            return "cactusBSSN_r"
        else:
            return app_name
    def getExecCmd(self, cmd):
        return "./%s_r_base.%s-m64 %s" % (self.getNameApp(self.app), self.config, launch_cmd_rate[abreviate_spec_name[self.app]][int(cmd)])

class NPB(Bench):
    def __init__(self, app):
        Bench.__init__(self, app)
        self.PATH = PATHS["NPB"]
    def getExecPath(self):
        return "%s/NPB3.3-SER/bin/" % (self.PATH)
    def getExecCmd(self, cmd):
        return "./%s.%s.x" %(self.app, cmd)

class PARSEC(Bench):
    def __init__(self, app):
        Bench.__init__(self, app)
        self.PATH = PATHS["PARSEC"]
    def getExecPath(self):
        if self.app == "canneal" or self.app == "dedup" or self.app == "streamcluster":
            return "%s/pkgs/kernels/%s/inst/amd64-linux.gcc-serial/bin" % (self.PATH, self.app)
        else:
            return "%s/pkgs/apps/%s/inst/amd64-linux.gcc-serial/bin" % (self.PATH, self.app)
    def getNameApp(self, app_name):
        if ("raytrace" in app_name):
            return "rtview"
        else:
            return app_name
    def getExecCmd(self, cmd):
        return "./%s %s" % (self.getNameApp(self.app), parsec_cmd[self.app][int(cmd)])

class LLAMA(Bench):
    def __init__(self, app):
        Bench.__init__(self, app)
        self.PATH = PATHS["LLAMA"]
    def getExecPath(self):
        return self.PATH
    def getNameApp(self):
        return "llama-cli"
    def getExecCmd(self, cmd):
        #number of threads in cmd
        return "./%s -m %s -t %s -n 4096 -p \"hola\"" % (self.getNameApp(), self.app, cmd)


