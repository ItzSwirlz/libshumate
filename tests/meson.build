test_env = [
  'GIO_USE_VOLUME_MONITOR=unix',
  'GSETTINGS_BACKEND=memory',
  'G_ENABLE_DIAGNOSTIC=0',
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
]

coordinate = executable(
  'coordinate',
  'coordinate.c',
  dependencies: libshumate_dep,
)

test(
  'coordinate',
  coordinate,
  env: test_env
)

