project('quaddtype', 'c', 'cpp', default_options : ['cpp_std=c++20', 'b_pie=true'])

py_mod = import('python')
py = py_mod.find_installation()
c = meson.get_compiler('c')
pkgconfig = import('pkgconfig')

is_windows = build_machine.system() == 'windows'

# Function to get environment variable
get_env = is_windows ? 
  (lambda v : run_command('cmd', '/c', 'echo %' + v + '%', check: false).stdout().strip()) :
  (lambda v : run_command('bash', '-c', 'echo $' + v, check: false).stdout().strip())

# Try to find SLEEF using pkg-config first
sleef_dep = dependency('sleef', required : false)
sleefquad_dep = dependency('sleefquad', required : false)

if not sleef_dep.found() or not sleefquad_dep.found()
  # If pkg-config fails, fall back to manual library detection
  conda_prefix = get_env('CONDA_PREFIX')
  sleef_path = get_env('SLEEF_PATH')
  if sleef_path == ''
    sleef_path = conda_prefix
  endif

  if sleef_path == ''
    error('SLEEF_PATH or CONDA_PREFIX environment variable is not set')
  endif

  sleef_include_dir = is_windows ? sleef_path / 'Library' / 'include' : sleef_path / 'include'
  sleef_library_dir = is_windows ? sleef_path / 'Library' / 'lib' : sleef_path / 'lib'

  add_project_arguments('-I' + sleef_include_dir, language: ['c', 'cpp'])
  add_project_link_arguments('-L' + sleef_library_dir, language: ['c', 'cpp'])

  sleef_lib = c.find_library('sleef', dirs: [sleef_library_dir])
  sleefquad_lib = c.find_library('sleefquad', dirs: [sleef_library_dir])

  if not sleef_lib.found() or not sleefquad_lib.found()
    error('SLEEF libraries not found. Please ensure SLEEF is properly installed.')
  endif

  sleef_dep = declare_dependency(
    include_directories: include_directories(sleef_include_dir),
    dependencies: [sleef_lib, sleefquad_lib]
  )
  sleefquad_dep = sleef_dep
endif

# Find NumPy include directory
incdir_numpy = get_env('NUMPY_INCLUDE_DIR')
if incdir_numpy == ''
  incdir_numpy = run_command(py,
    ['-c', 'import numpy; import os; print(os.path.relpath(numpy.get_include()))'],
    check : false
  ).stdout().strip()
endif

if incdir_numpy == ''
  error('NumPy include directory not found. Please ensure NumPy is installed.')
endif

includes = include_directories([incdir_numpy, 'quaddtype/src'])

srcs = [
    'quaddtype/src/casts.h',
    'quaddtype/src/casts.cpp',
    'quaddtype/src/scalar.h',
    'quaddtype/src/scalar.c',
    'quaddtype/src/dtype.h',
    'quaddtype/src/dtype.c',
    'quaddtype/src/quaddtype_main.c',
    'quaddtype/src/scalar_ops.h',
    'quaddtype/src/scalar_ops.cpp',
    'quaddtype/src/ops.hpp',
    'quaddtype/src/umath.h',
    'quaddtype/src/umath.cpp'
]

py.install_sources(['quaddtype/__init__.py'], subdir: 'quaddtype', pure: false)

py.extension_module('_quaddtype_main',
  srcs,
  c_args: is_windows ? ['/DWIN32', '/D_WINDOWS'] : ['-g', '-O0'],
  cpp_args: is_windows ? ['/DWIN32', '/D_WINDOWS', '/EHsc'] : ['-g', '-O0'],
  link_args: is_windows ? ['/DEFAULTLIB:sleef', '/DEFAULTLIB:sleefquad'] : ['-lsleef', '-lsleefquad'],
  dependencies: [sleef_dep, sleefquad_dep],
  link_language: 'cpp',
  install: true,
  subdir: 'quaddtype',
  include_directories: includes
)