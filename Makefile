ifeq ($(CXX),g++)
    EXTRA_FLAGS = --no-gnu-unique
else
    EXTRA_FLAGS =
endif

CXXFLAGS ?= -O2
CXXFLAGS += -shared -fPIC -std=c++26

SRC = src/main.cpp src/edge_action.cpp

PKG_INCLUDES = $(shell pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon 2>/dev/null)

.PHONY: all clean

all:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(EXTRA_FLAGS) $(SRC) -o hyprtoggle.so $(PKG_INCLUDES)

clean:
	rm -f hyprtoggle.so
