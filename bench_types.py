from SPEC_CMDS import abreviate_spec_name, launch_cmd_rate
from PARSEC_CMDS import parsec_cmd
from BENCH_CONF import PATHS

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
        self.PATH = PATHS["SPEC"]
        self.config = conf
        self.app = abreviate_spec_name[app]
    def getExecPath(self):
        return "%s/benchspec/CPU/%s/run/run_base_refrate_%s-m64.0000/" % (self.PATH, self.app, self.config)
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
        return "./%s_base.%s-m64 %s" % (self.getNameApp(self.app), self.config, launch_cmd_rate[self.app][int(cmd)])

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
        return "%s/pkgs/apps/%s/inst/amd64-linux.gcc/bin" % (self.PATH, self.app)
    def getNameApp(self, app_name):
        if ("raytrace" in app_name):
            return "rtview"
        else:
            return app_name
    def getExecCmd(self, cmd):
        return "./%s %s" % (self.getNameApp(self.app), parsec_cmd[self.app][int(cmd)])

