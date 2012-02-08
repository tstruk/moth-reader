# A Linux build Script.

# Setup Build Environment
env = Environment(
   CCFLAGS='-ggdb -pg -g3',
   LDFLAGS='-pg',
   CPPPATH = ['/usr/include/GL']
)

# External Libs
env.ParseConfig('pkg-config --cflags sdl || true')
env.ParseConfig('pkg-config --cflags gtk+-2.0 || true')
env.ParseConfig('pkg-config --cflags poppler-glib || true')

config = Configure(env);

# Check if OpenGL is there
if not config.CheckLibWithHeader( 'GL', 'gl.h', 'C' ):
	print "OpenGL Must be installed!"
	Exit(1)

# Check if SDL is there
if not config.CheckLibWithHeader( 'SDL', 'SDL.h', 'C' ):
	print "SDL Must be installed!"
	Exit(1)

# Check if gtk-dev is there
if not config.CheckLibWithHeader( 'gtk-x11-2.0', 'gtk/gtk.h', 'C' ):
	print "libgtk2.0-dev Must be installed!"
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
   LIBS=['SDL', 'GL', 'gtk-x11-2.0', 'poppler-glib'],
   source = [ 'moth.cpp', 'moth_reader.cpp', 'gui.cpp', 'moth_book.cpp', 'moth_reader_pdf.cpp' ] )
