AC_PREREQ([2.69])
AC_INIT([vkd3d],[1.11])

AC_CONFIG_AUX_DIR([bin])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS(include/config.h)

AC_ARG_VAR([WIDL], [widl IDL compiler])
AC_ARG_VAR([CROSSCC32], [32-bit Windows cross compiler])
AC_ARG_VAR([CROSSCC64], [64-bit Windows cross compiler])
AC_ARG_WITH([ncurses], AS_HELP_STRING([--with-ncurses], [Build with the ncurses library (default: test)]))
AC_ARG_WITH([opengl], AS_HELP_STRING([--with-opengl], [Build with the OpenGL library (default: test)]))
AC_ARG_WITH([spirv-tools], AS_HELP_STRING([--with-spirv-tools],
                                          [Build with SPIRV-Tools library (default: disabled)]))
AC_ARG_WITH([xcb], AS_HELP_STRING([--with-xcb], [Build with XCB library (default: test)]))
AC_ARG_ENABLE([demos],
              AS_HELP_STRING([--enable-demos], [Build demo programs (default: disabled)]),,
              [enable_demos=no])
AC_ARG_ENABLE([tests],
              AS_HELP_STRING([--enable-tests], [Build tests (default: enabled)]),,
              [enable_tests=yes])

AC_USE_SYSTEM_EXTENSIONS

dnl Check for progs
AM_PROG_AR
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_SED
AC_PROG_MKDIR_P
VKD3D_PROG_WIDL(3, 21)
AS_IF([test "x$WIDL" = "xno"], [AC_MSG_WARN([widl is required to build header files.])])

AC_CHECK_PROGS([FLEX], [flex], [none])
AS_IF([test "$FLEX" = "none"], [AC_MSG_ERROR([no suitable flex found. Please install the 'flex' package.])])
AC_ARG_VAR([LFLAGS], [extra flags for flex])

AC_CHECK_PROGS([BISON], [bison], [none])
AS_IF([test "$BISON" = "none"], [AC_MSG_ERROR([no suitable bison found. Please install the 'bison' package.])])
AC_ARG_VAR([YFLAGS], [extra flags for bison])

DX_PS_FEATURE([OFF])
DX_INIT_DOXYGEN([vkd3d], [Doxyfile], [doc])
AC_CONFIG_FILES([Doxyfile])

AM_INIT_AUTOMAKE([1.11 foreign silent-rules subdir-objects no-dist-gzip dist-xz -Wall -Werror])
AM_MAINTAINER_MODE([enable])
AM_SILENT_RULES([yes])

LT_PREREQ([2.4.2])
LT_INIT([win32-dll])

gl_LD_VERSION_SCRIPT

dnl Check compiler specific flags
AC_SUBST([VKD3D_CFLAGS])
AS_IF([test "x${GCC}" = "xyes"],
      [VKD3D_CFLAGS="-Wall -pipe -D_WIN32_WINNT=0x0600"
      VKD3D_CHECK_CFLAGS([-std=c99])
      VKD3D_CHECK_CFLAGS([-flto=auto])
      VKD3D_CHECK_CFLAGS([-fvisibility=hidden])
      VKD3D_CHECK_CFLAGS([-Wdeclaration-after-statement])
      VKD3D_CHECK_CFLAGS([-Wempty-body])
      VKD3D_CHECK_CFLAGS([-Wenum-conversion])
      VKD3D_CHECK_CFLAGS([-Wimplicit-fallthrough])
      VKD3D_CHECK_CFLAGS([-Winit-self])
      VKD3D_CHECK_CFLAGS([-Wmissing-prototypes])
      VKD3D_CHECK_CFLAGS([-Wpointer-arith])
      VKD3D_CHECK_CFLAGS([-Wshadow])
      VKD3D_CHECK_CFLAGS([-Wshift-overflow=2])
      VKD3D_CHECK_CFLAGS([-Wtype-limits])
      VKD3D_CHECK_CFLAGS([-Wunused-but-set-parameter])
      VKD3D_CHECK_CFLAGS([-Wvla])
      VKD3D_CHECK_CFLAGS([-Wwrite-strings])
      VKD3D_CHECK_CFLAGS([-Wl,--no-undefined])])

dnl Check for cross compilers
VKD3D_CHECK_MINGW32_PROG([CROSSCC32], [CROSSTARGET32], [no])
VKD3D_CHECK_MINGW64_PROG([CROSSCC64], [CROSSTARGET64], [no])

