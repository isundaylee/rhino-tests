CC=/home/wmoses/git/Parallel-IR/build/bin/compile -slow    -fsanitize=safe-stack -D_FORTIFY_SOURCE=2 -fstack-protector
CXX=/home/wmoses/git/Parallel-IR/build/bin/compile++ -slow -fsanitize=safe-stack -D_FORTIFY_SOURCE=2 -fstack-protector


CFLAGS := -D__INTEL_COMPILER -O3 -ggdb # -march=native

CFLAGS += $(OPTFLAGS) -fcilkplus
CXXFLAGS += $(OPTFLAGS) -fcilkplus

LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)
LDFLAGS += -lcilkrts -ldl

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) -c $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $< 
