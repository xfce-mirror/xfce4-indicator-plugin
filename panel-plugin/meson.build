plugin_sources = [
  'indicator-box.c',
  'indicator-box.h',
  'indicator-button-box.c',
  'indicator-button-box.h',
  'indicator-button.c',
  'indicator-button.h',
  'indicator.c',
  'indicator-config.c',
  'indicator-config.h',
  'indicator-dialog.c',
  'indicator-dialog.h',
  'indicator.h',
  xfce_revision_h,
]

plugin_sources += gnome.compile_resources(
  'indicator-dialog-resources',
  'indicator-dialog.gresource.xml',
)

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_library(
  'indicator-plugin',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-indicator-plugin'),
    '-DINDICATOR_DIR="@0@"'.format(indicator_dir),
    '-DINDICATOR_ICONS_DIR="@0@"'.format(indicator_iconsdir),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    ayatana_indicator,
    glib,
    gtk,
    libayatana_ido,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

indicator_desktop_in = configure_file(
  input: 'indicator.desktop.in.in',
  output: 'indicator.desktop.in',
  configuration: {
    'LIBXFCE4PANEL_VERSION_API': libxfce4panel_api,
  },
)

i18n.merge_file(
  input: indicator_desktop_in,
  output: 'indicator.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)
