SOURCES =  spot.c create_map.c forward_time.c rewind_time.c mutation.c
OBJECTS = $( SOURCES:%.c=%.o )
TARGET = sps

all: $(TARGET)

%.o: %.c
	gcc -pedantic -w -g $< -c -o $@

%(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -o $@ -lm -lgsl	
clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f *.txt
