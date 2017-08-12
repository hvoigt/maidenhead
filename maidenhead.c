#include <stdio.h>
#include <stdlib.h>

enum latitude_direction {
    NORTH,
    SOUTH
};

enum longitude_direction {
    WEST,
    EAST
};

static double tri2d(int degree, int minute, int second)
{
    return (double)degree +
        (double)minute/60.0 +
        (double)second/3600.0;
}

static double lat2abs(double lat, enum latitude_direction dir)
{
    if (dir == NORTH)
        return 90.0 + lat;

    /* SOUTH */
    return 90.0 - lat;
}

static double lon2abs(double lon, enum longitude_direction dir)
{
    if (dir == WEST)
        return 180.0 - lon;

    /* EAST */
    return 180.0 + lon;
}

static char n2char(char start, double n, double n_max, int base,
        double *remainder)
{
    int index = (int) (n / n_max * base);
    *remainder = n - ((double) index / (double) base * n_max);
    return start + index;
}

static void abs2locator(double lon, double lat, char *locator, size_t len)
{
    int pos = 0;
    double lat_rem, lon_rem;
    double lon_grid_max = 360; /* longitude maximum remainder (grid size) */
    double lat_grid_max = 180; /* latitude maximum remainder (grid size) */

    if (!((pos + 2) < len))
        goto out;

    /* field */
    locator[pos++] = n2char('A', lon, lon_grid_max, 18, &lon_rem);
    locator[pos++] = n2char('A', lat, lat_grid_max, 18, &lat_rem);
    if (!((pos + 2) < len))
        goto out;

    /* square */
    lon_grid_max /= 18.;
    lat_grid_max /= 18.;
    locator[pos++] = n2char('0', lon_rem, lon_grid_max, 10, &lon_rem);
    locator[pos++] = n2char('0', lat_rem, lat_grid_max, 10, &lat_rem);
    if (!((pos + 2) < len))
        goto out;

    /* subsquare */
    lon_grid_max /= 10.;
    lat_grid_max /= 10.;
    locator[pos++] = n2char('a', lon_rem, lon_grid_max, 24, &lon_rem);
    locator[pos++] = n2char('a', lat_rem, lat_grid_max, 24, &lat_rem);
    if (!((pos + 2) < len))
        goto out;

    /* extended square */
    lon_grid_max /= 24.;
    lat_grid_max /= 24.;
    locator[pos++] = n2char('0', lon_rem, lon_grid_max, 10, &lon_rem);
    locator[pos++] = n2char('0', lat_rem, lat_grid_max, 10, &lat_rem);

out:
    locator[pos] = '\0';
}

static void usage(int argc, const char *argv[])
{
    printf("Usage: %s <longitude> <latitude>\n\n", argv[0]);
    printf("Latitude and longitude are specified in "
            "degree, minute and second plus direction:\n"
            "   N - North\n"
            "   S - South\n"
            "   W - West\n"
            "   E - East\n\n"
            "Example: %s 9 57 60 E 53 32 37 N\n", argv[0]);
    exit(1);
}

int main(int argc, const char *argv[])
{
    int lon_deg, lon_min, lon_sec;
    enum longitude_direction lon_dir;
    int lat_deg, lat_min, lat_sec;
    enum latitude_direction lat_dir;

    double lat, lon;
    char maidenhead[9];

    if (argc < 9)
        usage(argc, argv);

    /*
     * Longitude
     * ---->
     */
    lon_deg = atoi(argv[1]);
    lon_min = atoi(argv[2]);
    lon_sec = atoi(argv[3]);
    switch (argv[4][0]) {
        case 'W':
            lon_dir = WEST;
            break;
        case 'E':
            lon_dir = EAST;
            break;
        default:
            printf("Error: Longitude direction needs to be W or E. "
                   "Got: '%s'\n\n", argv[4]);
            usage(argc, argv);
    }

    /*
     * Latitude
     * |
     * |
     * V
     */
    lat_deg = atoi(argv[5]);
    lat_min = atoi(argv[6]);
    lat_sec = atoi(argv[7]);
    switch (argv[8][0]) {
        case 'N':
            lat_dir = NORTH;
            break;
        case 'S':
            lat_dir = SOUTH;
            break;
        default:
            printf("Error: Latitude direction needs to be N or S. "
                   "Got: '%s'\n\n", argv[8]);
            usage(argc, argv);
    }

    /* first convert degress, minutes, seconds to one single value */
    lat = tri2d(lat_deg, lat_min, lat_sec);
    lon = tri2d(lon_deg, lon_min, lon_sec);

    printf("longitude: %f %s, latitude: %f %s\n",
            lon, lon_dir == WEST ? "W" : "E",
            lat, lat_dir == NORTH ? "N" : "S");

    /*
     * the maidenhead system expects the antimeridian of Greenwich for
     * longitude and the latitude starts at the south pole to avoid
     * negative numbers.
     */
    lat = lat2abs(lat, lat_dir);
    lon = lon2abs(lon, lon_dir);

    /* convert to grid pairs */
    abs2locator(lon, lat, maidenhead, sizeof(maidenhead));
    printf("Locator: %s\n", maidenhead);

    return 0;
}
