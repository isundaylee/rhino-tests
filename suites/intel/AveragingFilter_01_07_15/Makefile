COMPILER = gcc
#COMPILER ?= llvm-pir

# CXX := icpc
SRCDIR := src
BUILDDIR := release
# CFLAGS := -restrict -xAVX -O2 -ipo
EXTRA_CFLAGS :=
LIBFLAGS := -lcilkrts
option := res/nahelam.bmp res/nahelam1.bmp
vecreport := 1
ifdef vecreport
	CFLAGS += -vec-report1
endif

ifdef perf_num
	EXTRA_CFLAGS += -D PERF_NUM
endif

TARGET := $(BUILDDIR)/AveragingFilter

# icpc: $(TARGET)

# gcc: CXX := g++
# gcc: CFLAGS := -O2 -mfpmath=sse -flto
# gcc: $(TARGET)

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))

include $(COMPILER).mk

$(BUILDDIR) :
	if [ ! -d $(BUILDDIR) ]; then mkdir -p $(BUILDDIR); fi

$(TARGET): $(OBJECTS)
	echo " Linking..."
	$(CXX) $^ $(LIBFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(option)

clean:
	@echo " Cleaning..."
	@rm -fr $(BUILDDIR) $(TARGET) 2>/dev/null || true
	@rm -f *.valsig

.PHONY: clean
