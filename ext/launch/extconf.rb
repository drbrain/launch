require 'mkmf'

abort unless have_header 'launch.h'

create_makefile 'launch/launch'

