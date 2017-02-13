cc = gcc
CFLAG = -g -Wall -Wextra -Werror -Wno-unused
DISTDIR = lab1-junwhanyoo

default: simpsh

simpsh: simpsh.c
	$(CC) $(CFLAG) simpsh.c -o $@

clean:
	rm -fr simpsh
check:
	./test.sh

dist: $(DISTDIR).tar.gz

SOURCES =  simpsh.c Makefile README test.sh
$(DISTDIR).tar.gz : $(SOURCES)
	./test.sh
	tar -czf $@ --transform='s,^,$(DISTDIR)/,' $(SOURCES)
