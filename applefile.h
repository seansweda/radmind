/*
 * applesingle format:
 *  header:
 *      -magic number (4 bytes)
 *      -version number (4 bytes)
 *      -filler (16 bytes)
 *      -number of entries (2 bytes)
 *      -x number of entries, with this format:
 *          id (4 bytes)
 *          offset (4 bytes)
 *          length (4 bytes)
 *  data:
 *      -finder info
 *      -rsrc fork
 *      -data fork
 */

#define AS_HEADERLEN	26
#define AS_MAGIC	0x00051600
#define AS_VERSION	0x00020000
#define AS_NENTRIES	3

#define ASEID_DFORK	1
#define ASEID_RFORK	2
#define ASEID_FINFO	9

#define AS_FIE		0	/* for array of ae_entry structs */
#define AS_RFE		1
#define AS_DFE		2	

struct as_entry {
    unsigned long	ae_id;
    unsigned long	ae_offset;
    unsigned long	ae_length;
};

struct as_header {
    unsigned long	ah_magic;
    unsigned long	ah_version;
    unsigned char	ah_filler[ 16 ];
    unsigned short	ah_num_entries;
};

int retr_applefile( SNET *sn, char *pathdesc, char *path, 
	char *chksumval, char *temppath, int linenum );
int stor_applefile( int dfd, SNET *sn, char *filename );
