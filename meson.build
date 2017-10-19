project(
    'metrognome',
    'c',
    version: '0.1.0',
    meson_version: '>=0.43.0',
    license: 'GPL-2.0+'
)

cc = meson.get_compiler('c')
mathlib = cc.find_library('m', required: false)

deps = [
    dependency('gtk+-3.0', version:'>=3.20'),
    dependency('gstreamer-1.0'),
    mathlib
]

sources = [
    'main.c',
    'metronome.c'
]

executable(
    'metrognome',
    sources,
    install: true,
    dependencies: deps
)

metrognome_prefix = get_option('prefix')
metrognome_datadir = join_paths(metrognome_prefix, get_option('datadir'))

conf = configuration_data()
conf.set('VERSION', meson.project_version())
conf.set('METROGNOME_DATADIR', metrognome_datadir)

configure_file(
    input: 'config.h.meson',
    output: 'config.h',
    configuration: conf
)

install_data(
    'metrognome.ui',
    install_dir: metrognome_datadir
)
