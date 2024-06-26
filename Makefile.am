ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = @VKD3D_CFLAGS@
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/include/private -I$(builddir)/include/private
AM_LDFLAGS = -no-undefined

widl_headers = \
	include/vkd3d_d3d12.h \
	include/vkd3d_d3d12sdklayers.h \
	include/vkd3d_d3d12shader.h \
	include/vkd3d_d3dcommon.h \
	include/vkd3d_d3dx9shader.h \
	include/vkd3d_dxgi.h \
	include/vkd3d_dxgi1_2.h \
	include/vkd3d_dxgi1_3.h \
	include/vkd3d_dxgi1_4.h \
	include/vkd3d_dxgibase.h \
	include/vkd3d_dxgiformat.h \
	include/vkd3d_dxgitype.h \
	tests/dxcompiler.h

vkd3d_public_headers = \
	include/vkd3d.h \
	include/vkd3d_d3d12.h \
	include/vkd3d_d3d12sdklayers.h \
	include/vkd3d_d3d12shader.h \
	include/vkd3d_d3d9types.h \
	include/vkd3d_d3dcommon.h \
	include/vkd3d_d3dcompiler.h \
	include/vkd3d_d3dcompiler_types.h \
	include/vkd3d_d3dx9shader.h \
	include/vkd3d_dxgibase.h \
	include/vkd3d_dxgiformat.h \
	include/vkd3d_shader.h \
	include/vkd3d_types.h \
	include/vkd3d_utils.h \
	include/vkd3d_windows.h

vkd3d_tests = \
	tests/vkd3d_api \
	tests/vkd3d_common \
	tests/vkd3d_shader_api

vkd3d_cross_tests = \
	tests/d3d12 \
	tests/d3d12_invalid_usage \
	tests/hlsl_d3d12

