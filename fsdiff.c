/*
 * Copyright (c) 2003 Regents of The University of Michigan.
 * All Rights Reserved.  See COPYRIGHT.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <openssl/evp.h>

#include "applefile.h"
#include "transcript.h"
#include "llist.h"

void            (*logger)( char * ) = NULL;

extern char	*version, *checksumlist;

void	progress_update( char * );
void	fs_walk( struct llist *, int );
int	verbose = 0, ihookoutput = 0;
int	dodots = 0;
int	rootitems = 0;
float	pctdone = 0.0;
const EVP_MD    *md;

    void
progress_update( char *location )
{
    if ( pctdone > 100 ) {
	pctdone = 100.0;
    }

    if ( ihookoutput ) {
	printf( "%%%d\n", ( int )pctdone );
    } else {
	printf( "\rScanning %-50s %3d%% done", location, ( int )pctdone );
    }

    fflush( stdout );
}

    void
fs_walk( struct llist *path, int atroot  ) 
{
    DIR			*dir;
    struct dirent	*de;
    struct llist	*head = NULL;
    struct llist	*new;
    struct llist	*cur;
    int			len;
    float		increment = 0.0;
    char		temp[ MAXPATHLEN ];

    /* call the transcript code */
    if (( transcript( &path->ll_pinfo ) == 0 ) || ( skip )) {
	return;				
    }

    /* open directory */
    if (( dir = opendir( path->ll_pinfo.pi_name )) == NULL ) {
	if ( verbose ) putc( '\n', stdout );
	perror( path->ll_pinfo.pi_name );
	exit( 2 );	
    }

    /* read contents of directory */
    while (( de = readdir( dir )) != NULL ) {

	/* don't include . and .. */
	if (( strcmp( de->d_name, "." ) == 0 ) || 
		( strcmp( de->d_name, ".." ) == 0 )) {
	    continue;
	}

	if ( verbose && atroot ) {
	    rootitems++;
	}
	len = strlen( path->ll_pinfo.pi_name );

	/* absolute pathname. add 2 for / and NULL termination.  */
	if (( len + strlen( de->d_name ) + 2 ) > MAXPATHLEN ) {
	    if ( verbose ) putc( '\n', stdout );
	    fprintf( stderr, "Absolute pathname too long\n" );
	    exit( 2 );
	}

	if ( path->ll_pinfo.pi_name[ len - 1 ] == '/' ) {
	    if ( snprintf( temp, MAXPATHLEN, "%s%s", path->ll_pinfo.pi_name,
		    de->d_name ) > MAXPATHLEN ) {
		if ( verbose ) putc( '\n', stdout );
                fprintf( stderr, "%s%s: path too long\n",
                    path->ll_pinfo.pi_name, de->d_name );
		exit( 2 );
	    }
	} else {
            if ( snprintf( temp, MAXPATHLEN, "%s/%s", path->ll_pinfo.pi_name,
                    de->d_name ) > MAXPATHLEN ) {
		if ( verbose ) putc( '\n', stdout );
                fprintf( stderr, "%s/%s: path too long\n",
                    path->ll_pinfo.pi_name, de->d_name );
                exit( 2 );
            }
	}

	/* allocate new node for newly created relative pathname */
	new = ll_allocate( temp );

	/* insert new file into the list */
	ll_insert( &head, new ); 

    }

    if ( verbose && atroot ) {
	increment = ( float )( 100.0 / rootitems );
    }

    if ( closedir( dir ) != 0 ) {
	perror( "closedir" );
	exit( 2 );
    }

    /* call fswalk on each element in the sorted list */
    for ( cur = head; cur != NULL; cur = cur->ll_next ) {
	if ( verbose && atroot ) {
	    progress_update( cur->ll_pinfo.pi_name );
	}

	fs_walk ( cur, 0 );
	
	if ( verbose && atroot ) {
	    pctdone += increment;
	}
    }

    ll_free( head );
    if ( verbose && atroot ) {
	pctdone = 100.0;
	progress_update( "complete." );
	if ( ! ihookoutput ) putc( '\n', stdout );
    }

    return;
}

    int
main( int argc, char **argv ) 
{
    struct llist	*root;
    extern char 	*optarg;
    extern int		optind;
    char		*kfile = _RADMIND_COMMANDFILE;
    int			gotkfile = 0;
    int 		c, len, edit_path_change = 0;
    int 		errflag = 0, use_outfile = 0;

    edit_path = CREATABLE;
    cksum = 0;
    outtran = stdout;

    while (( c = getopt( argc, argv, "Ac:CIo:K:1Vv" )) != EOF ) {
	switch( c ) {
	case 'c':
            OpenSSL_add_all_digests();
            md = EVP_get_digestbyname( optarg );
            if ( !md ) {
                fprintf( stderr, "%s: unsupported checksum\n", optarg );
                exit( 2 );
            }
            cksum = 1;
            break;
	case 'o':
	    if (( outtran = fopen( optarg, "w" )) == NULL ) {
		perror( optarg );
		exit( 2 );
	    }
	    use_outfile = 1;
	    break;

	case 'K':
	    kfile = optarg;
	    gotkfile = 1;
	    break;

	case 'I':
	    ihookoutput = 1;
	    verbose = 1;
	    break;

	case '1':
	    skip = 1;
	case 'C':
	    edit_path_change++;
	    edit_path = CREATABLE;
	    break;	

	case 'A':
	    edit_path_change++;
	    edit_path = APPLICABLE;
	    break;

	case 'V':		
	    printf( "%s\n", version );
	    printf( "%s\n", checksumlist );
	    exit( 0 );

	case 'v':
	    verbose = 1;
	    break;

	case '?':
	    errflag++;
	    break;

	default: 
	    break;
	}
    }

    if ( verbose && ! use_outfile ) {
	errflag++;
    }

    if (( edit_path == APPLICABLE ) && ( skip )) {
	errflag++;
    }
    if ( edit_path_change > 1 ) {
	errflag++;
    }

    /* Check that kfile isn't an abvious directory */
    len = strlen( kfile );
    if ( kfile[ len - 1 ] == '/' ) {
        errflag++;
    }

    if ( errflag || ( argc - optind != 1 )) {
	fprintf( stderr, "usage: %s [ -C | -A | -1 ] ", argv[ 0 ] );
	fprintf( stderr, "[ -Iv ] [ -K command ] " );
	fprintf( stderr, "[ -c cksumtype ] [ -o file ] path\n" );
	exit ( 2 );
    }

    /* initialize the transcripts */
    transcript_init( kfile, gotkfile );
    root = ll_allocate( argv[ optind ] );

    fs_walk( root, 1 );

    /* free the transcripts */
    transcript_free( );
    hardlink_free( );
	    
    /* close the output file */     
    fclose( outtran );

    exit( 0 );	
}
