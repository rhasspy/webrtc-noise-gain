project('webrtc-audio-processing', 'c', 'cpp',
  version : '1.3',
  meson_version : '>= 0.63',
  default_options : [ 'warning_level=1',
                      'buildtype=debugoptimized',
                      'c_std=c11',
                      'cpp_std=c++17',
                    ]
)

version_split = meson.project_version().split('.')
# This will be incremented each time a breaking API change occurs
major_version = version_split[0]
# This will be incremented when there are backwards-compatible changes
minor_version = version_split[1]

# We maintain per-package versions to not have to break API for one if only the
# other has breaking changes

apm_major_version = major_version
apm_minor_version = minor_version
apm_project_name = 'webrtc-audio-processing-' + apm_major_version

ac_major_version = major_version
ac_minor_version = minor_version
ac_project_name = 'webrtc-audio-coding-' + ac_major_version

include_subdir = apm_project_name

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

host_system = host_machine.system()

# Don't rely on the cross file setting the system properly when targeting ios
if host_system == 'darwin' and meson.is_cross_build()
  ios_test_code = '''#include <TargetConditionals.h>
  #if ! TARGET_OS_IPHONE
  #error "Not iOS/tvOS/watchOS/iPhoneSimulator"
  #endif'''
  if cc.compiles(ios_test_code, name : 'building for iOS')
    host_system = 'ios'
  endif
endif

platform_cflags = []
os_cflags = []
os_deps = []
have_posix = false
have_win = false

# Let's use pkg-config if available. This will also fallback to the subproject
# if pkg-config is not found, which is really the most reliable way of building
# abseil due to strict C++ standard match requirements.
absl_dep = [
  dependency('absl_base', default_options: ['cpp_std=c++17']),
  dependency('absl_flags'),
  dependency('absl_strings'),
  dependency('absl_synchronization'),
  dependency('absl_bad_optional_access'),
]

if absl_dep[0].type_name() == 'internal'
  absl_subproj = subproject('abseil-cpp')
  headers = [
    absl_subproj.get_variable('absl_base_headers'),
    absl_subproj.get_variable('absl_flags_headers'),
    absl_subproj.get_variable('absl_strings_headers'),
    absl_subproj.get_variable('absl_synchronization_headers'),
    absl_subproj.get_variable('absl_types_headers'),
  ]
  install_headers(headers, preserve_path: true)
  pc_requires = []
else
  pc_requires = [absl_dep[0]]
endif

if ['darwin', 'ios'].contains(host_system)
  os_cflags = ['-DWEBRTC_MAC']
  if host_system == 'ios'
    os_cflags += ['-DWEBRTC_IOS']
  endif
  have_posix = true
elif host_system == 'android'
  os_cflags += ['-DWEBRTC_ANDROID', '-DWEBRTC_LINUX']
  os_deps += [cc.find_library('log')]
  os_deps += [dependency('gnustl', required : get_option('gnustl'))]
  have_posix = true
elif host_system == 'linux'
  os_cflags += ['-DWEBRTC_LINUX', '-DWEBRTC_THREAD_RR']
  os_deps += [cc.find_library('rt', required : false)]
  os_deps += [dependency('threads')]
  have_posix = true
elif host_system == 'windows'
  platform_cflags += ['-DWEBRTC_WIN', '-D_WIN32', '-U__STRICT_ANSI__']
  # this one is for MinGW to get format specifiers from inttypes.h in C++
  platform_cflags += ['-D__STDC_FORMAT_MACROS=1']
  # Avoid min/max from windows.h which breaks std::min/max
  platform_cflags += ['-DNOMINMAX']
  # Ensure M_PI etc are defined
  platform_cflags += ['-D_USE_MATH_DEFINES']
  os_deps += [cc.find_library('winmm')]
  have_win = true
endif

if have_posix
  platform_cflags += ['-DWEBRTC_POSIX']
endif

arch_cflags = []
have_arm = false
have_armv7 = false
have_neon = false
have_mips = false
have_mips64 = false
have_x86 = false
have_avx2 = false
if host_machine.cpu_family() == 'arm'
  if cc.compiles('''#ifndef __ARM_ARCH_ISA_ARM
#error no arm arch
#endif''')
    have_arm = true
    arch_cflags += ['-DWEBRTC_ARCH_ARM']
  endif
  if cc.compiles('''#ifndef __ARM_ARCH_7A__
#error no armv7 arch
#endif''')
    have_armv7 = true
    arch_cflags += ['-DWEBRTC_ARCH_ARM_V7']
  endif
endif
if cc.compiles('''#ifndef __aarch64__
#error no aarch64 arch
#endif''')
  have_neon = true
  arch_cflags += ['-DWEBRTC_ARCH_ARM64', '-DWEBRTC_HAS_NEON']
endif
if ['mips', 'mips64'].contains(host_machine.cpu_family())
  have_mips = true
  arch_cflags += ['WEBRTC_ARCH_MIPS_FAMILY']
endif
if host_machine.cpu_family() == 'mips64'
  have_mips64 = true
endif
if ['x86', 'x86_64'].contains(host_machine.cpu_family())
  have_x86 = true
  # This is unconditionally enabled for now, actual usage is determined by
  # runtime CPU detection, so we're just assuming the compiler supports avx2
  have_avx2 = true
  arch_cflags += ['-DWEBRTC_ENABLE_AVX2']
endif

neon_opt = get_option('neon')
if neon_opt != 'no' and not have_neon
  if neon_opt != 'runtime'
    if cc.compiles('#include <arm_neon.h>', args : '-mfpu=neon')
      arch_cflags += ['-mfpu=neon', '-DWEBRTC_HAS_NEON']
      have_neon = true
    endif
  else
    arch_cflags += ['-DWEBRTC_DETECT_NEON', '-mfpu=neon']
    have_neon = true
  endif
endif

common_cflags = [
  '-DWEBRTC_LIBRARY_IMPL',
  '-DWEBRTC_ENABLE_SYMBOL_EXPORT',
  '-DNDEBUG'
  ] + platform_cflags + os_cflags + arch_cflags
common_cxxflags = common_cflags
common_deps = os_deps + [absl_dep]
webrtc_inc = include_directories('.')

subdir('webrtc')

pkgconfig = import('pkgconfig')

pkgconfig.generate(
  libwebrtc_audio_processing,
  description: 'WebRTC Audio Processing library',
  subdirs: include_subdir,
  requires: pc_requires,
  extra_cflags: [
    '-DWEBRTC_LIBRARY_IMPL',
  ] + platform_cflags,
)

audio_processing_dep = declare_dependency(
  link_with: libwebrtc_audio_processing,
  dependencies: [absl_dep],
  include_directories: [webrtc_inc]
)

meson.override_dependency(apm_project_name, audio_processing_dep)

pkgconfig.generate(
  libwebrtc_audio_coding,
  description: 'WebRTC Audio Coding library',
  subdirs: include_subdir,
  extra_cflags: [
    '-DWEBRTC_LIBRARY_IMPL',
  ] + platform_cflags,
)

audio_coding_dep = declare_dependency(
  link_with: libwebrtc_audio_coding,
  include_directories: [webrtc_inc]
)

meson.override_dependency(ac_project_name, audio_coding_dep)