vkd3d_shader_tests = \
	tests/hlsl/abs.shader_test \
	tests/hlsl/all.shader_test \
	tests/hlsl/angle-unit.shader_test \
	tests/hlsl/annotations.shader_test \
	tests/hlsl/any.shader_test \
	tests/hlsl/arithmetic-float-uniform.shader_test \
	tests/hlsl/arithmetic-float.shader_test \
	tests/hlsl/arithmetic-int-uniform.shader_test \
	tests/hlsl/arithmetic-int.shader_test \
	tests/hlsl/arithmetic-uint.shader_test \
	tests/hlsl/array-dimension.shader_test \
	tests/hlsl/array-parameters.shader_test \
	tests/hlsl/array-size-expr.shader_test \
	tests/hlsl/asfloat.shader_test \
	tests/hlsl/asuint.shader_test \
	tests/hlsl/attributes.shader_test \
	tests/hlsl/bitwise.shader_test \
	tests/hlsl/bool-cast.shader_test \
	tests/hlsl/bool-semantics.shader_test \
	tests/hlsl/calculate-lod.shader_test \
	tests/hlsl/cast-64-bit.shader_test \
	tests/hlsl/cast-broadcast.shader_test \
	tests/hlsl/cast-componentwise-compatible.shader_test \
	tests/hlsl/cast-componentwise-equal.shader_test \
	tests/hlsl/cast-to-float.shader_test \
	tests/hlsl/cast-to-half.shader_test \
	tests/hlsl/cast-to-int.shader_test \
	tests/hlsl/cast-to-uint.shader_test \
	tests/hlsl/cbuffer.shader_test \
	tests/hlsl/ceil.shader_test \
	tests/hlsl/cf-cond-types.shader_test \
	tests/hlsl/clamp.shader_test \
	tests/hlsl/clip-distance.shader_test \
	tests/hlsl/clip.shader_test \
	tests/hlsl/combined-samplers.shader_test \
	tests/hlsl/comma.shader_test \
	tests/hlsl/compute.shader_test \
	tests/hlsl/conditional.shader_test \
	tests/hlsl/const.shader_test \
	tests/hlsl/coverage.shader_test \
	tests/hlsl/cross.shader_test \
	tests/hlsl/d3dcolor-to-ubyte4.shader_test \
	tests/hlsl/ddxddy.shader_test \
	tests/hlsl/default-values.shader_test \
	tests/hlsl/depth-out.shader_test \
	tests/hlsl/determinant.shader_test \
	tests/hlsl/discard.shader_test \
	tests/hlsl/distance.shader_test \
	tests/hlsl/dot.shader_test \
	tests/hlsl/duplicate-modifiers.shader_test \
	tests/hlsl/effect-compile.shader_test \
	tests/hlsl/effect-shader-objects-fx_2.shader_test \
	tests/hlsl/effect-shader-objects-fx_5.shader_test \
	tests/hlsl/effect-technique-fx_2.shader_test \
	tests/hlsl/effect-technique-fx_4.shader_test \
	tests/hlsl/effect-technique-fx_5.shader_test \
	tests/hlsl/effect-variables-fx_2.shader_test \
	tests/hlsl/entry-point-semantics.shader_test \
	tests/hlsl/exp.shader_test \
	tests/hlsl/expr-indexing.shader_test \
	tests/hlsl/float-comparison.shader_test \
	tests/hlsl/floor.shader_test \
	tests/hlsl/fmod.shader_test \
	tests/hlsl/for.shader_test \
	tests/hlsl/frac.shader_test \
	tests/hlsl/function-cast.shader_test \
	tests/hlsl/function-overload.shader_test \
	tests/hlsl/function-return.shader_test \
	tests/hlsl/function.shader_test \
	tests/hlsl/fwidth.shader_test \
	tests/hlsl/fxgroup-syntax.shader_test \
	tests/hlsl/gather-offset.shader_test \
	tests/hlsl/gather.shader_test \
	tests/hlsl/geometry.shader_test \
	tests/hlsl/get-sample-pos.shader_test \
	tests/hlsl/getdimensions.shader_test \
	tests/hlsl/half.shader_test \
	tests/hlsl/hard-copy-prop.shader_test \
	tests/hlsl/initializer-flatten.shader_test \
	tests/hlsl/initializer-implicit-array.shader_test \
	tests/hlsl/initializer-invalid-arg-count.shader_test \
	tests/hlsl/initializer-local-array.shader_test \
	tests/hlsl/initializer-matrix.shader_test \
	tests/hlsl/initializer-multi.shader_test \
	tests/hlsl/initializer-nested.shader_test \
	tests/hlsl/initializer-numeric.shader_test \
	tests/hlsl/initializer-objects.shader_test \
	tests/hlsl/initializer-static-array.shader_test \
	tests/hlsl/initializer-struct.shader_test \
	tests/hlsl/intrinsic-override.shader_test \
	tests/hlsl/instance-id.shader_test \
	tests/hlsl/invalid.shader_test \
	tests/hlsl/inverse-trig.shader_test \
	tests/hlsl/is-front-face.shader_test \
	tests/hlsl/ldexp.shader_test \
	tests/hlsl/length.shader_test \
	tests/hlsl/lerp.shader_test \
	tests/hlsl/lit.shader_test \
	tests/hlsl/load-level.shader_test \
	tests/hlsl/log.shader_test \
	tests/hlsl/logic-operations.shader_test \
	tests/hlsl/loop.shader_test \
	tests/hlsl/majority-pragma.shader_test \
	tests/hlsl/majority-syntax.shader_test \
	tests/hlsl/majority-typedef.shader_test \
	tests/hlsl/math.shader_test \
	tests/hlsl/matrix-indexing.shader_test \
	tests/hlsl/matrix-semantics.shader_test \
	tests/hlsl/max-min.shader_test \
	tests/hlsl/minimum-precision.shader_test \
	tests/hlsl/mul.shader_test \
	tests/hlsl/multiple-rt.shader_test \
	tests/hlsl/nested-arrays.shader_test \
	tests/hlsl/nointerpolation.shader_test \
	tests/hlsl/non-const-indexing.shader_test \
	tests/hlsl/normalize.shader_test \
	tests/hlsl/numeric-constructor-truncation.shader_test \
	tests/hlsl/numeric-types.shader_test \
	tests/hlsl/numthreads.shader_test \
	tests/hlsl/object-field-offsets.shader_test \
	tests/hlsl/object-parameters.shader_test \
	tests/hlsl/object-references.shader_test \
	tests/hlsl/pointer-cast.shader_test \
	tests/hlsl/pow.shader_test \
	tests/hlsl/precise-modifier.shader_test \
	tests/hlsl/rasteriser-ordered-views.shader_test \
	tests/hlsl/reflect.shader_test \
	tests/hlsl/refract.shader_test \
	tests/hlsl/register-reservations-numeric.shader_test \
	tests/hlsl/register-reservations-profile.shader_test \
	tests/hlsl/register-reservations-resources.shader_test \
	tests/hlsl/register-reservations-space.shader_test \
	tests/hlsl/return-implicit-conversion.shader_test \
	tests/hlsl/return.shader_test \
	tests/hlsl/round.shader_test \
	tests/hlsl/rt-get-sample-info.shader_test \
	tests/hlsl/sample-bias.shader_test \
	tests/hlsl/sample-cmp.shader_test \
	tests/hlsl/sample-grad.shader_test \
	tests/hlsl/sample-level.shader_test \
	tests/hlsl/sampler-offset.shader_test \
	tests/hlsl/sampler.shader_test \
	tests/hlsl/saturate.shader_test \
	tests/hlsl/shader-interstage-interface.shader_test \
	tests/hlsl/shape.shader_test \
	tests/hlsl/side-effects.shader_test \
	tests/hlsl/sign.shader_test \
	tests/hlsl/single-numeric-initializer.shader_test \
	tests/hlsl/sm1-const-folding.shader_test \
	tests/hlsl/sm1-const-allocation.shader_test \
	tests/hlsl/sm6-ternary.shader_test \
	tests/hlsl/sm6-uav-rwtexture.shader_test \
	tests/hlsl/smoothstep.shader_test \
	tests/hlsl/sqrt.shader_test \
	tests/hlsl/srv-buffers.shader_test \
	tests/hlsl/state-block-function-syntax.shader_test \
	tests/hlsl/state-block-syntax.shader_test \
	tests/hlsl/static-initializer.shader_test \
	tests/hlsl/step.shader_test \
	tests/hlsl/storage-qualifiers.shader_test \
	tests/hlsl/strings.shader_test \
	tests/hlsl/struct-array.shader_test \
	tests/hlsl/struct-assignment.shader_test \
	tests/hlsl/struct-semantics.shader_test \
	tests/hlsl/switch.shader_test \
	tests/hlsl/swizzle-constant-prop.shader_test \
	tests/hlsl/swizzle-matrix.shader_test \
	tests/hlsl/swizzles.shader_test \
	tests/hlsl/ternary.shader_test \
	tests/hlsl/tessellation.shader_test \
	tests/hlsl/texture-load-offset.shader_test \
	tests/hlsl/texture-load-typed.shader_test \
	tests/hlsl/texture-load.shader_test \
	tests/hlsl/texture-ordering.shader_test \
	tests/hlsl/tgsm.shader_test \
	tests/hlsl/transpose.shader_test \
	tests/hlsl/trigonometry.shader_test \
	tests/hlsl/trunc.shader_test \
	tests/hlsl/type-names.shader_test \
	tests/hlsl/uav-atomics.shader_test \
	tests/hlsl/uav-counter.shader_test \
	tests/hlsl/uav-load.shader_test \
	tests/hlsl/uav-out-param.shader_test \
	tests/hlsl/uav-rwbuffer.shader_test \
	tests/hlsl/uav-rwbyteaddressbuffer.shader_test \
	tests/hlsl/uav-rwstructuredbuffer.shader_test \
	tests/hlsl/uav-rwtexture.shader_test \
	tests/hlsl/unbounded-array-5.1.shader_test \
	tests/hlsl/uniform-parameters.shader_test \
	tests/hlsl/uniform-semantics.shader_test \
	tests/hlsl/vector-indexing-uniform.shader_test \
	tests/hlsl/vector-indexing.shader_test \
	tests/hlsl/vertex-shader-ops.shader_test \
	tests/hlsl/wave-ops-float.shader_test \
	tests/hlsl/wave-ops-int.shader_test \
	tests/hlsl/wave-ops-uint.shader_test \
	tests/hlsl/wave-reconvergence.shader_test \
	tests/hlsl/writemask-assignop-0.shader_test \
	tests/hlsl/writemask-assignop-1.shader_test \
	tests/hlsl/writemask-assignop-2.shader_test \
	tests/hlsl/writemask-assignop-3.shader_test \
	tests/preproc-if-expr.shader_test \
	tests/preproc-if.shader_test \
	tests/preproc-ifdef.shader_test \
	tests/preproc-invalid.shader_test \
	tests/preproc-macro.shader_test \
	tests/preproc-misc.shader_test

