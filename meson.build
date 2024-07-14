project('postgtk', 'c')

gnome = import('gnome')

deps = [dependency('gtk4'), dependency('libcurl')]

resources = gnome.compile_resources('postgtk',
    'data/postgtk.gresource.xml',
    source_dir: 'data',
)

executable(
    meson.project_name(),
    'src/app.c', 'src/services/request.c', 'src/window.c', 'main.c', resources,
    dependencies: deps,
)