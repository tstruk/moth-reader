# Scons build Script.

# Setup Build Environment
env = Environment(
   CCFLAGS='-std=gnu++0x -O3 -Wall -Wextra'
)

SConscript(['moth_index_gui/SConstruct'])

# External Libs
env.ParseConfig('pkg-config --cflags --libs sdl || true')
env.ParseConfig('pkg-config --cflags --libs gl || true')
env.ParseConfig('pkg-config --cflags --libs glu || true')
env.ParseConfig('pkg-config --cflags --libs glew || true')
env.ParseConfig('pkg-config --cflags --libs ftgl || true')
env.ParseConfig('pkg-config --cflags --libs gdkmm-3.0 || true')
env.ParseConfig('pkg-config --cflags --libs gtkmm-3.0 || true')
env.ParseConfig('pkg-config --cflags --libs poppler-glib || true')

config = Configure(env);

# Check if OpenGL is there
if not config.CheckLibWithHeader('GL', 'GL/gl.h', 'C'):
	print("OpenGL Must be installed")
	Exit(1)

# Check if OpenGL Extension Wrangler is there
if not config.CheckLibWithHeader('GLEW', 'GL/glew.h', 'C'):
	print("OpenGL Extension Wrangler Must be installed (libglew-dev)")
	Exit(1)

# Check if ftgl is there
if not config.CheckLibWithHeader('ftgl', 'ftgl.h', 'C++'):
	print("FreeType OpenGL Must be installed (libftgl-dev)")
	Exit(1)

# Check if SDL is there
if not config.CheckLibWithHeader('SDL', 'SDL.h', 'C'):
	print("SDL Must be installed!")
	Exit(1)

# Check if gdk-dev is there
if not config.CheckLibWithHeader('gdkmm-3.0', 'gdkmm-3.0/gdkmm.h', 'C++'):
	print("gdkmm-3.0 Must be installed!")
	Exit(1)

# Check if gtk-dev is there
if not config.CheckLibWithHeader('gtkmm-3.0', 'gtkmm-3.0/gtkmm.h', 'C++'):
	print ("gtkmm-3.0 or newer must be installed!")
	Exit(1)

# Check if poppler-glib is there
if not config.CheckLibWithHeader('poppler-glib', 'poppler.h', 'C'):
	print("libpoppler-glib-dev Must be installed!")
	Exit(1)

# Validate the configuration and assign it to env
env = config.Finish();

# Build main program
moth = env.Program(
            target =  'moth',
            source = ['moth.cpp',
                      'moth_image.cpp',
                      'moth_gui.cpp',
                      'moth_gui_dialog.cpp',
                      'moth_index.cpp',
                      'moth_book.cpp',
                      'moth_fonts.cpp',
                      'moth_reader.cpp',
                      'moth_reader_pdf.cpp'])

env.Install('/usr/bin', moth)
env.Alias('install', '/usr/bin')
