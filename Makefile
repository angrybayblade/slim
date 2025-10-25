cc=gcc
flags=-Wall -Werror -lX11 -lXtst
src=src
test=test
bin=bin

all: clean setup $(bin)/serial

clean:
	rm -rf $(bin)/

setup:
	mkdir -p $(bin)


$(bin)/serial: $(src)/serial.c $(src)/keyboard.c
	gcc -o $@ $^ $(flags)

tests: clean setup $(bin)/echo $(bin)/press $(bin)/sequence $(bin)/combination

$(bin)/echo: $(test)/echo.c $(src)/keyboard.c
	gcc -o $@ $^ $(flags)

$(bin)/press: $(test)/press.c $(src)/keyboard.c
	gcc -o $@ $^ $(flags)

$(bin)/sequence: $(test)/sequence.c $(src)/keyboard.c
	gcc -o $@ $^ $(flags)

$(bin)/combination: $(test)/combination.c $(src)/keyboard.c
	gcc -o $@ $^ $(flags)
