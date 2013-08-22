/* Program:   XUPTIME OS/2 1.1
   Platform:  OS/2
   Compilers: Borland C, EMX GNU C, IBM C-Set
   Compile:   $(CC) uptime.c
   Purpose:   Print the uptime of the machine based on the creation date
              of the swap file. In contrast too tools querying the uptime
              timer of OS/2, this can also cover uptimes > ~ 50 days.
   Author:    Tobias Ernst, tobi@bland.fido.de
   License:   Public Domain
*/

#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSDATETIME
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define eatwhite(cp) { while (*(cp) == ' ' || *(cp) == '\n' || *(cp) == '\t') { (cp)++; } }



/* get_boot_drive: get the drive letter of this OS/2 system's boot drive */

char get_boot_drive(void)
{
    ULONG StartIndex = QSV_BOOT_DRIVE;
    ULONG LastIndex = QSV_BOOT_DRIVE;
    UCHAR DataBuf[4];
    ULONG DataBufLen = 4;
    APIRET rc;

    rc = DosQuerySysInfo(StartIndex, LastIndex, DataBuf, DataBufLen);
    if (rc != 0)
    {
        fprintf(stderr, "DosQuerySysInfo returned %ld\n", rc);
        exit(0);
    }
    return (DataBuf[0] + 'A' -1);
}



/* get_swap_file: get the name of the swap file of this OS/2 system by
                  parsing it's config.sys file.  */

char *get_swap_file(void)
{
    char configsys[] = "X:\\CONFIG.SYS";
    char buffer[256], *cp, *swap_file = NULL;
    FILE *f;
    int i;

    configsys[0] = get_boot_drive();
    if ((f = fopen(configsys, "r")) == NULL)
    {
        fprintf (stderr, "Could not open %s for reading.\n", configsys);
        exit(0);
    }
    while ( (cp = fgets(buffer, 256, f)) != NULL)
    {
        eatwhite(cp);
        if (!strnicmp(cp, "SWAPPATH", 8))
        {
            cp += 8;
            eatwhite(cp);
            if (*cp == '=')
            {
                cp++;
                eatwhite(cp);
                for (i = 0;
                     cp[i] != ' ' && cp[i] != '\t'
                     && cp[i] != '\n' && cp[i] != '\0';
                     i++);
                swap_file = malloc(i + 13);
                strncpy(swap_file, cp, i);
                if (swap_file[i - 1] != '\\')
                {
                    swap_file[i++] = '\\';
                }
                strcpy(swap_file + i, "SWAPPER.DAT");

                return (swap_file);
            }
        }

        /* eat rest of line if it was > 256 chars */
        while (*buffer && buffer[strlen(buffer) - 1] != '\n')
        {
            if (fgets(buffer, 256, f) == NULL)
            {
                break;
            }
        }
    }
    fprintf (stderr, "%s does not contain a SWAPPATH statement.\n", configsys);
    exit(0);
    return 0;
}


/* is_leap_year: tests if the specified year (4 digit) is a leap year */

