###############################################################################
# Makefile automatically generated by Code::Blocks IDE                        #
###############################################################################

# Project:          testunicode
# Project filename: D:\documents\Cracklock\testunicode\testunicode.cbp
# Compiler used:    Microsoft Visual C++ Toolkit 2003

### Variables used in this Makefile
Debug_CC=cl.exe
Debug_CPP=cl.exe
Debug_LD=link.exe
Debug_LIB=link.exe
Debug_RESCOMP=RC.Exe
Release_CC=cl.exe
Release_CPP=cl.exe
Release_LD=link.exe
Release_LIB=link.exe
Release_RESCOMP=RC.Exe

### Compiler/linker options
Debug_GLOBAL_CFLAGS= /Ot
Debug_PROJECT_CFLAGS=
Debug_GLOBAL_LDFLAGS=
Debug_PROJECT_LDFLAGS=
Debug_GLOBAL_INCS= /IC:/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/include
Debug_PROJECT_INCS=
Debug_GLOBAL_LIBDIRS= /LIBPATH:C:/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/lib
Debug_PROJECT_LIBDIRS=
Debug_GLOBAL_LIBS= user32.lib gdi32.lib Advapi32.lib ole32.lib shell32.lib comdlg32.lib comctl32.lib
Debug_PROJECT_LIBS=
Release_GLOBAL_CFLAGS= /Ot
Release_PROJECT_CFLAGS=
Release_GLOBAL_LDFLAGS=
Release_PROJECT_LDFLAGS=
Release_GLOBAL_INCS= /IC:/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/include
Release_PROJECT_INCS=
Release_GLOBAL_LIBDIRS= /LIBPATH:C:/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/lib
Release_PROJECT_LIBDIRS=
Release_GLOBAL_LIBS= user32.lib gdi32.lib Advapi32.lib ole32.lib shell32.lib comdlg32.lib comctl32.lib
Release_PROJECT_LIBS=

### Targets compiler flags
Debug_CFLAGS= $(Debug_PROJECT_CFLAGS) /MTd /W3 /O0 /DWIN32 /D_DEBUG /D_WINDOWS $(Debug_GLOBAL_CFLAGS)
Release_CFLAGS= $(Release_PROJECT_CFLAGS) /MT /O2 /W3 /Zi /Ob1 /DWIN32 /DNDEBUG /D_WINDOWS $(Release_GLOBAL_CFLAGS)

### Targets linker flags
Debug_LDFLAGS= $(Debug_PROJECT_LDFLAGS) /pdb:Debug\testunicode.pdb $(Debug_GLOBAL_LDFLAGS)
Release_LDFLAGS= $(Release_PROJECT_LDFLAGS) /DEBUG /INCREMENTAL:NO $(Release_GLOBAL_LDFLAGS)

### Targets include directories
Debug_INCS= $(Debug_PROJECT_INCS) $(Debug_GLOBAL_INCS)
Release_INCS= $(Release_PROJECT_INCS) $(Release_GLOBAL_INCS)

### Targets library directories
Debug_LIBDIRS= $(Debug_PROJECT_LIBDIRS) $(Debug_GLOBAL_LIBDIRS)
Release_LIBDIRS= $(Release_PROJECT_LIBDIRS) $(Release_GLOBAL_LIBDIRS)

### Targets libraries
Debug_LIBS= $(Debug_PROJECT_LIBS) $(Debug_GLOBAL_LIBS)
Release_LIBS= $(Release_PROJECT_LIBS) $(Release_GLOBAL_LIBS)

###############################################################################
#         You shouldn't need to modify anything beyond this point             #
###############################################################################

### Resources used in this Makefile
Debug_RESOURCE=Debug_private.res
Release_RESOURCE=Release_private.res

### Objects used in this Makefile
Debug_OBJS=Debug/testunicode.obj 
Debug_LINKOBJS=$(Debug_OBJS)
Release_OBJS=Release/testunicode.obj 
Release_LINKOBJS=$(Release_OBJS)

### The targets of this project
Debug_BIN=Debug/testunicode.exe
Release_BIN=Release/testunicode.exe

.PHONY: all all-before all-custom all-after clean clean-custom distclean distclean-custom depend_Debug Debug-before Debug-after depend_Release Release-before Release-after 

all: all-before Debug Release all-after


dist:
	@zip testunicode.cbp.zip testunicode.cbp Makefile ReadMe.txt resource.h testunicode.cpp testunicode.rc 

clean_Debug:
	$(RM) $(Debug_BIN) $(Debug_OBJS) $(Debug_RESOURCE) 

distclean_Debug:
	$(RM) $(Debug_BIN) $(Debug_OBJS) $(Debug_DEPS) $(Debug_RESOURCE) 

clean_Release:
	$(RM) $(Release_BIN) $(Release_OBJS) $(Release_RESOURCE) 

distclean_Release:
	$(RM) $(Release_BIN) $(Release_OBJS) $(Release_DEPS) $(Release_RESOURCE) 

clean: clean_Debug clean_Release 

distclean: distclean_Debug distclean_Release 

depend:

Debug_DIRS:
	-@if not exist "Debug\." mkdir "Debug"
	-@if not exist "Debug\." mkdir "Debug"

Debug: depend_Debug Debug_DIRS Debug-before $(Debug_BIN) Debug-after

$(Debug_BIN): $(Debug_LINKOBJS) $(Debug_RESOURCE)
	$(Debug_LD) /nologo /subsystem:windows $(Debug_LIBDIRS) /out:$(Debug_BIN) $(Debug_LIBS) $(Debug_LINKOBJS) $(Debug_RESOURCE) $(Debug_LDFLAGS)

Release_DIRS:
	-@if not exist "Release\." mkdir "Release"
	-@if not exist "Release\." mkdir "Release"

Release: depend_Release Release_DIRS Release-before $(Release_BIN) Release-after

$(Release_BIN): $(Release_LINKOBJS) $(Release_RESOURCE)
	$(Release_LD) /nologo /subsystem:windows $(Release_LIBDIRS) /out:$(Release_BIN) $(Release_LIBS) $(Release_LINKOBJS) $(Release_RESOURCE) $(Release_LDFLAGS)


Debug/testunicode.obj: testunicode.cpp
	$(Debug_CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /c testunicode.cpp /FoDebug/testunicode.obj

$(Debug_RESOURCE): 
	$(Debug_RESCOMP)  /ID:/Microsoft\ Platform\ SDK/Include/mfc -fo$(Debug_RESOURCE) Debug_private.rc


Release/testunicode.obj: testunicode.cpp
	$(Release_CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /c testunicode.cpp /FoRelease/testunicode.obj

$(Release_RESOURCE): 
	$(Release_RESCOMP)  /ID:/Microsoft\ Platform\ SDK/Include/mfc -fo$(Release_RESOURCE) Release_private.rc



