/*
 * =====================================================================================
 *
 *       Filename:  gpsTest.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/04/2014 10:57:51 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hong-Bin Yoon (HBY), yoon48@illinois.edu
 *   Organization:  Aerospace Robotics and Controls Lab (ARC)
 *
 * =====================================================================================
 */

/* Produces a stream of data from the com port specified for the GPS */
/* Will request the standard NMEA messages be sent as part of startup on the port */
/* get our local serial convenience functions */
#include "spanread.h"




std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

/* --------------------------------------------------------------------------
Calculate a CRC value to be used by CRC calculation functions.
-------------------------------------------------------------------------- */
unsigned long CRC32Value(int i)
{
    int j;
    unsigned long ulCRC;
    ulCRC = i;
    for ( j = 8 ; j > 0; j-- )
    {
        if ( ulCRC & 1 )
            ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
        else
            ulCRC >>= 1;
    }
    return ulCRC;
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32( unsigned long ulCount, /* Number of bytes in the data block */
                                   unsigned char *ucBuffer ) /* Data block */
{
    unsigned long ulTemp1;
    unsigned long ulTemp2;
    unsigned long ulCRC = 0;
    while ( ulCount-- != 0 )
    {
        ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
        ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
        ulCRC = ulTemp1 ^ ulTemp2;
    }
    return( ulCRC );
}


int set_interface_attribs(int fd, int speed, int parity)
{
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("-(!) error %d from tcgetattr\n", errno);
                exit(EXIT_FAILURE);
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_iflag &= ~(IGNBRK|BRKINT|ICRNL|INLCR|PARMRK|INPCK|ISTRIP|IXON);
        tty.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
                     ONOCR | OFILL | OLCUC | OPOST);                // no remapping, no delays
        tty.c_lflag &= ~(ECHO|ECHONL|IEXTEN|ISIG);;                // no signaling chars, no echo,
                                        // no canonical processing

        tty.c_cflag &= ~(CSIZE|PARENB);
        tty.c_cflag |= CS8;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf("-(!) error %d from tcsetattr", errno);
                exit(EXIT_FAILURE);
        }
        return 0;
}

