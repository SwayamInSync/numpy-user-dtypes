project('numpy_quaddtype', 'c', 'cpp', default_options : ['cpp_std=c++20', 'b_pie=true'])

py_mod = import('python')
py = py_mod.find_installation()
py_dep = py.dependency()

c = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

is_windows = build_machine.system() == 'windows'

incdir_numpy = ''
if is_windows
  add_project_arguments('-DWIN32', '-D_WINDOWS', language : ['c', 'cpp'])
  
  conda_env_dir = run_command('cmd', '/c', 'echo %CONDA_PREFIX%', check: true).stdout().strip()
  sleef_include_dir = conda_env_dir + '\\Library\\include'
  sleef_library_dir = conda_env_dir + '\\Library\\lib'
  incdir_numpy = run_command('cmd', '/c', 'echo %NUMPY_INCLUDE_DIR%', check: true).stdout().strip()
  
  add_project_arguments('-I' + sleef_include_dir, language: ['c', 'cpp'])
  add_project_link_arguments('-L' + sleef_library_dir, language: ['c', 'cpp'])
  
  sleef_lib = c.find_library('sleef', dirs: [sleef_library_dir])
  sleefquad_lib = c.find_library('sleefquad', dirs: [sleef_library_dir])
  
  sleef_dep = declare_dependency(include_directories: include_directories(sleef_include_dir),
                                 dependencies: [sleef_lib, sleefquad_lib])
else
  sleef_dep = [
    c.find_library('sleef', required : true),
    c.find_library('sleefquad', required : true)
  ]
endif

incdir_numpy = run_command(py,
  ['-c', 'import numpy; print(numpy.get_include())'],
  check : true
).stdout().strip()

includes = include_directories(
    [
        incdir_numpy,
        'numpy_quaddtype/src',
    ]
)

srcs = [
    'numpy_quaddtype/src/quad_common.h',
    'numpy_quaddtype/src/casts.h',
    'numpy_quaddtype/src/casts.cpp',
    'numpy_quaddtype/src/scalar.h',
    'numpy_quaddtype/src/scalar.c',
    'numpy_quaddtype/src/dtype.h',
    'numpy_quaddtype/src/dtype.c',
    'numpy_quaddtype/src/quaddtype_main.c',
    'numpy_quaddtype/src/scalar_ops.h',
    'numpy_quaddtype/src/scalar_ops.cpp',
    'numpy_quaddtype/src/ops.hpp',
    'numpy_quaddtype/src/umath.h',
    'numpy_quaddtype/src/umath.cpp',
    'numpy_quaddtype/src/dragon4.h',
    'numpy_quaddtype/src/dragon4.c'
]

py.install_sources(
    [
        'numpy_quaddtype/__init__.py',
    ],
    subdir: 'numpy_quaddtype',
    pure: false
)

py.extension_module('_quaddtype_main',
srcs,
link_args: is_windows ? ['/DEFAULTLIB:sleef', '/DEFAULTLIB:sleefquad'] : ['-lsleef', '-lsleefquad'],
link_language: 'cpp',
dependencies: [sleef_dep] + (is_windows ? [] : [sleefquad_dep]),
install: true,
subdir: 'numpy_quaddtype',
include_directories: includes
)