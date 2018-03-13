imgfix: imgfix.c

clean:
	rm -f  imgfix imgfix.o

test:	tests/runtest.sh imgfix
	@tests/runtest.sh
