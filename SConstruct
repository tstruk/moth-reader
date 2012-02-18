# A Linux build Script.

# Setup Build Environment
env = Environment(
   CCFLAGS='-std=gnu++0x -ggdb -g -O0',
   LDFLAGS='-g',
   CPPPATH = ['/usr/include/GL']
)

# External Libs
env.ParseConfig('pkg-config --cflags sdl || true')
env.ParseConfig('pkg-config --cflags gl || true')
env.ParseConfig('pkg-config --cflags glew || true')
env.ParseConfig('pkg-config --cflags gtkmm-2.4 || true')
env.ParseConfig('pkg-config --cflags poppler-glib || true')

config = Configure(env);

# Check if OpenGL is there
if not config.CheckLibWithHeader( 'GL', 'gl.h', 'C' ):
	print "OpenGL Must be installed"
	Exit(1)

# Check if OpenGL Extension Wrangler is there
if not config.CheckLibWithHeader( 'GLEW', 'glew.h', 'C' ):
	print "OpenGL Extension Wrangler Must be installed (libglew-dev)"
	Exit(1)

# Check if SDL is there
if not config.CheckLibWithHeader( 'SDL', 'SDL.h', 'C' ):
	print "SDL Must be installed!"
	Exit(1)

# Check if gtk-dev is there
if not config.CheckLibWithHeader( 'gtkmm-2.4', 'gtkmm.h', 'C++' ):
	print "libgtkmm-2.4-dev or newer must be installed!"
	Exit(1)

# Check if poppler-glib is there
if not config.CheckLibWithHeader( 'poppler-glib', 'poppler.h', 'C' ):
	print "libpoppler-glib-dev Must be installed!"
	Exit(1)

# Validate the configuration and assign it to our env
env = config.Finish();

# Build main program
env.Program(
   target = 'moth',
   LIBS=['SDL', 'GL', 'GLU', 'GLEW', 'gtkmm-2.4', 'poppler-glib'],
   source = [ 'moth.cpp',
              'moth_gui.cpp',
              'moth_gui_file_choose.cpp',
              'moth_book.cpp',
              'moth_reader.cpp',
              'moth_reader_pdf.cpp' ] )
