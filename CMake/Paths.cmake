set(buildSystemDirectory ${PROJECT_SOURCE_DIR}/BuildSystem)
set(builder "${buildSystemDirectory}/Build")
set(functionMapDirectory "${buildSystemDirectory}/BrawlFuncMap.map")
set(librariesDirectory ${PROJECT_SOURCE_DIR}/Libraries)
set(globalsDirectory ${PROJECT_SOURCE_DIR}/Globals)
set(codesDirectory ${PROJECT_SOURCE_DIR}/Codes)

set(PPC_BIN_DIRECTORY "/opt/devkitpro/devkitPPC/bin/")
set(PPC_COMPILER powerpc-eabi-g++)
set(PPCC_COMPILER powerpc-eabi-gcc)
set(PPC_LINKER powerpc-eabi-ld)

if (WIN32)
    set(PPC_BIN_DIRECTORY C:\\devkitPro\\devkitPPC\\bin)
    set(PPC_COMPILER ${PPC_COMPILER}.exe)
    set(PPCC_COMPILER ${PPC_COMPILER}.exe)
    set(PPC_LINKER ${PPC_LINKER}.exe)
    set(builder "${builder}.exe")
endif (WIN32)