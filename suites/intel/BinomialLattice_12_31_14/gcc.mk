CC ?= gcc
CXX ?= g++

CFLAGS := -D__INTEL_COMPILER -O3 -std=c++11

CFLAGS += $(OPTFLAGS) -fcilkplus -fno-exceptions
CXXFLAGS += $(OPTFLAGS) -fcilkplus -fno-exceptions

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)
LDFLAGS += -lcilkrts

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	@$(CXX) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $< 
