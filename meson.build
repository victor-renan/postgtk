project('postgtk', 'c')

gnome = import('gnome')

deps = [dependency('gtk4'), dependency('libcurl'), dependency('jansson')]

resources = gnome.compile_resources('postgtk',
    'layout/postgtk.gresource.xml',
    source_dir: 'layout',
)

executable(
    meson.project_name(),
    'src/app.c', 'src/request.c', 'src/window.c', 'main.c', resources,
    dependencies: deps,
)