vkd3d_test_headers = \
	tests/d3d12_crosstest.h \
	tests/d3d12_test_utils.h \
	tests/dxcompiler.h \
	tests/shader_runner.h \
	tests/utils.h \
	tests/vulkan_procs.h

vkd3d_demos = \
	demos/vkd3d-gears \
	demos/vkd3d-triangle

vkd3d_demos_headers = \
	demos/demo.h \
	demos/demo_win32.h \
	demos/demo_xcb.h

BUILT_SOURCES = $(widl_headers) \
	include/private/vkd3d_version.h

noinst_LTLIBRARIES = libvkd3d-common.la
libvkd3d_common_la_SOURCES = \
	libs/vkd3d-common/blob.c \
	libs/vkd3d-common/debug.c \
	libs/vkd3d-common/error.c \
	libs/vkd3d-common/memory.c \
	libs/vkd3d-common/utf8.c
libvkd3d_common_la_LIBADD = @PTHREAD_LIBS@

lib_LTLIBRARIES = libvkd3d-shader.la libvkd3d.la libvkd3d-utils.la

VKD3D_V_FLEX = $(vkd3d_v_flex_@AM_V@)
vkd3d_v_flex_ = $(vkd3d_v_flex_@AM_DEFAULT_V@)
vkd3d_v_flex_0 = @echo "  FLEX    " $@;
vkd3d_v_flex_1 =

