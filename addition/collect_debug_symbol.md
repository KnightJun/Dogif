### set env var
- add dump_syms.exe path to path env
- add DebugSymbolsPath
### change xmake source rules\c++\xmake.lua
```lua
rule("c++.debug.symbols")
    before_build(function (target, opt)
        target:set("symbols", "debug")
    end)
    after_build(function (target)
        if target:symbolfile() then
            symName = target:targetfile()..".sym"
            pdbFileName = path.filename(target:symbolfile())
            os.execv("dump_syms.exe", {target:symbolfile()},{stdout = symName}) 
            symPath = "$(env DebugSymbolsPath)"
            -- os.cp(target:symbolfile(), "$(env DebugSymbolsPath)")
            local file = io.open(symName, "r")
            if file then
                local data = file:read()
                file:close()
                _, _, hashVal = string.find(data, "%s([%u%d]+)%s"..pdbFileName)
                symTargetPath = path.join(symPath, path.filename(target:targetfile()), hashVal)
                os.mkdir(symTargetPath)
                os.cp(target:symbolfile(), symTargetPath)
                os.cp(symName, symTargetPath)
            end
        end
    end)
    
rule("c++")
    add_deps("c++.debug.symbols")
```