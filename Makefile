# Project Name
TARGET = PassthruExample

# Sources
CPP_SOURCES = PassthruExample.cpp

# Library Locations
LIBDAISY_DIR = 3rd_party/libDaisy/
# DAISYSP_DIR = 3rd_party/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