VKD3D_V_BISON = $(vkd3d_v_bison_@AM_V@)
vkd3d_v_bison_ = $(vkd3d_v_bison_@AM_DEFAULT_V@)
vkd3d_v_bison_0 = @echo "  BISON   " $@;
vkd3d_v_bison_1 =

libs/vkd3d-shader/preproc.yy.c: libs/vkd3d-shader/preproc.l
	@$(MKDIR_P) libs/vkd3d-shader
	$(VKD3D_V_FLEX)$(FLEX) $(LFLAGS) -o $@ $<

libs/vkd3d-shader/preproc.tab.c libs/vkd3d-shader/preproc.tab.h &: libs/vkd3d-shader/preproc.y
	@$(MKDIR_P) libs/vkd3d-shader
	$(VKD3D_V_BISON)$(BISON) $(YFLAGS) -d -o libs/vkd3d-shader/preproc.tab.c $<

libs/vkd3d-shader/hlsl.yy.c: $(srcdir)/libs/vkd3d-shader/hlsl.l
	@$(MKDIR_P) libs/vkd3d-shader
	$(VKD3D_V_FLEX)$(FLEX) $(LFLAGS) -o $@ $<

libs/vkd3d-shader/hlsl.tab.c libs/vkd3d-shader/hlsl.tab.h &: libs/vkd3d-shader/hlsl.y
	@$(MKDIR_P) libs/vkd3d-shader
	$(VKD3D_V_BISON)$(BISON) $(YFLAGS) -d -o libs/vkd3d-shader/hlsl.tab.c $<

BUILT_SOURCES += \
	libs/vkd3d-shader/hlsl.tab.h \
	libs/vkd3d-shader/preproc.tab.h

vkd3d_shader_yyfiles = \
	libs/vkd3d-shader/hlsl.tab.c \
	libs/vkd3d-shader/hlsl.tab.h \
	libs/vkd3d-shader/hlsl.yy.c \
	libs/vkd3d-shader/preproc.tab.c \
	libs/vkd3d-shader/preproc.tab.h \
	libs/vkd3d-shader/preproc.yy.c

CLEANFILES = $(vkd3d_shader_yyfiles)

nodist_libvkd3d_shader_la_SOURCES = $(vkd3d_shader_yyfiles)

libvkd3d_shader_la_SOURCES = \
	include/private/list.h \
	include/private/rbtree.h \
	include/private/vkd3d_common.h \
	include/private/vkd3d_memory.h \
	include/vkd3d_shader.h \
	libs/vkd3d-shader/checksum.c \
	libs/vkd3d-shader/d3d_asm.c \
	libs/vkd3d-shader/d3dbc.c \
	libs/vkd3d-shader/dxbc.c \
	libs/vkd3d-shader/dxil.c \
	libs/vkd3d-shader/fx.c \
	libs/vkd3d-shader/glsl.c \
	libs/vkd3d-shader/hlsl.c \
	libs/vkd3d-shader/hlsl.h \
	libs/vkd3d-shader/hlsl_codegen.c \
	libs/vkd3d-shader/hlsl_constant_ops.c \
	libs/vkd3d-shader/ir.c \
	libs/vkd3d-shader/preproc.h \
	libs/vkd3d-shader/spirv.c \
	libs/vkd3d-shader/tpf.c \
	libs/vkd3d-shader/vkd3d_shader.map \
	libs/vkd3d-shader/vkd3d_shader_main.c \
	libs/vkd3d-shader/vkd3d_shader_private.h
