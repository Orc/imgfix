#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
imgfix(FILE *input, FILE *output)
{
    register c;
    char last;
    int bp = 0;
    static char tag[] = "<img";


    while ( (c = fgetc(input)) != EOF ) {
	if ( bp > 0 ) {
	    if ( tag[bp] == c ) {
		if ( ++bp == 4 ) {
		    /* found an image tag; slew forward until we find a '>' */
		    fputs("<img", output);
		    last = 0;

		    while ( (c = fgetc(input)) != EOF && c != '>' ) {
			last = c;
			fputc(c, output);
		    }
		    if ( c == '>' ) {
			if ( last != '/' )
			    fputc('/', output);
			fputc(c, output);
		    }
		    bp = 0;
		}
	    }
	    else {
		fwrite(tag, bp, 1, output);
		fputc(c, output);
		bp = 0;
	    }
	}
	else if ( c == '<' )
	    bp = 1;
	else
	    fputc(c, output);
    }
    if ( bp > 0 )
	fwrite(tag, bp, 1, output);

    return ferror(output);
}


main(argc,argv)
char **argv;
{
    int i;
    int dryrun = 0;
    int ofd;
    char *outputfile = malloc(1);

    if ( outputfile == 0 ) {
	fprintf(stderr, "memory squeeze! cannot make initial output file\n");
	exit(1);
    }
    
    if ( argc > 1 && strcmp(argv[1], "-n") == 0 ) {
	dryrun = 1;
	++argv, --argc;
    }

    if ( argc < 2 )
	return imgfix(stdin, stdout);


    for ( i=1; i < argc; i++ ) {
	FILE *input;
	FILE *output;

	if ( (input = fopen(argv[i], "r")) == NULL ) {
	    fprintf(stderr, "cannot open <%s>\n", argv[i]);
	    continue;
	}

	if ( dryrun ) {
	    imgfix(input, stdout);
	}
	else {
	    if ( (outputfile = realloc(outputfile, strlen(argv[i])+7)) == 0 ) {
		fprintf(stderr, "out of memory allocating work file name\n");
		exit(1);
	    }
	    strcpy(outputfile, argv[i]);
	    strcat(outputfile, "XXXXXX");

	    if ( (ofd = mkstemp(outputfile)) == EOF ) {
		fprintf(stderr, "cannot create work file <%s>\n", outputfile);
		exit(1);
	    }
	    output = fdopen(ofd, "w");

	    if ( output == NULL ) {
		fprintf(stderr, "cannot open work file <%s>\n", outputfile);
		exit(1);
	    }

	    if ( imgfix(input, output) == 0 ) {
		fclose(output);
		close(ofd);
		if ( rename(outputfile, argv[i]) != 0 ) {
		    perror(argv[i]);
		    unlink(outputfile);
		    exit(1);
		}
	    }
	}
    }
}
