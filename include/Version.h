#pragma once

#define VER_Major    1
#define VER_Minor    0
#define VER_Release    0 
#ifndef VER_Build
#define VER_Build    0
#endif
#define VER_Version    MAKELONG(MAKEWORD(VER_Major, VER_Minor), VER_Release)

#define _Stringizing(v)    #v
#define _VerJoin(a, b, c)  _Stringizing(a.b.c)

#define STR_BuildDate       __DATE__
#define STR_BuildTime       __TIME__
#define STR_BuilDateTime    __DATE__ " " __TIME__
#define STR_Version         _VerJoin(VER_Major, VER_Minor, VER_Release)

#define STR_AppName         "Dogif"
#define STR_Author          "Dogif"
#define STR_Corporation     "Dogif.top"
#define STR_Web             "Dogif.top"
#define STR_Email           ""
#define STR_WebUrl          "https://Dogif.top"

#define STR_Description     "Dogif"
#define STR_Copyright       "Copyright (C) 2021-2021 " STR_Corporation ". All rights reserved."