libvkd3d_shader_la_CFLAGS = $(AM_CFLAGS) -DLIBVKD3D_SHADER_SOURCE -I$(srcdir)/libs/vkd3d-shader @SPIRV_TOOLS_CFLAGS@
libvkd3d_shader_la_LDFLAGS = $(AM_LDFLAGS) -version-info 10:0:9
libvkd3d_shader_la_LIBADD = libvkd3d-common.la @SPIRV_TOOLS_LIBS@ -lm
if HAVE_LD_VERSION_SCRIPT
libvkd3d_shader_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libs/vkd3d-shader/vkd3d_shader.map
EXTRA_libvkd3d_shader_la_DEPENDENCIES = $(srcdir)/libs/vkd3d-shader/vkd3d_shader.map
endif

EXTRA_DIST = \
	libs/vkd3d-shader/hlsl.l \
	libs/vkd3d-shader/hlsl.y \
	libs/vkd3d-shader/preproc.l \
	libs/vkd3d-shader/preproc.y

libvkd3d_la_SOURCES = \
	include/private/vkd3d_blob.h \
	include/private/vkd3d_common.h \
	include/private/vkd3d_memory.h \
	include/private/vkd3d_shader_utils.h \
	include/private/vkd3d_test.h \
	include/private/vkd3d_utf8.h \
	include/vkd3d_d3d12.idl \
	include/vkd3d_d3dcommon.idl \
	include/vkd3d_unknown.idl \
	libs/vkd3d/cache.c \
	libs/vkd3d/command.c \
	libs/vkd3d/device.c \
	libs/vkd3d/resource.c \
	libs/vkd3d/state.c \
	libs/vkd3d/utils.c \
	libs/vkd3d/vkd3d.map \
	libs/vkd3d/vkd3d_main.c \
	libs/vkd3d/vkd3d_private.h \
	libs/vkd3d/vkd3d_shaders.h \
	libs/vkd3d/vulkan_procs.h
libvkd3d_la_CFLAGS = $(AM_CFLAGS) -DLIBVKD3D_SOURCE
libvkd3d_la_LDFLAGS = $(AM_LDFLAGS) -version-info 12:0:11
libvkd3d_la_LIBADD = libvkd3d-common.la libvkd3d-shader.la @DL_LIBS@ @PTHREAD_LIBS@
if HAVE_LD_VERSION_SCRIPT
libvkd3d_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libs/vkd3d/vkd3d.map
EXTRA_libvkd3d_la_DEPENDENCIES = $(srcdir)/libs/vkd3d/vkd3d.map
endif

libvkd3d_utils_la_SOURCES = \
	libs/vkd3d-utils/reflection.c \
	libs/vkd3d-utils/vkd3d_utils.map \
	libs/vkd3d-utils/vkd3d_utils_main.c \
	libs/vkd3d-utils/vkd3d_utils_private.h
libvkd3d_utils_la_CFLAGS = $(AM_CFLAGS) -DLIBVKD3D_UTILS_SOURCE
libvkd3d_utils_la_LDFLAGS = $(AM_LDFLAGS) -version-info 6:0:5
libvkd3d_utils_la_LIBADD = libvkd3d-common.la libvkd3d-shader.la libvkd3d.la @PTHREAD_LIBS@
if HAVE_LD_VERSION_SCRIPT
libvkd3d_utils_la_LDFLAGS += -Wl,--version-script=$(srcdir)/libs/vkd3d-utils/vkd3d_utils.map
EXTRA_libvkd3d_utils_la_DEPENDENCIES = $(srcdir)/libs/vkd3d-utils/vkd3d_utils.map
endif

EXTRA_DIST += ANNOUNCE LICENSE