dnl Check for headers
AC_CHECK_HEADERS([dlfcn.h \
                  vulkan/vulkan.h \
                  vulkan/spirv.h vulkan/GLSL.std.450.h \
                  spirv/unified1/spirv.h spirv/unified1/GLSL.std.450.h])
AS_IF([test "x$ac_cv_header_vulkan_vulkan_h" != "xyes"], [AC_MSG_ERROR([vulkan.h not found.])])
AS_IF([test "x$ac_cv_header_spirv_unified1_spirv_h" != "xyes" \
       -a "x$ac_cv_header_vulkan_spirv_h" != "xyes"],
      [AC_MSG_ERROR([spirv.h not found.])])
AS_IF([test "x$ac_cv_header_spirv_unified1_GLSL_std_450_h" != "xyes" \
       -a "x$ac_cv_header_vulkan_GLSL_std_450_h" != "xyes"],
      [AC_MSG_ERROR([GLSL.std.450.h not found.])])

VKD3D_CHECK_VULKAN_HEADER_VERSION([228], [AC_MSG_ERROR([Vulkan headers are too old, 1.3.228 is required.])])

AC_CHECK_DECL([SpvCapabilityDemoteToHelperInvocationEXT],, [AC_MSG_ERROR([SPIR-V headers are too old.])], [
#ifdef HAVE_SPIRV_UNIFIED1_SPIRV_H
# include "spirv/unified1/spirv.h"
#else
# include "vulkan/spirv.h"
#endif])

AC_CHECK_DECLS([program_invocation_name],,,[#include <errno.h>])

dnl Check for libraries
m4_ifdef([PKG_PROG_PKG_CONFIG], [PKG_PROG_PKG_CONFIG], [m4_fatal([pkg-config autoconf macros not found.])])

AC_CHECK_LIB([m], [ceilf])

AC_SUBST([DL_LIBS])
AC_CHECK_LIB([dl], [dlopen],
             [DL_LIBS="-ldl"],
             [AS_IF([test "$ac_cv_header_dlfnc_h" = "xyes"], [AC_MSG_ERROR([libdl not found.])])])

case $host_os in
  mingw32*) ;;
  *) AC_CHECK_HEADERS([pthread.h])
     AS_IF([test "x$ac_cv_header_pthread_h" != "xyes"], [AC_MSG_ERROR([pthread.h not found.])])
     AC_ARG_VAR([PTHREAD_LIBS], [linker flags for pthreads])
     VKD3D_CHECK_PTHREAD
     VKD3D_CHECK_PTHREAD_SETNAME_NP
     ;;
esac

AC_ARG_VAR([SONAME_LIBVULKAN], [shared object name for the Vulkan library])
AC_ARG_VAR([VULKAN_LIBS], [linker flags for the Vulkan library])
AS_IF([test "x$SONAME_LIBVULKAN" = "x"],
      [VKD3D_CHECK_VULKAN],
      [AC_DEFINE_UNQUOTED([SONAME_LIBVULKAN],["$SONAME_LIBVULKAN"],[Define to the shared object name of the Vulkan library.])])

AC_ARG_VAR([SONAME_LIBDXCOMPILER], [shared object name for the dxcompiler library])
AC_ARG_VAR([DXCOMPILER_LIBS], [linker flags for the dxcompiler library])
AS_IF([test "x$SONAME_LIBDXCOMPILER" = "x"],
      [VKD3D_CHECK_SONAME([dxcompiler], [DxcCreateInstance], [HAVE_DXCOMPILER=yes], [HAVE_DXCOMPILER=no], [$DXCOMPILER_LIBS])],
      [AC_DEFINE_UNQUOTED([SONAME_LIBDXCOMPILER],["$SONAME_LIBDXCOMPILER"],[Define to the shared object name of the dxcompiler library.])])

AS_IF([test "x$with_ncurses" != "xno"],
      [PKG_CHECK_MODULES([NCURSES], [ncurses],
                         [AC_DEFINE([HAVE_NCURSES], [1], [Define to 1 if you have ncurses.]) with_ncurses=yes],
                         [with_ncurses=no])])
AS_IF([test "x$with_opengl" != "xno"],
      [PKG_CHECK_MODULES([OPENGL], [egl opengl],
                         [AC_DEFINE([HAVE_OPENGL], [1], [Define to 1 if you have OpenGL.]) with_opengl=yes],
                         [with_opengl=no])])
AS_IF([test "x$with_spirv_tools" = "xyes"],
      [PKG_CHECK_MODULES([SPIRV_TOOLS], [SPIRV-Tools-shared],
      [AC_DEFINE([HAVE_SPIRV_TOOLS], [1], [Define to 1 if you have SPIRV-Tools.])])],
      [with_spirv_tools=no])

HAVE_XCB=no
AS_IF([test "x$with_xcb" != "xno"],
      [PKG_CHECK_MODULES([XCB], [xcb xcb-event xcb-icccm xcb-keysyms],
                                [AC_DEFINE([HAVE_XCB], [1], [Define to 1 if you have libxcb.])
                                HAVE_XCB=yes],
                                [HAVE_XCB=no])])

dnl Check for functions
VKD3D_CHECK_FUNC([HAVE_BUILTIN_CLZ], [__builtin_clz], [__builtin_clz(0)])
VKD3D_CHECK_FUNC([HAVE_BUILTIN_POPCOUNT], [__builtin_popcount], [__builtin_popcount(0)])
VKD3D_CHECK_FUNC([HAVE_BUILTIN_ADD_OVERFLOW], [__builtin_add_overflow], [__builtin_add_overflow(0, 0, (int *)0)])
VKD3D_CHECK_FUNC([HAVE_SYNC_ADD_AND_FETCH], [__sync_add_and_fetch], [__sync_add_and_fetch((int *)0, 0)])
VKD3D_CHECK_FUNC([HAVE_SYNC_BOOL_COMPARE_AND_SWAP], [__sync_bool_compare_and_swap], [__sync_bool_compare_and_swap((int *)0, 0, 0)])
VKD3D_CHECK_FUNC([HAVE_ATOMIC_EXCHANGE_N], [__atomic_exchange_n], [__atomic_exchange_n((int *)0, 0, 0)])
AC_CHECK_FUNCS([gettid])

dnl Makefiles
case $host_os in
  mingw32*)
      AC_SUBST([DEMO_LIBS],["-ld3d12 -ldxgi -lgdi32"])
      AC_SUBST([DEMO_CFLAGS],["-municode"])
      ;;
  *) AS_IF([test "x$enable_demos" = "xyes" -a "x$HAVE_XCB" != "xyes"],
      [AC_MSG_ERROR([libxcb is required for demos.])])
      AC_SUBST([DEMO_LIBS],[$XCB_LIBS])
      AC_SUBST([DEMO_CFLAGS],[$XCB_CFLAGS])
      ;;
