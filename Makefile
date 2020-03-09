SOURCES =  spot.c cmd_parameters.c create_map.c forward_time.c sampling.c rewind_time.c mutation.c
OBJECTS = $(SOURCES:%.c=%.o)

TARGET = sps

all: $(TARGET)

%.o: %.c
	gcc -openmp -pedantic -w  -g $< -c -o $@

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -o $@ -lm -lgsl -lgslcblas

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f *.png