pkgconfigdir = $(libdir)/pkgconfig
pkginclude_HEADERS = $(vkd3d_public_headers)
nodist_pkgconfig_DATA = libvkd3d.pc libvkd3d-shader.pc libvkd3d-utils.pc
CLEANFILES += libvkd3d.pc libvkd3d-shader.pc libvkd3d-utils.pc
EXTRA_DIST += \
	libs/vkd3d/libvkd3d.pc.in \
	libs/vkd3d-shader/libvkd3d-shader.pc.in \
	libs/vkd3d-utils/libvkd3d-utils.pc.in

bin_PROGRAMS = vkd3d-compiler vkd3d-dxbc

vkd3d_compiler_SOURCES = programs/vkd3d-compiler/main.c
vkd3d_compiler_CFLAGS = $(AM_CFLAGS) @NCURSES_CFLAGS@
vkd3d_compiler_LDADD = libvkd3d-shader.la @NCURSES_LIBS@

vkd3d_dxbc_SOURCES = programs/vkd3d-dxbc/main.c
vkd3d_dxbc_CFLAGS = $(AM_CFLAGS) @NCURSES_CFLAGS@
vkd3d_dxbc_LDADD = libvkd3d-shader.la @NCURSES_LIBS@

LDADD = libvkd3d.la libvkd3d-shader.la libvkd3d-utils.la
AM_DEFAULT_SOURCE_EXT = .c

TEST_EXTENSIONS = .shader_test

if BUILD_TESTS
check_PROGRAMS = $(vkd3d_tests) $(vkd3d_cross_tests) tests/shader_runner
dist_check_SCRIPTS = tests/test-driver.sh
TESTS = $(vkd3d_tests) $(vkd3d_cross_tests) $(vkd3d_shader_tests)
tests_d3d12_LDADD = $(LDADD) @PTHREAD_LIBS@ @DL_LIBS@
tests_d3d12_invalid_usage_LDADD = $(LDADD) @DL_LIBS@
tests_hlsl_d3d12_LDADD = $(LDADD) @DL_LIBS@
tests_shader_runner_CFLAGS = $(AM_CFLAGS) -I$(builddir)/tests @OPENGL_CFLAGS@
tests_shader_runner_LDADD = $(LDADD) @OPENGL_LIBS@ @DL_LIBS@
tests_shader_runner_SOURCES = \
	tests/shader_runner.c \
	tests/shader_runner_d3d9.c \
	tests/shader_runner_d3d11.c \
	tests/shader_runner_d3d12.c \
	tests/shader_runner_gl.c \
	tests/shader_runner_vulkan.c
tests_vkd3d_api_LDADD = libvkd3d.la @DL_LIBS@
tests_vkd3d_shader_api_LDADD = libvkd3d-shader.la
SHADER_TEST_LOG_COMPILER = tests/shader_runner
SHADER_TEST_LOG_DRIVER = $(srcdir)/tests/test-driver.sh
endif

EXTRA_DIST += $(vkd3d_shader_tests)

if BUILD_DEMOS
DEMOS_LDADD = $(LDADD) libvkd3d-shader.la @DL_LIBS@ @DEMO_LIBS@
DEMOS_CFLAGS = $(AM_CFLAGS) @DEMO_CFLAGS@
bin_PROGRAMS += $(vkd3d_demos)

demos_vkd3d_gears_SOURCES = demos/gears.c demos/gears_hlsl.h
demos_vkd3d_gears_CFLAGS = $(DEMOS_CFLAGS)
demos_vkd3d_gears_LDADD = $(DEMOS_LDADD) -lm

demos_vkd3d_triangle_SOURCES = demos/triangle.c demos/triangle_hlsl.h
demos_vkd3d_triangle_CFLAGS = $(DEMOS_CFLAGS)
demos_vkd3d_triangle_LDADD = $(DEMOS_LDADD)
endif

noinst_HEADERS = $(vkd3d_test_headers) $(vkd3d_demos_headers)

VKD3D_V_WIDL = $(vkd3d_v_widl_@AM_V@)
vkd3d_v_widl_ = $(vkd3d_v_widl_@AM_DEFAULT_V@)
vkd3d_v_widl_0 = @echo "  WIDL    " $@;
vkd3d_v_widl_1 =

if HAVE_WIDL
CLEANFILES += $(widl_headers)
endif
EXTRA_DIST += $(widl_headers) $(widl_headers:.h=.idl)
$(widl_headers): %.h: %.idl
if HAVE_WIDL
	$(VKD3D_V_WIDL)$(WIDL) -I$(srcdir)/include -h -o $@ $<
else
	@echo "widl is required to generate $@"
