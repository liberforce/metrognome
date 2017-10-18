project('metrognome', 'c')

gtk = dependency ('gtk+-3.0', version:'>=3.20')
gst = dependency ('gstreamer-1.0')
sources = ['main.c', 'metronome.c']
executable('metrognome', sources, dependencies: [gtk, gst])
