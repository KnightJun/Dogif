import psutil, os, shutil, zipfile
def get_pid(pname):
    for proc in psutil.process_iter():
        if proc.name() == pname:
            return proc.pid
    return None

def get_dependency(filename):
    f = open(filename, "r")
    flist = f.read().replace('\\', '/').splitlines()
    flist = [x for x in flist if x != ""]
    return [ x for x in flist if x[0] != "!"], [ x[1:] for x in flist if x[0] == "!"]

def clean_dir(path):
    try:
        shutil.rmtree(path)
    except:
        None
    os.mkdir(path)

def match_path(targetFile, mapsList):
    pathMap = {}
    for tFile in targetFile:
        if(tFile[-1] == '/'):
            continue
        sLen = len(tFile)
        matchPath = ""
        for mapEle in mapsList:
            fPath = mapEle.path.replace('\\', '/')
            if fPath[-sLen:] == tFile:
                matchPath = fPath
        if(matchPath):
            pathMap[tFile] = matchPath
        else:
            print("can't find " + tFile)
            exit(-1)
    return pathMap

def collectFiles(mapPaths, depList, targetDir):
    clean_dir(targetDir)
    for depFile in depList:
        absPath = os.path.join(targetDir, depFile)
        if(depFile[-1] == '/'):
            os.mkdir(absPath)
        else:
            shutil.copyfile(mapPaths[depFile], absPath)

def collectFilesAttach(attList, targetDir):
    for depFile in attList:
        absPath = os.path.join(targetDir, depFile)
        if(depFile[-1] == '/'):
            os.mkdir(absPath)
        else:
            shutil.copyfile(depFile, absPath)

def PackToZip(absDir,zipF, fileList):
    zipF = zipfile.ZipFile(zipF,"w",zipfile.ZIP_DEFLATED) 
    cwd = os.getcwd()
    os.chdir(absDir)
    for relfile in fileList:
        zipF.write(relfile)
    os.chdir(cwd)

def listDependFile(mapsList):
    for mapEle in mapsList:
        print(mapEle.path)

targetProcess = "Dogif.exe"
pid = get_pid(targetProcess)
if(not pid):
    print("Can't find target process:" + targetProcess)
    exit(-1)
p = psutil.Process(pid)
depList, attList = get_dependency("./Packing/Dependency.txt")
installPath = "./Packing/Install/"
mapPaths = match_path(depList, p.memory_maps())
collectFiles(mapPaths, depList, installPath)
collectFilesAttach(attList, installPath)
PackToZip(installPath, "./Packing/Output/Dogif.zip", [x for x in depList if x[-1] != '/'])