endif

libvkd3d-utils.pc: $(srcdir)/libs/vkd3d-utils/libvkd3d-utils.pc.in Makefile
	$(AM_V_GEN)$(SED) -e 's![@]prefix[@]!$(prefix)!g' \
		-e 's![@]exec_prefix[@]!$(exec_prefix)!g' \
		-e 's![@]includedir[@]!$(includedir)!g' \
		-e 's![@]libdir[@]!$(libdir)!g' \
		-e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
		$< > $@

libvkd3d-shader.pc: $(srcdir)/libs/vkd3d-shader/libvkd3d-shader.pc.in Makefile
	$(AM_V_GEN)$(SED) -e 's![@]prefix[@]!$(prefix)!g' \
		-e 's![@]exec_prefix[@]!$(exec_prefix)!g' \
		-e 's![@]includedir[@]!$(includedir)!g' \
		-e 's![@]libdir[@]!$(libdir)!g' \
		-e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
		$< > $@

libvkd3d.pc: $(srcdir)/libs/vkd3d/libvkd3d.pc.in Makefile
	$(AM_V_GEN)$(SED) -e 's![@]prefix[@]!$(prefix)!g' \
		-e 's![@]exec_prefix[@]!$(exec_prefix)!g' \
		-e 's![@]includedir[@]!$(includedir)!g' \
		-e 's![@]libdir[@]!$(libdir)!g' \
		-e 's![@]PACKAGE_VERSION[@]!$(PACKAGE_VERSION)!g' \
		$< > $@

include/private/vkd3d_version.h: dummy-vkd3d-version
	@$(MKDIR_P) include/private
	version=`(GIT_DIR=$(top_srcdir)/.git git rev-parse --short HEAD 2>/dev/null || echo '') \
		| $(SED) -e 's!\(..*\)! (git \1)!' \
		| $(SED) -n -e '$$s!\(.*\)!#define VKD3D_VCS_ID "\1"!p'` \
		&& (echo $$version | cmp -s - $@) \
		|| echo $$version >$@ || ($(RM) $@ && exit 1)
.SILENT: include/private/vkd3d_version.h
CLEANFILES += include/private/vkd3d_version.h

.PHONY: dummy-vkd3d-version
dummy-vkd3d-version:

## Cross-compile tests
cross_implibs = crosslibs/d3d12
CROSS_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/include/private -I$(builddir)/include -I$(builddir)/tests
CROSS_CFLAGS = -g -O2 -Wall -municode ${CROSS_CPPFLAGS} \
	-D_WIN32_WINNT=0x0600 -D__USE_MINGW_ANSI_STDIO=0 -DVKD3D_CROSSTEST=1
EXTRA_DIST += $(cross_implibs:=.cross32.def) $(cross_implibs:=.cross64.def)
EXTRA_DIST += tests/driver.c tests/shader_runner_d3d11.c tests/shader_runner_d3d9.c

shader_runner_cross_sources = \
	$(srcdir)/tests/shader_runner.c \
	$(srcdir)/tests/shader_runner_d3d9.c \
	$(srcdir)/tests/shader_runner_d3d11.c \
	$(srcdir)/tests/shader_runner_d3d12.c

driver_cross_sources = \
	$(srcdir)/tests/driver.c

if HAVE_CROSSTARGET32
CROSS32_CC = @CROSSCC32@
CROSS32_DLLTOOL = @CROSSTARGET32@-dlltool
CROSS32_IMPLIBS = $(cross_implibs:=.cross32.a)
CROSS32_EXEFILES =
if BUILD_TESTS
CROSS32_EXEFILES += $(vkd3d_cross_tests:=.cross32.exe)
endif
if BUILD_DEMOS
CROSS32_EXEFILES += $(vkd3d_demos:demos/vkd3d-%=demos/%.cross32.exe)
endif

CROSS32_FILES = $(CROSS32_EXEFILES)
if BUILD_TESTS
CROSS32_FILES += tests/shader_runner.cross32.exe
CROSS32_FILES += tests/driver.cross32.exe
endif

CLEANFILES += $(CROSS32_IMPLIBS) $(CROSS32_FILES)
crosstest32: $(CROSS32_FILES)

