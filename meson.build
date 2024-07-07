project('postgtk', 'c')

gnome = import('gnome')

deps = [dependency('gtk4')]

resources = gnome.compile_resources('postgtk',
    'data/postgtk.gresource.xml',
    source_dir: 'data',
)

executable(
    meson.project_name(),
    'src/app.c', 'src/window.c', 'main.c', resources,
    dependencies: deps,
)