void set_blocking(int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("-(!) error %d from tggetattr", errno);
                exit(EXIT_FAILURE);
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf("-(!) error %d setting term attributes", errno);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  extractMsg
 *  Description:  Points msg to the beginning of the message, points crc to the
 *  beginning of the crc, sets the * to '\0'.
 * =====================================================================================
 */
void extractMsg( char *buf, char **msg, char **crc )
{
    unsigned long len;
    *msg=buf+1;
    len=strlen(*msg);
    len-=11; // Exclude the checksum.
    *(*msg+len)='\0';
    *crc=*msg+len+1;
    return;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  isValid
 *  Description:  Returns true if calculated CRC matches given CRC.
 * =====================================================================================
 */
int isValid( char *buf )
{
    unsigned long int len, CRC, givenCRC;
    char *msg, *givenASCII;

    extractMsg( buf, &msg, &givenASCII ); 

    len=strlen(msg);
    CRC = CalculateBlockCRC32( len, (unsigned char *)msg );

    sscanf( givenASCII, "%x", &givenCRC);
    return (int)(CRC-givenCRC)==0 ; // Not sure why I need the typecast.
}

void help( char *arg0)
{
    printf("Usage: %s [LOGNAME] [OPTIONS]\n", arg0);
    printf("LOGNAME: usb1, usb2, or usb3\n");
    printf("OPTIONS\n");
    printf("%s gpsfile\tLog GPS data\n", ARGGPS);
    printf("%s attfile\tLog att data\n", ARGATT);
    printf("%s imufile\tLog acc and gyro data\n", ARGIMU);
    printf("%s pashrfile\tLog pashr data\n", ARGPASHR);
    return;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  readargs
 *  Description:  initializes params with arguments.
 * =====================================================================================
 */
    void
readargs ( int argc, char **argv, char **p )
{
    p[DEVICE]=argv[1];
    int i;
    for( i=2; i<argc; ++i )
    {
        if( !strcmp(argv[i],ARGGPS) ) p[TYPE_GPS]=argv[++i];
        else if( !strcmp(argv[i],ARGIMU) ) p[TYPE_IMU]=argv[++i];
        else if( !strcmp(argv[i],ARGATT) ) p[TYPE_ATT]=argv[++i];
        else if( !strcmp(argv[i],ARGPASHR) ) p[TYPE_PASHR]=argv[++i];
    }
    return ;
}		/* -----  end of function readargs  ----- */

    
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getType
 *  Description:  Returns the type of data.
 * =====================================================================================
 */
    int
getType ( char *buf )
{
    char *type;
    int rv;
    int i;
    for( i=0; *(buf+i)!=','; ++i );
    
    type	= (char *)calloc ( (size_t)(i+1), sizeof(char) );
    if ( type==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }
    strncpy( type, buf, i );
    type[i]='\0';

    if( !strcmp( type, GPSNAME ) ) rv=TYPE_GPS;
    else if( !strcmp( type, ATTNAME ) ) rv=TYPE_ATT;
    else if( !strcmp( type, IMUNAME ) ) rv=TYPE_IMU;
    else if( !strcmp( type, PASHRNAME ) ) rv=TYPE_PASHR;
    else rv=-1;
    
    free (type);
    type	= NULL;

    return rv;
}		/* -----  end of function getType  ----- */

int main( int argc, char** argv )
{
    if( argc==1 )
    {
        help(argv[0]);
        exit(EXIT_FAILURE);
    }
    int spanFd; /* File pointer to device */
    FILE *spanFile;
    ssize_t rs;
    FILE** logFiles;
    char **prams;

    // Memory allocation. The first element is not used to keep name alignment.
    logFiles	= (FILE **) calloc ( (size_t)(PARAMSZ), sizeof(FILE*) );
    if ( logFiles==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }
    prams	= (char ** )calloc ( (size_t)(PARAMSZ), sizeof(char **) );
    if ( prams==NULL ) {
        fprintf ( stderr, "\ndynamic memory allocation failed\n" );
        exit (EXIT_FAILURE);
    }

    readargs( argc, argv, prams);

    /* Open the device */
    if( (spanFd=open( prams[DEVICE], O_RDWR | O_NOCTTY ))==-1 )
        err_sys("open %s", prams[DEVICE]);
    spanFile = fdopen( spanFd, "r" );
    /* sets to 115200, 8N1 */
    set_interface_attribs( spanFd, B115200, 0);

    /* Open log files  and start SPAN log*/
    write( spanFd, STOPLOGGING, strlen(STOPLOGGING) );
    for( int i=1; i<PARAMSZ; ++i ) // maintains name alignment.
    {
        if( prams[i]==NULL ) 
        {
            logFiles[i]=NULL;
            continue;
        }
        // Open log.
        if( (logFiles[i]=fopen(prams[i],"w"))==NULL )
            err_sys("Cannot open %s", prams[i]);
        // Start corresponding span logging.
        switch ( i ) {
            case TYPE_GPS:	
                write( spanFd, GETBESTPOS, strlen(GETBESTPOS) );
                break;

            case TYPE_IMU:	
                write( spanFd, GETIMUDATA, strlen(GETIMUDATA) );
                break;

            case TYPE_ATT:	
                write( spanFd, GETATT, strlen(GETATT) );
                break;

            case TYPE_PASHR:	
                write( spanFd, GETPASHR, strlen(GETPASHR) );
                break;

            default:	
                break;
        }				/* -----  end switch  ----- */
    }

    while( 1 )
    {
        int type;
        char buf[MAXMSG];
        int numChars;
        int gData_status;
        /* clear buffer */
        memset(&buf[0], 0, sizeof(buf));
            
        /* read line */
        //if( (numChars=read( spanFd, &buf, MAXMSG ))==-1 )
         //   err_sys("read serial");
        fgets( buf, MAXMSG, spanFile );
                    
        /* Checksum */
        if( !isValid( buf ) )
        {
            fprintf( stderr, "Bad checksum: %s\n", buf );
            continue;
        }
        if( (type=getType(buf))==-1 )
            fprintf( stderr, "Unknown type: %s\n", buf );
        // TODO: test if file open
        if( logFiles[type]!=NULL )
        {
            fprintf( logFiles[type], "%s\n", buf );
        }
        else if( prams[type]!=NULL )
        {
            fprintf( stderr, "File %d should be open. Trying to reopen.\n", type );
            if( (logFiles[type]=fopen(prams[type],"w"))==NULL )
                err_sys("Cannot open %s", prams[type]); // TODO don't crash, just warn
        }
        printf("%s\n", buf); // Prints all to stdout
    }

    free (prams);
    prams	= NULL;
    close(spanFd);
    // TODO close in a loop
    for( int i=1; i<PARAMSZ; ++i )
    {
        if( prams[i]!=NULL )
            fclose(logFiles[i]);
    }
    free (logFiles);
    logFiles	= NULL;
    return EXIT_SUCCESS;
}