int is_leap_year(unsigned year)
{
    if (!((year % 100) % 4))
    {
        if (!(year % 100))
        {
            if (!(year % 400))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}


/* n_days_in_month: returns the number of days in a certain month */

int n_days_in_month(unsigned month, unsigned year)
{
    switch (month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return (is_leap_year(year)) ? 29 : 28;
    }
    return 0;
}



/* n_days_since_1900: returns the number of days that have passed since
                      1/1/1900 */

unsigned long n_days_since_1900(int dd, int md, int yd)
{
  int d=1,m,y;
  unsigned long ds=0;

  for (y = 1900; y<yd; y++)
  {
      ds += is_leap_year(y) ? 366 : 365;
  }
  for (m=1; m<md; m++)
  {
      ds += n_days_in_month(m, y);
  }
  ds += dd - d;
  return ds;
}


/* get_uptime: returns the age of the swap file in seconds */

unsigned long get_uptime_from_swapfile(char *swapfile, int verbose)
{
    FILEFINDBUF3 InfoBuf;
    HDIR hDir = 0x0001;
    ULONG cSearch = 1;
    ULONG usAttrib;
    APIRET rc;
    DATETIME DateTime;
    unsigned long uptime;

    rc = DosFindFirst((UCHAR *)swapfile,
                      &hDir,
                      FILE_SYSTEM | FILE_HIDDEN | FILE_READONLY,
                      &InfoBuf,
                      sizeof InfoBuf,
                      &cSearch,
                      FIL_STANDARD);

    if (rc != 0)
    {
        fprintf(stderr, "DosFindFirst returned %ld\n", rc);
        exit(0);
    }

    DosFindClose(hDir);

    rc = DosGetDateTime(&DateTime);

    if (rc != 0)
    {
        fprintf(stderr, "DosGetDateTime returned %ld\n", rc);
        exit (0);
    }

    uptime  = n_days_since_1900(DateTime.day, DateTime.month, DateTime.year);
    uptime -= n_days_since_1900(InfoBuf.fdateCreation.day,
                                InfoBuf.fdateCreation.month,
                                InfoBuf.fdateCreation.year + 1980);
    uptime *= (60L * 60L * 24L);

    uptime +=   (ULONG)DateTime.hours * 60L * 60L
              + (ULONG)DateTime.minutes * 60L
              + (ULONG)DateTime.seconds;
    uptime -=    (ULONG)InfoBuf.ftimeCreation.hours * 60L * 60L
              + (ULONG)InfoBuf.ftimeCreation.minutes * 60L
              + (ULONG)InfoBuf.ftimeCreation.twosecs * 2L;


    if (verbose)
    {
        printf ("OS/2 swap file used:    %s\n", swapfile);
        printf ("Swapfile creation date: %02d/%02d/%04d, %02d:%02d\n",
           InfoBuf.fdateCreation.month, InfoBuf.fdateCreation.day,
           InfoBuf.fdateCreation.year + 1980,
           InfoBuf.ftimeCreation.hours, InfoBuf.ftimeCreation.minutes);
        printf ("Last modification:      %02d/%02d/%04d, %02d:%02d\n",
           InfoBuf.fdateLastWrite.month, InfoBuf.fdateLastWrite.day,
           InfoBuf.fdateLastWrite.year + 1980,
           InfoBuf.ftimeLastWrite.hours, InfoBuf.ftimeLastWrite.minutes);
        printf ("Last read access:       %02d/%02d/%04d, %02d:%02d\n",
           InfoBuf.fdateLastAccess.month, InfoBuf.fdateLastAccess.day,
           InfoBuf.fdateLastAccess.year + 1980,
           InfoBuf.ftimeLastAccess.hours, InfoBuf.ftimeLastAccess.minutes);
        printf ("Current date is:        %02d/%02d/%04d, %02d:%02d\n",
           DateTime.month, DateTime.day, DateTime.year,
           DateTime.hours, DateTime.minutes);
        printf ("-------------------------------------------------\n");
    }

    return uptime;
}



/* get_uptime_from_os2_timer: get the "uptime" from QSV_MS_COUNT */

unsigned long get_uptime_from_os2_timer(int verbose)
{
    ULONG StartIndex = QSV_MS_COUNT;
    ULONG LastIndex = QSV_MS_COUNT;
    ULONG ms_count;
    ULONG DataBufLen = 4;
    APIRET rc;

    rc = DosQuerySysInfo(StartIndex, LastIndex,
                         (UCHAR *)(&ms_count), DataBufLen);
    if (rc != 0)
    {
        fprintf(stderr, "DosQuerySysInfo returned %ld\n", rc);
        exit(0);
    }

    if (verbose)
    {
        printf ("Milliseconds passed since last timer wrap: %lu\n", ms_count);
        printf ("Note: There is no way to determine how many times the timer has wrapped,\n");
        printf ("so the calculated uptime value below may be incorrect.\n\n");
    }
    return ms_count / (ULONG)1000;
}


void print_help(void)
{
    printf ("XUPTIME OS/2 1.1 written by Tobias Ernst (tobi@bland.fido.de)\n");
    printf ("=============================================================\n\n");
    printf ("An extended uptime meter for OS/2 that can handle uptimes greater than 50 days.\n\n");
    printf ("Usage:\n");
    printf ("         xuptime [-h] [-?] [-2] [-v]\n\n");
    printf ("Meaning of the options:\n");
    printf ("         -h   Print this help scren\n");
    printf ("         -?   Same as -h\n");
    printf ("         -2   Query uptime from OS/2 system timer. This does only work if the\n");
    printf ("              uptime is < 50 days. If you do NOT specify this parameter, the\n");
    printf ("              uptime is calculated from the creation date of the swap file,\n");
    printf ("              which will also work for uptimes >= 50 days.\n");
    printf ("         -v   Print more verbose information (only works if -2 not specified).\n\n");
}

int main(int argc, char **argv)
{
    int i, verbose = 0, use_os2_timer = 0;
    char *s;
    unsigned long uptime, days, hours, minutes, seconds;
    char *hostname = getenv("HOSTNAME");


    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' || argv[i][0] == '/')
        {
            switch(argv[i][1])
            {
            case 'h':
            case '?':
                print_help();
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case '2':
                use_os2_timer = 1;
                break;
            }
        }
    }

    if (use_os2_timer)
    {
        uptime = get_uptime_from_os2_timer(verbose);
    }
    else
    {
        s = get_swap_file();
        uptime = get_uptime_from_swapfile(s, verbose);
    }

    if (hostname == NULL)
    {
        hostname = "localhost";
    }

    days    = uptime / (60L * 60L * 24L);
    uptime  = uptime % (60L * 60L * 24L);
    hours   = uptime / (60L * 60L);
    uptime  = uptime % (60L * 60L);
    minutes = uptime / (60L);
    seconds = uptime % (60L);

    printf ("%s: uptime is %ld day%s, %02ld:%02ld hours and %02ld second%s\n",
            hostname, days, "s" + (days == 1), hours,
            minutes, seconds, "s" + (seconds == 1));

    free(s);
    return days;
}

