
add_rules("mode.debug", "mode.release")
add_repositories("KnightJun-repo git@github.com:knightjun/xmake-repo.git main")

add_requires("GoatScreenShot main", {debug = is_mode("debug")})
add_requires("SalmonActions main" , {debug = is_mode("debug")})
add_requires("KoiAnimation main"  , {debug = is_mode("debug")})
add_requires("OwlKeyLog main"  , {debug = is_mode("debug")})
add_requires("Breakpad"  , {debug = is_mode("debug")})
add_requires("SingleApplication"  , {debug = is_mode("debug")})
add_requires("spdlog"  , {debug = is_mode("debug")})
add_requires("fervor autoupdate", {debug = is_mode("debug")})
add_requires("DeerShortcut main"  , {debug = is_mode("debug")})

add_includedirs("include")

rule("VerUpdate")
    before_build(function (target)
        import("core.project.config")
        local buildVerFile = path.join(config.buildir(), target:name().."_build_ver")
        local buildVer = -1
        if os.isfile(buildVerFile) then 
            buildVer = tonumber(io.readfile(buildVerFile))
        end 
        buildVer = tostring(buildVer + 1)
        io.writefile(buildVerFile, buildVer)
        target:add("defines", "VER_Build="..buildVer)
    end)

target("DynamicShotWidget")
    add_rules("qt.static")
    add_packages("RobinLog")
    add_defines("DynamicShotWidget_LIB")
    add_frameworks("QtWidgets", "QtGui")
    add_files("DynamicShotWidget/DynamicShotWidget.cpp") 
    add_files("include/DynamicShotWidget.h")
    add_headerfiles("include/DynamicShotWidget.h")

target("Dogif")
    add_rules("qt.widgetapp")
    add_frameworks("QtWidgets", "QtGui", "QtNetwork")
    -- add_rules("VerUpdate")
    add_deps("DynamicShotWidget")
    add_packages("GoatScreenShot", "SalmonActions", "KoiAnimation", "OwlKeyLog")
    add_packages("DeerShortcut", "SingleApplication")
    add_packages("Breakpad", "fervor", "spdlog", "DeerShortcut")
    add_links("Dwmapi")
    add_defines("UNICODE")
    add_files("src/*.cpp") 
    add_files("src/KeySequenceEdit.h")
    add_files("PlayProgress/PlayProgress.cpp")
    add_files("include/OptionWindow.h")
    add_files("include/AnimationShotWidget.h")
    add_files("include/PlayProgress.h")
    add_files("optionUi/OptionWindow.ui")
    add_files("optionUi/res.qrc")
    add_files("src/app.rc")

target("Packing")
    set_default(false)
    on_build(function (target)
        os.execv("python.exe", {".\\Packing\\Packing.py"})
        os.execv("c:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe", {".\\Packing\\setup.iss"})
    end)