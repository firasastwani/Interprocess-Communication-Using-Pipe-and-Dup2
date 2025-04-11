compile:
	gcc -Wall -pedantic-errors lab12.c -o lab12.out

run:
	./lab12.out head -4 words.txt -pipe grep r

clean:
	rm *.out