esac
AM_CONDITIONAL([BUILD_DEMOS], [test "x$enable_demos" = "xyes"])
AM_CONDITIONAL([BUILD_DOC], [test $DX_FLAG_doc = 1])
AM_CONDITIONAL([BUILD_TESTS], [test "x$enable_tests" != "xno"])
AM_CONDITIONAL([HAVE_WIDL], [test "x$WIDL" != "xno"])
AM_CONDITIONAL([HAVE_CROSSTARGET32], [test "x$CROSSTARGET32" != "xno"])
AM_CONDITIONAL([HAVE_CROSSTARGET64], [test "x$CROSSTARGET64" != "xno"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

dnl Output configuration summary
AS_IF([test "x$CROSSTARGET32" != "xno" -o "x$CROSSTARGET64" != "xno"],
      [HAVE_CROSSTARGET=yes], [HAVE_CROSSTARGET=no])
AS_IF([test $DX_FLAG_doc = 1], [HAVE_DOCS=yes], [HAVE_DOCS=no])

AS_IF([test "x$enable_demos" != "xyes" -a "x$enable_tests" = "xno"], [CROSSTARGET64=no CROSSTARGET32=no])

AS_ECHO(["
  Configuration summary for $PACKAGE $VERSION

  widl: ${WIDL}

  Have ncurses: ${with_ncurses}
  Have OpenGL: ${with_opengl}
  Have SPIRV-Tools: ${with_spirv_tools}
  Have xcb: ${HAVE_XCB}
  Have dxcompiler: ${HAVE_DXCOMPILER}

  Building demos: ${enable_demos}
  Building tests: ${enable_tests}
  Building documentation: ${HAVE_DOCS}
"])

AS_IF([test "x$enable_demos" = "xyes"], [AS_ECHO(["  Cross-compiling demos: ${HAVE_CROSSTARGET}"])])
AS_IF([test "x$enable_tests" != "xno"], [AS_ECHO(["  Cross-compiling tests: ${HAVE_CROSSTARGET}"])])
AS_IF([test "x$CROSSTARGET32" != "xno"], [AS_ECHO(["    Using 32-bit cross compiler: $CROSSCC32"])])
AS_IF([test "x$CROSSTARGET64" != "xno"], [AS_ECHO(["    Using 64-bit cross compiler: $CROSSCC64"])])
AS_ECHO([])
