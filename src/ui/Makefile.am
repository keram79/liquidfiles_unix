# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -Wall -I .. -std=c++11

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
noinst_LIBRARIES = libui.a

libui_a_SOURCES = credentials.cpp \
				  common_arguments.cpp \
				  attach_command.cpp \
				  attach_chunk_command.cpp \
				  delete_attachments_command.cpp \
				  delete_filelink_command.cpp \
				  download_command.cpp \
				  filedrop_command.cpp \
				  filelink_command.cpp \
				  filelinks_command.cpp \
				  file_request_command.cpp \
				  get_api_key_command.cpp \
				  help_command.cpp \
				  messages_command.cpp \
				  send_command.cpp