-include tests/$(DEPDIR)/*.cross32.Po
-include demos/$(DEPDIR)/*.cross32.Po

$(CROSS32_IMPLIBS): %.cross32.a: %.cross32.def
	@${MKDIR_P} crosslibs
	$(AM_V_GEN)$(CROSS32_DLLTOOL) -k -m i386 --as-flags=-32 -d $< -l $@

$(CROSS32_EXEFILES): %.cross32.exe: %.c $(CROSS32_IMPLIBS) $(widl_headers)
	$(AM_V_CCLD)depbase=`echo $@ | $(SED) 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS32_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $< $(CROSS32_IMPLIBS) -ldxgi -lgdi32 -ld3dcompiler_47 && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

tests/shader_runner.cross32.exe: $(shader_runner_cross_sources) $(CROSS32_IMPLIBS) $(widl_headers)
	$(AM_V_CCLD)depbase=`echo $@ | sed 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS32_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $(shader_runner_cross_sources) $(CROSS32_IMPLIBS) -ldxgi -lgdi32 -ld3dcompiler_47 && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

tests/driver.cross32.exe: $(driver_cross_sources)
	$(AM_V_CCLD)depbase=`echo $@ | sed 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS32_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $(driver_cross_sources) && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

else
crosstest32:
endif

if HAVE_CROSSTARGET64
CROSS64_CC = @CROSSCC64@
CROSS64_DLLTOOL = @CROSSTARGET64@-dlltool
CROSS64_IMPLIBS = $(cross_implibs:=.cross64.a)
CROSS64_EXEFILES =
if BUILD_TESTS
CROSS64_EXEFILES += $(vkd3d_cross_tests:=.cross64.exe)
endif
if BUILD_DEMOS
CROSS64_EXEFILES += $(vkd3d_demos:demos/vkd3d-%=demos/%.cross64.exe)
endif

CROSS64_FILES = $(CROSS64_EXEFILES)
if BUILD_TESTS
CROSS64_FILES += tests/shader_runner.cross64.exe
CROSS64_FILES += tests/driver.cross64.exe
endif

CLEANFILES += $(CROSS64_IMPLIBS) $(CROSS64_FILES)
crosstest64: $(CROSS64_FILES)

-include tests/$(DEPDIR)/*.cross64.Po
-include demos/$(DEPDIR)/*.cross64.Po

$(CROSS64_IMPLIBS): %.cross64.a: %.cross64.def
	@${MKDIR_P} crosslibs
	$(AM_V_GEN)$(CROSS64_DLLTOOL) -k -m i386:x86-64 --as-flags=-64 -d $< -l $@

$(CROSS64_EXEFILES): %.cross64.exe: %.c $(CROSS64_IMPLIBS) $(widl_headers)
	$(AM_V_CCLD)depbase=`echo $@ | sed 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS64_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $< $(CROSS64_IMPLIBS) -ldxgi -lgdi32 -ld3dcompiler_47 && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

tests/shader_runner.cross64.exe: $(shader_runner_cross_sources) $(CROSS64_IMPLIBS) $(widl_headers)
	$(AM_V_CCLD)depbase=`echo $@ | sed 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS64_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $(shader_runner_cross_sources) $(CROSS64_IMPLIBS) -ldxgi -lgdi32 -ld3dcompiler_47 && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

tests/driver.cross64.exe: $(driver_cross_sources)
	$(AM_V_CCLD)depbase=`echo $@ | sed 's![^/]*$$!$(DEPDIR)/&!;s!\.exe$$!!'`; \
	$(CROSS64_CC) $(CROSS_CFLAGS) -MT $@ -MD -MP -MF $$depbase.Tpo -o $@ $(driver_cross_sources) && \
	$(am__mv) $$depbase.Tpo $$depbase.Po

else
crosstest64:
endif

tests/crosstests.txt: FORCE
	$(AM_V_GEN) for i in $(vkd3d_cross_tests) ; do echo $$i ; done > $@

tests/shader_tests.txt: FORCE
	$(AM_V_GEN) for i in $(vkd3d_shader_tests) ; do echo $$i ; done > $@

crosstest-lists: tests/crosstests.txt tests/shader_tests.txt
CLEANFILES += tests/crosstests.txt tests/shader_tests.txt

FORCE:

.PHONY: crosstest crosstest32 crosstest64 crosstest-lists FORCE
crosstest: crosstest32 crosstest64 crosstest-lists

if BUILD_DOC
@DX_RULES@
all: doxygen-doc
CLEANFILES += $(DX_CLEANFILES)
endif
