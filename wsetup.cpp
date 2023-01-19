#include "/home/xenos/wwv/github/wwiv/sdk/vardec.h"
#include "linuxvars.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <menu.h>
#include <ncurses.h>
#include "fidoadr.h"
#include "wsetup.h"
#define _DEBUG 1
/* ------------------------------------------------ */
/* Reads configuration options from WWIVTOSS.DAT    */
/* ------------------------------------------------ */
#define ESC 27
//#define ESC KEY_EXIT
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
tosser_config cfg;
configrec syscfg;
int registered, num_areas, num_nodes, num_uplink, maxy, maxx;
char mailer[20], areatype[20];
arearec area;
WINDOW *main_win, *sub_win, *sub_win2;

struct
{
    char domain[20];
} domains[11];


void cleanup(void)
{

    int i,j;
    arearec foo;
    FILE *temp;

    temp=fopen("areas.$$$","w+");
    fclose(temp);
    j=0;
    for (i=0; i<=num_areas; i++)
    {
        read_area(i,&foo);
        if (foo.active)
        {
            write_area2(j,&foo);
            j++;
        }
    }
    num_areas=j-1;
    unlink("areas.dat");
    rename("areas.$$$","areas.dat");
    if (num_areas<=0) init_areas_config();
}

int
get_number_of_areas (void)
{
    FILE *handle;
    int count;
    uint32_t len;
    handle = fopen ("areas.dat", "r");
    if (handle < 0)
    {
        endwin ();
        printf ("Err: Can't open AREAS.DAT.\n");
        exit (1);
    }
    fseek (handle, 0L, SEEK_END);
    len = ftell (handle);
    count = ((int) (len / sizeof (arearec)) - 1);
    fclose (handle);
    return (count);
}

const char *
yn (int yn)
{
    if (yn)
    {
        return ("Yes");
    }
    else
    {
        return ("No ");
    }
}

char
*Notify (const char *s)
{
    static char key;
    WINDOW *w;
    w = newwin (3, strlen (s) + 4, 12, 40 - (strlen (s) / 2));
    werase (w);
    box (w, 0, 0);
    mvwprintw (w, 1, 2, "%s", s);
    wrefresh (w);
    key=toupper(getch ());
    delwin (w);
    return (&key);
}

char
Ask (const char *s)
{
    int Done=0;
    static char key;
    WINDOW *w;
    w = newwin (3, strlen (s) + 4, 12, 40 - (strlen (s) / 2));
    werase (w);
    box (w, 0, 0);
    mvwprintw (w, 1, 2, "%s", s);
    wrefresh (w);
    while (!Done)
    {
        key=toupper(getch ());
        if (key=='Y'||key=='N') Done=1;
    }
    delwin (w);
    if (key=='Y') return('Y');
    return('N');
}

void
BoxInput (const char *title, char *stash, int length)
{
    char s[length + 1];
    WINDOW *w;
    w = newwin (3, length + 2, 11, 39 - (length / 2));
    werase (w);
    box (w, 0, 0);
    mvwprintw (w, 0, length / 2 - (strlen (title) / 2), "%s", title);
    wrefresh (w);
    //  refresh();
    echo ();
    curs_set (1);
    mvwgetnstr (w, 1, 1, s, length);
    curs_set (0);
    noecho ();
    delwin (w);
    strcpy (stash, s);
}

void
read_wwivtoss_config (void)
{
    FILE *configfile;

    configfile = fopen ("wwivtoss.dat", "r");
    if (configfile <= 0)
    {
        endwin ();
        printf ("Error opening WWIVTOSS.DAT!\r\n");
        exit (1);
    }
    fread (&cfg, 1, sizeof (tosser_config), configfile);
    fclose (configfile);
    /* registered=cfg.registered; MMH removed */
    registered = 1;
    cfg.registered = 1;
    cfg.add_hard_cr = cfg.add_line_feed = 1;
    cfg.soft_to_hard = cfg.hard_to_soft = cfg.add_soft_cr = 0;
    num_areas = cfg.num_areas = get_number_of_areas ();

}

void
read_wwiv_config (void)
{
    FILE *configfile;

    configfile = fopen ("config.dat", "r");
    if (configfile <= 0)
    {
        endwin ();
        printf ("Error opening WWIV CONFIG.DAT!\r\n");
        printf
        ("\r\n\r\n\7Error:  WSETUP Must be run from the main WWIV directory!\r\n\r\n");
        exit (1);
    }
    fread (&syscfg, 1, sizeof (configrec), configfile);
    fclose (configfile);
}

void
write_wwivtoss_config (void)
{
    FILE *configfile;

    configfile = fopen ("wwivtoss.dat", "wb+");
    if (configfile <= 0)
    {
        printw ("Error opening WWIVTOSS.DAT!\r\n");
        exit (1);
    }
    fwrite (&cfg, 1, sizeof (tosser_config), configfile);
    fclose (configfile);
}

void
read_domain_config (void)
{
    FILE *handle;
    handle = fopen ("domains.dat", "rb");
    if (handle <= 0)
        exit (1);
    fread (&domains, 1, sizeof (domains), handle);
    fclose (handle);
}

void
write_domain_config (void)
{
    FILE *handle;
    handle = fopen ("domains.dat", "wb+");
    if (handle <= 0)
        exit (1);
    fwrite (&domains, 1, sizeof (domains), handle);
    fclose (handle);
}

void
read_nodes_config(void)
{
    FILE *handle;
    handle = fopen("nodes.dat", "rb");
    if (handle <= 0 )
        exit (1);
    fread (&nodes, 1, sizeof (nodes), handle);
    fclose (handle);
    num_nodes=cfg.num_nodes;
}

void
write_nodes_config (void)
{
    FILE *handle;
    handle = fopen ("nodes.dat", "wb+");
    if (handle <= 0)
        exit (1);
    fwrite (&nodes, 1, sizeof (nodes), handle);
    fclose (handle);
}

void
read_uplink_config(void)
{
    FILE *handle;
    handle = fopen("uplinks.dat", "rb");
    if (handle <= 0 )
        exit (1);
    fread (&uplink, 1, sizeof (uplink), handle);
    fclose (handle);
    num_uplink=cfg.num_uplinks;
    if (num_uplink==0) cfg.num_uplinks=num_uplink=1;
}

void
write_uplink_config (void)
{
    FILE *handle;
    handle = fopen ("uplink.dat", "wb+");
    if (handle <= 0)
        exit (1);
    fwrite (&uplink, 1, sizeof (uplink), handle);
    fclose (handle);
    cfg.num_uplinks=num_uplink;
}

void
init_areas_config(void)
{
    int i;
    strcpy(area.name,"");
    strcpy(area.comment,"");
    area.group=0;
    strcpy(area.origin,"");
    strcpy(area.subtype,"");
    strcpy(area.comment,"");
    strcpy(area.directory,"");
    strcpy(area.res_char,"");
    area.type=1;
    area.alias_ok=0;
    area.pass_color=0;
    area.high_ascii=0;
    area.keep_tag=0;
    area.net_num=0;
    area.group=0;
    area.count=0;
    area.active=1;
    area.translate=0;
    area.val_incoming=0;
    for (i=0; i<MAX_NODES; i++)
        area.feed[i].zone=999;
    i=0;
    FILE *f;
    f = fopen ("areas.dat", "wb+");
    fclose (f);
    write_area(i,&area);
    strcpy(area.name,"AREA_1");
    strcpy(area.comment,"First Area");
    area.group=0;
    strcpy(area.origin,cfg.origin_line);
    strcpy(area.subtype,"1");
    i=1;
    write_area(i,&area);
    cfg.num_areas=1;
}

char *
mailer_type_string (int mailer_type)
{
    switch (mailer_type)
    {
    case 0:
        sprintf (mailer, "FrontDoor");
        break;
    case 1:
        sprintf (mailer, "InterMail");
        break;
    case 2:
        sprintf (mailer, "BinkleyTerm");
        break;
    case 3:
        sprintf (mailer, "Portal");
        break;
    case 4:
        sprintf (mailer, "DBridge");
        break;
    }
    return mailer;
}

const char *
packet_type_string (int packet_type)
{
    switch(packet_type)
    {
    case 0:
        return("2.0 (Stoneage)");
        break;
    case 1:
        return("2.+ (FSC-0039)");
        break;
    case 2:
        return("2.N (FSC-0048)");
        break;
    case 3:
        return("2.2 (FSC-0045)");
        break;
    default:
        return("2.0 (Stoneage)");
        break;
    }
}

char *
area_type_string (int area_type)
{
    switch (area_type)
    {
    case 0:
        sprintf (areatype, "Pass-through   ");
        break;
    case 1:
        sprintf (areatype, "Import to WWIV ");
        break;
    case 2:
        sprintf (areatype, "Save in *.MSG  ");
        break;
    }
    return areatype;
}

void
write_area (long un, arearec * a)
{
    long pos;
    FILE *handle;

    for (int j = 0; j < MAX_NODES; j++)
    {
        if (a->feed[j].zone == 10)
            a->feed[j].zone = 32765;
        if (a->feed[j].net == 10)
            a->feed[j].net = 32765;
        if (a->feed[j].node == 10)
            a->feed[j].node = 32765;
        if (a->feed[j].point == 10)
            a->feed[j].point = 32765;
    }
    if (handle = fopen ("areas.dat", "rb+"))
    {
        pos = ((long) sizeof (arearec)) * ((long) un);
        fseek (handle, pos, SEEK_SET);
        fwrite ((void *) a, 1, sizeof (arearec), handle);
        fclose (handle);
    }
}
void
write_area2 (long un, arearec * a)
{
    long pos;
    FILE *handle;

    for (int j = 0; j < MAX_NODES; j++)
    {
        if (a->feed[j].zone == 10)
            a->feed[j].zone = 32765;
        if (a->feed[j].net == 10)
            a->feed[j].net = 32765;
        if (a->feed[j].node == 10)
            a->feed[j].node = 32765;
        if (a->feed[j].point == 10)
            a->feed[j].point = 32765;
    }
    if (handle = fopen ("areas.$$$", "rb+"))
    {
        pos = ((long) sizeof (arearec)) * ((long) un);
        fseek (handle, pos, SEEK_SET);
        fwrite ((void *) a, 1, sizeof (arearec), handle);
        fclose (handle);
    }
}

void
read_area (long un, arearec * a)
{
    long pos, nu;
    FILE *handle;

    handle = fopen ("areas.dat", "rb+");
    if (handle < 0)
    {
        return;
    }

    nu = ((long) (filelength (handle) / sizeof (arearec)) - 1);

    if (un > nu)
    {
        fclose (handle);
        return;
    }
    pos = ((long) sizeof (arearec)) * ((long) un);
    fseek (handle, pos, SEEK_SET);
    fread ((void *) a, 1, sizeof (arearec), handle);
    fclose (handle);
    for (int j = 0; j < MAX_NODES; j++)
    {
        if (a->feed[j].zone == 32765)
            a->feed[j].zone = 10;
        if (a->feed[j].net == 32765)
            a->feed[j].net = 10;
        if (a->feed[j].node == 32765)
            a->feed[j].node = 10;
        if (a->feed[j].point == 32765)
            a->feed[j].point = 10;
    }
}

void
print_wwivtoss_config ()
{
    clear ();
    printw ("BBS Name: %s", cfg.bbs_name);
    printw ("Sysop Name: %s", cfg.sysop_name);
    printw ("Password: %s", cfg.password);
    printw ("System Address: %s", cfg.system_address);

    printw ("Origin: %s", cfg.origin_line);
    printw ("Log file: %s", cfg.log_file);
    printw ("Registration: %s", cfg.registration);
    printw ("Site Location: %s", cfg.location);
    printw ("Reserved: %s", cfg.reserved);
    printw ("Crash Mail: %s", yn (cfg.crash));
    printw ("Direct Mail: %s", yn (cfg.direct));
    printw ("Center: %s", yn (cfg.center));
    printw ("Delete Sent: %s", yn (cfg.delete_sent));
    printw ("Import Received Mail: %s", yn (cfg.import_recd));
    printw ("Use AreaFix: %s", yn (cfg.use_areafix));
    printw ("Skip Mail: %d", cfg.skip_mail);
    printw ("Notify Author Flag: %d", cfg.notify);

    printw ("Default Compression: %d", cfg.default_compression);
    printw ("Registered Flag: %d", cfg.registered);
    printw ("Mailer Type: %s", mailer_type_string (cfg.mailer));
    refresh ();
    int key = getch ();
    clear ();
    printw ("BBS Name: %s", cfg.bbs_name);
    printw ("Installed: %d", cfg.installed);
//      printw("Date Installed: on file %lu", cfg.date_installed);
    printw ("Date Installed: %s", ctime (&cfg.date_installed));
    printw ("Cleanup Nets: %d", cfg.cleanup);
    printw ("Delete Received Mail?: %d", cfg.kill_recd);
    printw ("Number of Areas: %d", cfg.num_areas);
    printw ("Number of Nodes: %d", cfg.num_nodes);


    printw ("Log Feeds Echos? %d", cfg.log_feeds);
    printw ("Add Hard C/R: %d", cfg.add_hard_cr);
    printw ("Add Line Feed: %d", cfg.add_line_feed);
    printw ("Pass High ASCII Mail: %d", cfg.high_ascii);
    printw ("Soft To Hard C/R: %d", cfg.soft_to_hard);
    printw ("Hard To Soft C/R: %d", cfg.hard_to_soft);
    printw ("Add Soft C/R: %d", cfg.add_soft_cr);
    printw ("Route Mail?: %d", cfg.route_me);
    printw ("Days to Keep Log: %d", cfg.log_days);
    printw ("Colorize TO: Line: %d", cfg.color_to);
    printw ("Initial Quote?: %d", cfg.initial_quote);
    printw ("Number of Uplinks: %d", cfg.num_uplinks);
    printw ("Bounce Bad Mail: %d", cfg.bounce_mail);
    printw ("Check Dupe Messages: %d", cfg.check_dupes);
    printw ("Use Existing Origin? %d", cfg.pass_origin);
    refresh ();
    key = getch ();
    clear ();
    printw ("BBS Name: %s", cfg.bbs_name);
    for (int i = 0; i < 11; i++)
    {
        if (cfg.aka_list[i].zone || cfg.aka_list[i].net || cfg.aka_list[i].node
                || cfg.aka_list[i].point)
        {
            printw ("Aka #%d: Address: %d:%d/%d.%d WWIVnet: %d Node: %d", i,
                    (int16_t) cfg.aka_list[i].zone, cfg.aka_list[i].net,
                    cfg.aka_list[i].node, cfg.aka_list[i].point,
                    cfg.aka_list[i].wwiv_netnum, cfg.aka_list[i].wwiv_node);
        }
    }
    for (int i = 0; i < 11; i++)
    {
        if (cfg.route_to[i].route_zone)
        {
            printw ("Route Zone %d to %d:%d/%d.%d", cfg.route_to[i].route_zone,
                    cfg.route_to[i].zone, cfg.route_to[i].net,
                    cfg.route_to[i].node, cfg.route_to[i].point);
        }
    }
    refresh ();
    key = getch ();
    clear ();
    printw ("BBS Name: %s", cfg.bbs_name);

    refresh ();
    key = getch ();
    clear ();
    printw ("BBS Name: %s", cfg.bbs_name);
    printw ("Groups======");
    for (int i = 0; i < 10; i++)
    {
        printw ("Group #%d: ID %s: %s", i, cfg.group[i].group_id,
                cfg.group[i].group_name);
    }
    refresh ();
    key = getch ();
}

void
setup_archivers(void)
{
    strcpy(cfg.archive[0].name,"PKT");
    strcpy(cfg.archive[1].name,"ARC");
    strcpy(cfg.archive[2].name,"ARJ");
    strcpy(cfg.archive[3].name,"LZH");
    strcpy(cfg.archive[4].name,"PAK");
    strcpy(cfg.archive[5].name,"ZIP");
    strcpy(cfg.archive[6].name,"ZOO");
    strcpy(cfg.archive[7].name,"SQZ");
    strcpy(cfg.archive[8].name,"RAR");
    strcpy(cfg.archive[0].archive_file,"");
    strcpy(cfg.archive[0].unarchive_file,"");
    strcpy(cfg.archive[1].archive_file,"PKPAK ");
    strcpy(cfg.archive[1].unarchive_file,"ARCE ");
    strcpy(cfg.archive[2].archive_file,"ARJ ");
    strcpy(cfg.archive[2].unarchive_file,"ARJ ");
    strcpy(cfg.archive[3].archive_file,"LHA ");
    strcpy(cfg.archive[3].unarchive_file,"LHA ");
    strcpy(cfg.archive[4].archive_file,"PAK ");
    strcpy(cfg.archive[4].unarchive_file,"PAK");
    strcpy(cfg.archive[5].archive_file,"PKZIP");
    strcpy(cfg.archive[5].unarchive_file,"PKUNZIP");
    strcpy(cfg.archive[6].archive_file,"ZOO ");
    strcpy(cfg.archive[6].unarchive_file,"ZOO ");
    strcpy(cfg.archive[7].archive_file,"SQZ ");
    strcpy(cfg.archive[7].unarchive_file,"SQZ ");
    strcpy(cfg.archive[8].archive_file,"RAR ");
    strcpy(cfg.archive[8].unarchive_file,"RAR ");
    strcpy(cfg.archive[1].archive_switches,"-a ");
    strcpy(cfg.archive[1].unarchive_switches,"");
    strcpy(cfg.archive[2].archive_switches,"a -e -m1 -s -t0 -y ");
    strcpy(cfg.archive[2].unarchive_switches,"e -c -y ");
    strcpy(cfg.archive[3].archive_switches,"a /mt ");
    strcpy(cfg.archive[3].unarchive_switches,"e /cm ");
    strcpy(cfg.archive[4].archive_switches,"a /l /st ");
    strcpy(cfg.archive[4].unarchive_switches,"e /wa");
    strcpy(cfg.archive[5].archive_switches,"-ao ");
    strcpy(cfg.archive[5].unarchive_switches,"-o -ed ");
    strcpy(cfg.archive[6].archive_switches,"a: ");
    strcpy(cfg.archive[6].unarchive_switches,"eO ");
    strcpy(cfg.archive[7].archive_switches,"a /q0 /z3 ");
    strcpy(cfg.archive[7].unarchive_switches,"e /o1 ");
    strcpy(cfg.archive[8].archive_switches,"a -y ");
    strcpy(cfg.archive[8].unarchive_switches,"e -o+ ");
}

void
init_nodes_config()
{
    FILE *handle;
    strcpy(nodes[1].address,"1:376/126.0");
    strcpy(nodes[1].sysop_name,"Craig Dooley");
    strcpy(nodes[1].pkt_pw,"");
    nodes[1].compression=0;
    nodes[1].groups=0;
    nodes[1].allow_areafix=0;
    strcpy(nodes[1].areafix_pw,"");
    nodes[1].archive_status=0;
    nodes[1].direct=0;

    if ((handle = fopen("nodes.dat","wb+"))==NULL)
        exit(1);
    fwrite(nodes, 1, sizeof (nodes), handle);
    fclose(handle);
    cfg.num_nodes=1;
}

void init_uplink_config(void)
{
    FILE *fp;

    strcpy(uplink[1].address,"");
    strcpy(uplink[1].areafix_prog,"");
    strcpy(uplink[1].areafix_pw,"");
    strcpy(uplink[1].areas_filename,"");
    strcpy(uplink[1].origin_address,"");
    uplink[1].add_plus=0;
    uplink[1].unconditional=0;
    uplink[1].areas_file_type=0;

    if ((fp = fopen("uplinks.dat","wb+"))==NULL)
        exit(1);

    fwrite(uplink,1, sizeof(uplink),fp);
    fclose(fp);
    cfg.num_uplinks=1;
    num_uplink=1;
}
void init_domains_config(void)
{
    FILE *fp;

    strcpy(domains[0].domain,"Fidonet");
    if ((fp = fopen("domains.dat","wb+"))==NULL)
        exit(1);

    fwrite(domains,1, sizeof(domains),fp);
    fclose(fp);
}
void init_xlat_dat(void)
{
    unsigned int i;
    FILE *fp;

    for (i=0; i<222; i++)
    {
        xlat[i].in=i+33;
        xlat[i].out=i+33;
    }

    if ((fp = fopen("xlat.dat","wb+"))==NULL)
        exit(1);

    fwrite(xlat, 1, sizeof (xlat),fp);
    fclose(fp);
}
void init_wwivtoss_config(void)
{
    int i;
    FILE *fp;
    time_t t;

    t=time(NULL);
    for (i=0; i<12; i++)
    {
        cfg.aka_list[i].zone=0;
        cfg.aka_list[i].net=0;
        cfg.aka_list[i].node=0;
        cfg.aka_list[i].point=0;
        cfg.aka_list[i].wwiv_node=0;
        cfg.aka_list[i].wwiv_netnum=0;
    }
    setup_archivers();
    sprintf(cfg.origin_line,"WWIVTOSS V. %s Default Origin Line",VERSION);
    cfg.notify=0;
    cfg.installed=0;
    cfg.date_installed=t;
    cfg.crash=cfg.direct=cfg.center=cfg.delete_sent=cfg.import_recd=cfg.use_areafix=cfg.skip_mail=cfg.default_compression=cfg.registered=cfg.mailer=cfg.installed=cfg.cleanup=cfg.kill_recd=cfg.auto_add=cfg.log_feeds=0;
    cfg.add_hard_cr=cfg.add_line_feed=1;
    cfg.high_ascii=1;
    cfg.soft_to_hard=cfg.hard_to_soft=cfg.add_soft_cr=0;
    cfg.route_me=1;
    cfg.log_days=0;
    cfg.color_to=0;
    cfg.initial_quote=0;
    strcpy(cfg.bbs_name,syscfg.systemname);
    strcpy(cfg.sysop_name,syscfg.sysopname);
    if ((fp = fopen("wwivtoss.dat","wb+"))==NULL)
        exit(1);

    fwrite(&cfg,1, sizeof (tosser_config),fp);
    fclose(fp);
}

void
display_main ()
{
    char line[81] = "\0";
    sprintf (line, " WWIVToss Alt Setup v0.01 : %s (%d:%d/%d.%d) ",
             syscfg.systemname, cfg.aka_list[0].zone, cfg.aka_list[0].net,
             cfg.aka_list[0].node, cfg.aka_list[0].point);
    strlen (line);
    box (stdscr, 0, 0);
    mvprintw (1, 40 - (strlen (line) / 2), "%s", line);
    refresh ();
}

void
display_title (const char *title)
{
    werase (sub_win);
    box (sub_win, 0, 0);
    mvwprintw (sub_win, 0, 38 - (strlen (title) / 2), "[ %s ]", title);
    wrefresh (sub_win);
    refresh ();
}

void
display_area (long i)
{
    arearec a;
    display_title ("Area Manager");
    read_area(i, &a);
    wattron (sub_win, COLOR_PAIR (9));
    mvwprintw (sub_win, 1, 1, "   Area %3ld of %3ld   ", i, num_areas);
    wattroff (sub_win, COLOR_PAIR (9));
    wattron (sub_win, COLOR_PAIR (1));
    mvwprintw (sub_win, 3, 2, "Name: %s", a.name);
//      mvwchgat(sub_win,3,2, 1, A_BOLD|A_UNDERLINE, 0, NULL);
    wattroff (sub_win, COLOR_PAIR (1));
    wrefresh (sub_win);
    if (a.active)
    {
        wattron (sub_win, COLOR_PAIR (0));
        mvwprintw (sub_win, 4, 2, "WWIV Subtype: %s", a.subtype);
        mvwchgat (sub_win, 4, 7, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwprintw (sub_win, 4, 25, "WWIV NetNum: %d", a.net_num);
        wattron (sub_win, COLOR_PAIR (2));
        mvwprintw (sub_win, 5, 2, "Comment: %s", a.comment);
        mvwchgat (sub_win, 5, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        wattron (sub_win, COLOR_PAIR (3));
        mvwprintw (sub_win, 6, 2, "Origin: %s", a.origin);
        mvwchgat (sub_win, 6, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        wattron (sub_win, COLOR_PAIR (4));
        mvwprintw (sub_win, 7, 2, "Directory: %s", a.directory);
        mvwchgat (sub_win, 7, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        wattron (sub_win, COLOR_PAIR (5));
//              printw("Type: Passthru, local, regular: %d", a.type);

        mvwprintw (sub_win, 8, 2, "Type: %s", area_type_string (a.type));
        mvwchgat (sub_win, 8, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        if (a.group)
        {
//            mvwprintw (sub_win, 8, 25, "Group: %c - %s   ", a.group,
//                     cfg.group[a.group - 'A'].group_name);
            mvwprintw (sub_win, 8, 25, "Group: %s   ",  cfg.group[a.group - 'A'].group_name);
            mvwchgat (sub_win, 8, 25, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        }
        else
        {
            mvwprintw (sub_win, 8, 25, "Group: None ");
            mvwchgat (sub_win, 8, 25, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        }
        mvwprintw (sub_win, 9, 2,
                   "Alias OK: %s  Pass Color: %s  High ASCII: %s  ",
                   yn (a.alias_ok), yn (a.pass_color),
                   yn (a.high_ascii));
        mvwchgat (sub_win, 9, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwchgat (sub_win, 9, 17, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwchgat (sub_win, 9, 34, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwprintw (sub_win, 10, 2, "Keep Tag: %s  ", yn (a.keep_tag));
        wprintw (sub_win, "Def Origin: %s  ", yn (a.def_origin));
        wprintw (sub_win, "XTranslate: %s  ", yn (a.translate));
        mvwchgat (sub_win, 10, 18, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwprintw (sub_win, 11, 2, "Validate Incoming: %s ",
                   yn (a.val_incoming));
        mvwchgat (sub_win, 10, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwchgat (sub_win, 10, 34, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwchgat (sub_win, 11, 2, 1, A_BOLD | A_UNDERLINE, 0, NULL);
        mvwprintw (sub_win, 12, 2, "Count: %d  Count Out: %d", a.count,
                   a.count_out);
        mvwprintw (sub_win, 13, 2, "Feed Nodes: ");
        WINDOW *feed_win = newwin (8, 63, 15, 15);
        for (int j = 0; j < MAX_NODES; j++)
        {
            if (a.feed[j].zone != 999)
                wprintw (feed_win, "%d:%d/%d.%d ", a.feed[j].zone,
                         a.feed[j].net, a.feed[j].node, a.feed[j].point);
        }
        wrefresh (feed_win);
        wattroff (sub_win, COLOR_PAIR (5));
        wrefresh (sub_win);
//        refresh();
        delwin (feed_win);
    }				// area is active
//      delwin(sub_win);
}

void
display_node(int nodenum)
{
    char s[161],s1[15];
    int i,x=0;
    for (i=0; i<15; i++) s1[i]=32;
    for (i=0; i<12; i++)
    {
        if (nodes[nodenum].groups & (1<< i))
            s1[i]='A'+i;
        else
            s1[i]=32;
    }
    s1[11]=0;
    sprintf(s,"Node # %d out of %d",nodenum,cfg.num_nodes);
    werase(sub_win);
    mvwprintw(sub_win,1,2,"%s",s);
    mvwprintw(sub_win,3,2,"Node Address    : %s",nodes[nodenum].address);
    mvwprintw(sub_win,4,2,"Node Network    : %s",nodes[nodenum].domain);
    mvwprintw(sub_win,5,2,"Node Sysop      : %s",nodes[nodenum].sysop_name);
    mvwprintw(sub_win,6,2,"Sysop Voice #   : %s",nodes[nodenum].sysop_phone);
    mvwprintw(sub_win,7,2,"Packet Password : %s",nodes[nodenum].pkt_pw);
    mvwprintw(sub_win,8,2,"Areafix Password: %s",nodes[nodenum].areafix_pw);
    mvwprintw(sub_win,9,2,"Allowed Groups  : %s",s1);
    switch(nodes[nodenum].archive_status) {
    case 0:
        strcpy(s,"NONE");
        break;
    case 1:
        strcpy(s,"HOLD");
        break;
    case 2:
        strcpy(s,"CRASH");
        break;
    default:
        strcpy(s,"NONE");
        nodes[nodenum].archive_status=0;
        break;
    }
    mvwprintw(sub_win,10,2,"Archive Status  : %s",s);
    mvwprintw(sub_win,11,2,"Compression     : %s",cfg.archive[nodes[nodenum].compression].name);
    mvwprintw(sub_win,12,2,"Packet Type     : %s",packet_type_string(nodes[nodenum].packet_type));
    wrefresh(sub_win);
}

void
edit_node(int nodenum)
{
    int bDone=0,x=0,key;
    char s[80];
    while (!bDone) {
        display_node(nodenum);
        mvwprintw(sub_win,2,2,"*** EDITING NODE ***");
        mvwchgat(sub_win,2,2,21,A_REVERSE,COLOR_RED,NULL);
        mvwchgat(sub_win,3+x,20,24,A_REVERSE|A_BOLD,0,NULL);
        wrefresh(sub_win);
        key=toupper(getch());
        if (key=='Q'||key==ESC) bDone=1;
        if (key=='J'||key==KEY_DOWN) if (++x > 9) x=0;
        if (key=='K'||key==KEY_UP) if (--x < 0) x=9;
        if (key==0xA || key==' ') {
            mvwchgat(sub_win,3+x,20,24,A_REVERSE,COLOR_BLUE,NULL);
            wrefresh(sub_win);
            if ( x>=0 && x <= 5) {
                echo();
                curs_set(1);
                wgetnstr(sub_win,s,22);
                curs_set(0);
                noecho();
                if (s[0]) {
                    if (x==0) strcpy(nodes[nodenum].address,s);
                    if (x==1) strcpy(nodes[nodenum].domain,s);
                    if (x==2) strcpy(nodes[nodenum].sysop_name,s);
                    if (x==3) strcpy(nodes[nodenum].sysop_phone,s);
                    if (x==4) strcpy(nodes[nodenum].pkt_pw,s);
                    if (x==5) strcpy(nodes[nodenum].areafix_pw,s);
                }
            } else {
                if ( x==6 ) { // allowed groups toggle A-J
                    curs_set(0);
                    int Done=0;
                    while (!Done) {
                        display_node(nodenum);
                        mvwprintw(sub_win,2,2,"*** EDITING NODE ***");
                        mvwchgat(sub_win,2,2,21,A_REVERSE,COLOR_RED,NULL);
                        mvwchgat(sub_win,3+x,20,24,A_REVERSE|A_BOLD,COLOR_BLUE,NULL);
                        wrefresh(sub_win);
                        key=toupper(getch());
                        if (key==ESC||key=='Q'||key==0xA) Done=1;
                        if (key>='A' && key<='J') {
                            int j=key-'A';
                            int k;
                            k=1<<j;
                            nodes[nodenum].groups=nodes[nodenum].groups^k;
                        }
                    }
                }
                if ( x==7 ) { // Archive Status
                    if (++nodes[nodenum].archive_status>2) nodes[nodenum].archive_status=0;
                }
                if ( x==8) { // Compression
                    if (++nodes[nodenum].compression>8) nodes[nodenum].compression=0;
                }
                if ( x==9) { // Packet Type
                    if (++nodes[nodenum].packet_type>3) nodes[nodenum].packet_type=0;
                }
            }
        }
    }
}


void
edit_uplink(int nodenum)
{   int bDone=0,x=0,key;
    char s[80];
    while (!bDone) {
        display_uplink(nodenum);
        //        mvwprintw(sub_win,2,2,"*** EDITING NODE ***");
        //        mvwchgat(sub_win,2,2,21,A_REVERSE,COLOR_RED,NULL);
        mvwchgat(sub_win,2+x,21,24,A_REVERSE|A_BOLD,0,NULL);
        wrefresh(sub_win);
        key=toupper(getch());
        if (key=='Q'||key==ESC) bDone=1;
        if (key=='J'||key==KEY_DOWN) {
            if (++x > 11) x=0;
            if (x==1) x=2;
            if (x==6) x=7;
            if (x==9) x=10;
        }
        if (key=='K'||key==KEY_UP) {
            if (--x < 0) x=11;
            if (x==1) x=0;
            if (x==6) x=5;
            if (x==9) x=8;
        }

        /*    mvwprintw(sub_win,2,2,"Node Address      : %s",uplink[nodenum].address);
        mvwprintw(sub_win,4,2,"Areafix Program   : %s",uplink[nodenum].areafix_prog);
        mvwprintw(sub_win,5,2,"Areafix Password  : %s",uplink[nodenum].areafix_pw);
        mvwprintw(sub_win,6,2,"Unconditional Add : %s",uplink[nodenum].unconditional?"Yes":"No");
        mvwprintw(sub_win,7,2,"Use '+' To Add?   : %s",uplink[nodenum].add_plus?"Yes":"No");
        mvwprintw(sub_win,9,2,"Areas Filename    : %s",uplink[nodenum].areas_filename);
        mvwprintw(sub_win,10,2,"Areas File Type   : %s",uplink[nodenum].areas_file_type?"<AREA> <DESCRIPTION>":"Random");
        mvwprintw(sub_win,12,2,"Allowed Groups    : %s",s1);
        mvwprintw(sub_win,13,2,"Origin Address    : %s",uplink[nodenum].origin_address);
        wrefresh(sub_win);
        */

        if (key==0xA || key==' ') {
            mvwchgat(sub_win,2+x,21,24,A_REVERSE,COLOR_BLUE,NULL);
            wrefresh(sub_win);
            if ( x>=0 && x <= 5) {
                echo();
                curs_set(1);
                wgetnstr(sub_win,s,22);
                curs_set(0);
                noecho();
                if (s[0]) {
                    if (x==0) strcpy(nodes[nodenum].address,s);
                    if (x==1) strcpy(nodes[nodenum].domain,s);
                    if (x==2) strcpy(nodes[nodenum].sysop_name,s);
                    if (x==3) strcpy(nodes[nodenum].sysop_phone,s);
                    if (x==4) strcpy(nodes[nodenum].pkt_pw,s);
                    if (x==5) strcpy(nodes[nodenum].areafix_pw,s);
                }
            } else {
                if ( x==6 ) { // allowed groups toggle A-J
                    curs_set(0);
                    int Done=0;
                    while (!Done) {
                        display_node(nodenum);
                        mvwprintw(sub_win,2,2,"*** EDITING NODE ***");
                        mvwchgat(sub_win,2,2,21,A_REVERSE,COLOR_RED,NULL);
                        mvwchgat(sub_win,3+x,20,24,A_REVERSE,COLOR_BLUE,NULL);
                        wrefresh(sub_win);
                        key=toupper(getch());
                        if (key==ESC||key=='Q'||key==0xA) Done=1;
                        if (key>='A' && key<='J') {
                            int j=key-'A';
                            int k;
                            k=1<<j;
                            nodes[nodenum].groups=nodes[nodenum].groups^k;
                        }
                    }
                }
                if ( x==7 ) { // Archive Status
                    if (++nodes[nodenum].archive_status>2) nodes[nodenum].archive_status=0;
                }
                if ( x==8) { // Compression
                    if (++nodes[nodenum].compression>8) nodes[nodenum].compression=0;
                }
                if ( x==9) { // Packet Type
                    if (++nodes[nodenum].packet_type>3) nodes[nodenum].packet_type=0;
                }
            }
        }
    }
}
void
display_uplink(int nodenum)
{
    char s[161],s1[15];
    int i;
    for (i=0; i<15; i++)
        s1[i]=32;
    for (i=0; i<12; i++)
    {
        if (uplink[nodenum].groups & (1<<i))
            s1[i]='A'+i;
        else
            s1[i]=32;
    }
    s1[11]=0;
    sprintf(s,"Uplink # %d out of %d",nodenum,num_uplink);
    werase(sub_win);
    mvwprintw(sub_win,1,1,"%s",s);
    mvwprintw(sub_win,2,2,"Node Address      : %s",uplink[nodenum].address);
    mvwprintw(sub_win,4,2,"Areafix Program   : %s",uplink[nodenum].areafix_prog);
    mvwprintw(sub_win,5,2,"Areafix Password  : %s",uplink[nodenum].areafix_pw);
    mvwprintw(sub_win,6,2,"Unconditional Add : %s",uplink[nodenum].unconditional?"Yes":"No");
    mvwprintw(sub_win,7,2,"Use '+' To Add?   : %s",uplink[nodenum].add_plus?"Yes":"No");
    mvwprintw(sub_win,9,2,"Areas Filename    : %s",uplink[nodenum].areas_filename);
    mvwprintw(sub_win,10,2,"Areas File Type   : %s",uplink[nodenum].areas_file_type?"<AREA> <DESCRIPTION>":"Random");
    mvwprintw(sub_win,12,2,"Allowed Groups    : %s",s1);
    mvwprintw(sub_win,13,2,"Origin Address    : %s",uplink[nodenum].origin_address);
    wrefresh(sub_win);
}

void
AreaManager ()
{
    long i = 1;
    int key = 0, bDone = 0;
    while (!bDone)
    {
        display_area (i);
        key = toupper (getch ());
        switch (key)
        {
        case KEY_RIGHT:
        case ']':
            if (++i > num_areas)
                i = 1;
            break;
        case KEY_LEFT:
        case '[':
            if (--i < 1)
                i = num_areas;
            break;
        case KEY_UP:
            i += 10;
            if (i > num_areas)
                i = num_areas;
            break;
        case KEY_DOWN:
            i -= 10;
            if (i < 0)
                i = 0;
            break;
        case KEY_HOME:
        case '<':            // Go to start of areas
            i = 1;
            break;
        case KEY_END:
        case '>':            // Go to end of areas
            i = num_areas;
            break;
        case 'A':            // Toggle Alias_OK
            area.alias_ok = not (area.alias_ok);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'H':            // Toggle High ASCII
            area.high_ascii = not (area.high_ascii);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'K':            // Toggle Keep Taglines
            area.keep_tag = not (area.keep_tag);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'P':            // Toggle Pass Color
            area.pass_color = not (area.pass_color);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'X':            // Toggle Translate
            area.translate = not (area.translate);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'V':            // Toggle Validate Incoming
            area.val_incoming = not (area.val_incoming);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'G':            // Toggle Group
            if (area.group)
            {
                area.group++;
            }
            else
            {
                area.group = 'A';
            }
            if (area.group > 'A' + 9)
                area.group = 0;
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'N':		// Enter Area Name
            mvwchgat (sub_win, 3, 8, 60, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            wgetnstr (sub_win, area.name, 60);
            noecho ();
            if (area.name[0] == 0)
            {
                break;
            }
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'S':		// Enter WWIV Subtype
            mvwchgat (sub_win, 4, 16, 7, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            wgetnstr (sub_win, area.subtype, 7);
            noecho ();
            if (area.subtype[0] == 0)
            {
                break;
            }
            else
            {
                for (int j = 0; j < strlen (area.subtype); j++)
                {
                    area.subtype[j] = toupper (area.subtype[j]);
                }
            }
            area.net_num = 1;
            area.type = 1;
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'C':		// Enter WWIV Comment
            mvwchgat (sub_win, 5, 11, 60, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            curs_set (2);
            wgetnstr (sub_win, area.comment, 60);
            curs_set (0);
            noecho ();
            if (area.comment[0] == 0)
            {
                break;
            }
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'O':              // Enter Origin
            mvwchgat (sub_win, 6, 10, 60, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            curs_set (1);
            wgetnstr (sub_win, area.origin, 60);
            curs_set (0);
            noecho ();
            if (area.origin[0] == 0)
            {
                read_area (i, &area);
                area.def_origin = not (area.def_origin);
            }
            else
            {
                area.def_origin = 0;
            }
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'E':
            area.def_origin = not (area.def_origin);
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'D':		// Enter Directory
            mvwchgat (sub_win, 7, 13, 60, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            curs_set(1);
            wgetnstr (sub_win, area.directory, 60);
            curs_set(0);
            noecho ();
            if (area.directory[0] == 0)
            {
                break;
            }
            write_area (i, &area);
            read_area (i, &area);
            break;
        case 'T':		// Toggle Type (Pass-through, Import to WWIV, Save in *.MSG)
            if (++area.type >= 3)
                area.type = 0;
            write_area (i, &area);
            break;
        case '!':		// Toggle Active
            area.active = not (area.active);
            write_area (i, &area);
            break;
        case KEY_IC:
        case '+':		// Add an area
            read_area (0, &area);
            i = num_areas + 1;
            write_area (i, &area);
            num_areas = get_number_of_areas ();
            break;
        case KEY_DC:
        case '-': // Delete an area
            // TODO: delete_area(i);
            char key2;
            key2=Ask("Delete this area?");
            if (key2=='N') break;
            Notify("Deleting area... (not really yet)");
            break;
        case ESC:
        case 'Q':
            bDone=1;
            break;
        default:
            break;
        }
        read_area (i, &area);
        wrefresh (sub_win);
    }
    sort_areas();
    cleanup();
}

void
SetUpScreen ()
{
    initscr ();
    set_escdelay (1);
    getmaxyx (stdscr, maxy, maxx);
    start_color ();
    // initialize all colors
    init_pair (1, COLOR_WHITE, COLOR_RED);
    init_pair (2, COLOR_GREEN, COLOR_BLACK);
    init_pair (3, COLOR_BLUE, COLOR_BLACK);
    init_pair (4, COLOR_CYAN, COLOR_BLACK);
    init_pair (5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair (9, COLOR_WHITE, COLOR_BLUE);
    nl ();
    raw ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
}

void
SiteInfo ()
{
    int bDone = 0, x = 0, key, suby, subx;
    char s[81];
    display_main ();
    sub_win2 = newwin (8, 76, 8, 2);
    while (!bDone)
    {
        display_title ("Site Information");
        werase (sub_win2);
        box (sub_win2, 0, 0);
        mvwprintw (sub_win2, 1, 1, "[1] BBS Name: %s", cfg.bbs_name);
//    mvwprintw(sub_win2,2,38-(strlen(cfg.bbs_name)/2),"%s"
        mvwprintw (sub_win2, 2, 1, "[2] Sysop Name: %s", cfg.sysop_name);
//    mvwprintw(sub_win2,4,38-(strlen(cfg.sysop_name)/2),"%s", cfg.sysop_name);
        mvwprintw (sub_win2, 3, 1, "[3] Site Location: %s", cfg.location);
        mvwprintw (sub_win2, 4, 1, "[4] Password: %s", cfg.password);
        mvwprintw (sub_win2, 5, 1, "[5] Origin: %s", cfg.origin_line);
//      mvwprintw(sub_win2,6,1,"System Address: %s", cfg.system_address);
        mvwprintw (sub_win2, 6, 1, "[6] Log file: %s", cfg.log_file);
//      mvwprintw(sub_win2,7,1,"[7] Registration: %s", cfg.registration);
//      mvwprintw(sub_win2,9,1,"Reserved: %s", cfg.reserved);
        mvwchgat (sub_win2, 1 + x, 1, 74, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win2);
        key = toupper (getch ());
        mvwchgat (sub_win2, 1 + x, 1, 74, A_NORMAL, 0, NULL);
        wrefresh (sub_win2);
        switch (key)
        {

        case 0xA:
            ungetch ('1' + x);
            break;
        case 'K':
        case KEY_UP:
            if ((--x) < 0)
                x = 5;
            break;
        case 'J':
        case KEY_DOWN:
            if ((++x) > 5)
                x = 0;
            break;
        case '1':		// BBS Name cfg.bbs_name
            mvwchgat (sub_win2, 1, 15, 60, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 60);
            curs_set (0);
            noecho ();
            /*
               BoxInput("BBS Name",s,76);
             */
            refresh ();
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                strcpy (cfg.bbs_name, s);
            }
            break;
        case '2':		// Sysop Name cfg.sysop_name
            mvwchgat (sub_win2, 2, 17, 58, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 58);
            curs_set (0);
            noecho ();
//                      BoxInput("Sysop's Real Name",s,78);
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                strcpy (cfg.sysop_name, s);
            }
            break;
        case '3':		// cfg.location
            mvwchgat (sub_win2, 3, 20, 39, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 39);
            curs_set (0);
            noecho ();
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                strcpy (cfg.location, s);
            }
            break;
        case '4':		// cfg.password
            mvwchgat (sub_win2, 4, 15, 19, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 19);
            curs_set (0);
            noecho ();
            if (s[0] == 0)
            {
                cfg.password[0] = '\0';
            }
            else
            {
                strcpy (cfg.password, s);
            }
            break;
        case '5':		// cfg.origin_line
            mvwchgat (sub_win2, 5, 13, 62, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 62);
            curs_set (0);
            noecho ();
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                strcpy (cfg.origin_line, s);
            }
            break;
        case '6':		// cfg.log_file
            mvwchgat (sub_win2, 6, 15, 60, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win2);
            echo ();
            curs_set (1);
            wgetnstr (sub_win2, s, 60);
            curs_set (0);
            noecho ();
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                strcpy (cfg.log_file, s);
            }
            break;
        /*************************************************************************
             **     No Registration Needed - This Section is commented out
             *************************************************************************
            		case '7': // cfg.registration
            			mvwchgat(sub_win2, 7, 19, 80, A_REVERSE, 0, NULL);
                        wrefresh(sub_win2);
                        echo();
                        wgetnstr(sub_win2,s,80);
                        noecho();
                        if (s[0]==0) {
            				break;
            			} else {
            				strcpy(cfg.registration,s);
                        }
            			break;
            *************************************************************************/
        case 'Q':
        case ESC:
            bDone = 1;
        }

    }
    delwin (sub_win2);
}

void
NetworkAddresses ()
{
    int key, bDone = 0;
    char s[81], temp[81];
    clear ();
    display_main ();
    int x = 0;
    while (!bDone)
    {
        display_title ("Network Addresses and AKAs");
        mvwprintw (sub_win, 2, 2, "#");
        mvwprintw (sub_win, 2, 6, "Address");
        mvwprintw (sub_win, 2, 25, "Fake WWIV Node");
        mvwprintw (sub_win, 2, 44, "WWIV Net #");
        mvwprintw (sub_win, 2, 58, "Domain");
        for (int i = 0; i < 11; i++)
        {
            sprintf (temp, "%d:%d/%d.%d", cfg.aka_list[i].zone,
                     cfg.aka_list[i].net, cfg.aka_list[i].node,
                     cfg.aka_list[i].point);
            sprintf (s, "%-2d  %-18s      %-5u             %-2u        %s", i,
                     temp, cfg.aka_list[i].wwiv_node,
                     cfg.aka_list[i].wwiv_netnum, domains[i].domain);
            mvwprintw (sub_win, 4 + i, 2, "%s", s);
        }
        mvwchgat (sub_win, 4 + x, 2, 74, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        switch (key)
        {
        case 'K':
        case KEY_UP:
            if ((--x) < 0)
                x = 10;
            break;
        case 'J':
        case KEY_DOWN:
            if ((++x) > 10)
                x = 0;
            break;
        case KEY_DC:
            cfg.aka_list[x].zone = 0;
            cfg.aka_list[x].net = 0;
            cfg.aka_list[x].node = 0;
            cfg.aka_list[x].point = 0;
            cfg.aka_list[x].wwiv_node = 0;
            cfg.aka_list[x].wwiv_netnum = 0;
            strcpy(domains[x].domain,"");
            break;
        case 0xA:		// Edit this entry
            mvwchgat(sub_win, 4 + x, 6, 10, A_REVERSE, COLOR_BLUE, NULL);
            echo();
            curs_set(1);
            wrefresh (sub_win);
            wgetnstr(sub_win,s, 15);
            mvwchgat(sub_win, 4 + x, 6, 10, A_REVERSE, 0, NULL);
            if (s[0])
            {
                FIDOADR fadr;
                fidoadr_split(s,&fadr);
                cfg.aka_list[x].zone = fadr.zone;
                cfg.aka_list[x].net = fadr.net;
                cfg.aka_list[x].node = fadr.node;
                cfg.aka_list[x].point = fadr.point;
            }
            mvwchgat(sub_win, 4 + x, 25, 10, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win);
            wgetnstr(sub_win,s, 10);
            mvwchgat(sub_win, 4 + x, 25, 10, A_REVERSE, 0, NULL);
            if (s[0]) cfg.aka_list[x].wwiv_node = atoi(s);
            mvwchgat(sub_win, 4 + x, 44, 10, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win);
            wgetnstr(sub_win,s, 10);
            mvwchgat(sub_win, 4 + x, 44, 10, A_REVERSE, 0, NULL);
            if (s[0]) cfg.aka_list[x].wwiv_netnum = atoi(s);
            mvwchgat(sub_win, 4 + x, 58, 10, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win);
            wgetnstr(sub_win,s, 20);
            mvwchgat(sub_win, 4 + x, 58, 10, A_REVERSE, 0, NULL);
            if (s[0])
                strcpy (domains[x].domain, s);
            sprintf (temp, "%d:%d/%d.%d", cfg.aka_list[x].zone,
                     cfg.aka_list[x].net, cfg.aka_list[x].node,
                     cfg.aka_list[x].point);
            sprintf (s, "%-2d  %-18s      %-5u             %-2u        %s", x,
                     temp, cfg.aka_list[x].wwiv_node,
                     cfg.aka_list[x].wwiv_netnum, domains[x].domain);
            mvwprintw (sub_win, 4 + x, 2, "%s", s);
            noecho();
            curs_set(0);
            wrefresh (sub_win);
            write_domain_config();
            break;

        case 'Q':
        case ESC:
            bDone = 1;
            break;
            //                      default:
        }
    }
}

void
Groups ()
{
    int bDone = 0, x = 0, key, suby, subx;
    char s[81];
    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Groups");
        for (int i = 0; i < 10; i++)
        {
            sprintf(cfg.group[i].group_id,"%c",65+i);
            mvwprintw (sub_win, 5 + i, 24, "Group ID [%s]: %s",
                       cfg.group[i].group_id, cfg.group[i].group_name);
        }
        mvwchgat (sub_win, 5 + x, 24, 34, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        mvwchgat (sub_win, 5 + x, 24, 34, A_NORMAL, 0, NULL);
        if (key == KEY_UP)
            if (--x < 0)
                x = 9;
        if (key == KEY_DOWN)
            if (++x > 9)
                x = 0;
        if (key==KEY_DC) strcpy(cfg.group[x].group_name,"\0");
        if (key == 0xA)
            key = 'A' + x;
        if (key >= 'A' && key <= 'J')
        {
            int i = key - 'A';
            x = i;
            mvwchgat (sub_win, 5 + i, 38, 20, A_REVERSE, COLOR_BLUE, NULL);
            wrefresh (sub_win);
            echo ();
            curs_set (1);
            wgetnstr (sub_win, s, 20);
            curs_set (0);
            noecho ();
            if (s[0] == 0)
            {
                // break;
            }
            else
            {
                strcpy (cfg.group[i].group_name, s);
            }
        }
        if (key == 'Q' || key == ESC)
            bDone = 1;
    }
}

void
PathSetup ()
{
    int bDone = 0, x = 0, key, suby, subx;
    char s[81];
    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Path Setup");	// paths 50 char width 64 max
        mvwprintw (sub_win, 7, 1, "      Path to Inbound: %s",
                   cfg.inbound_path);
        mvwprintw (sub_win, 8, 1, "     Path to Outbound: %s",
                   cfg.outbound_path);
        mvwprintw (sub_win, 9, 1, "Path to Temp Outbound: %s",
                   cfg.outbound_temp_path);
        mvwprintw (sub_win, 10, 1, "    Netmail Directory: %s",
                   cfg.netmail_path);
        mvwprintw (sub_win, 11, 1, "       Temporary Path: %s", cfg.temp_path);
        mvwprintw (sub_win, 12, 1, "        Bad Echo Path: %s",
                   cfg.badecho_path);
        mvwchgat (sub_win, 7 + x, 1, 76, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        if (key == KEY_UP || key == 'K')
            if (--x < 0)
                x = 5;
        if (key == KEY_DOWN || key == 'J')
            if (++x > 5)
                x = 0;
        if (key == 'Q' || key == ESC)
            bDone = 1;
        if (key == 0xA)
        {
            curs_set (1);
            key = '1' + x;
            if (key == '1')
            {
                mvwchgat (sub_win, 7, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wattrset (sub_win, A_REVERSE | A_BOLD);
                wgetnstr (sub_win, s, 50);
                noecho ();
                wattrset (sub_win, A_NORMAL);
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.inbound_path, s);
//              if (!strcmp(s[strlen(s)-1],"\\")) strcat(s,"\\");
                }
                mvwprintw (sub_win, 7, 24, "%-50s", cfg.inbound_path);
            }

//
            if (key == '2')
            {
                mvwchgat (sub_win, 8, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wgetnstr (sub_win, s, 50);
                noecho ();
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.outbound_path, s);
                }
                mvwprintw (sub_win, 8, 24, "%-50s", cfg.outbound_path);
            }

//
            if (key == '3')
            {
                mvwchgat (sub_win, 9, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wgetnstr (sub_win, s, 50);
                noecho ();
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.outbound_temp_path, s);
                }
                mvwprintw (sub_win, 9, 24, "%-50s", cfg.outbound_temp_path);
            }
//
            if (key == '4')
            {
                mvwchgat (sub_win, 10, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wgetnstr (sub_win, s, 50);
                noecho ();
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.netmail_path, s);
                }
                mvwprintw (sub_win, 10, 24, "%-50s", cfg.netmail_path);
            }
//
            if (key == '5')
            {
                mvwchgat (sub_win, 11, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wgetnstr (sub_win, s, 50);
                noecho ();
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.temp_path, s);
                }
                mvwprintw (sub_win, 11, 24, "%-50s", cfg.temp_path);
            }
//
            if (key == '6')
            {
                mvwchgat (sub_win, 12, 24, 50, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh (sub_win);
                echo ();
                wgetnstr (sub_win, s, 50);
                noecho ();
                if (s[0] == 0)
                {
//                                              break;
                }
                else
                {
                    strcpy (cfg.badecho_path, s);
                }
                mvwprintw (sub_win, 12, 24, "%-50s", cfg.badecho_path);
            }
            curs_set (0);
        }
    }
}

void
edit_archiver (int i)
{
    int bDone=0,key,x=0;
    char s[80];
    WINDOW *w;
    if (i == 0)
    {
        Notify ("Hey! .PKT Files aren't compressed, silly!");
        return;
    }
    else
    {
        w = newwin (8, 60, 3, 10);

        while (!bDone) {
            werase(w);
            box (w, 0, 0);
            mvwprintw (w, 1, 2, "     Archive Type  : %s ", cfg.archive[i].name);
            mvwprintw (w, 3, 2, "  Archive command  : %s", cfg.archive[i].archive_file);
            mvwprintw (w, 4, 2, "  Archive switches : %s", cfg.archive[i].archive_switches);
            mvwprintw (w, 5, 2, "Unarchive command  : %s",
                       cfg.archive[i].unarchive_file);
            mvwprintw (w, 6, 2, "Unarchive switches : %s", cfg.archive[i].unarchive_switches);
            mvwchgat (w, 3+x, 1, 58, A_REVERSE|A_BOLD, 0, NULL);
            wrefresh (w);
            key=toupper(getch());
            if (key=='Q'||key==ESC) bDone=1;
            if (key=='J'||key==KEY_DOWN) if (++x >3) x=0;
            if (key=='K'||key==KEY_UP) if (--x <0) x=3;
            if (key==0xA) {
                mvwchgat (w, 3+x, 23, 30, A_REVERSE, COLOR_BLUE, NULL);
                wrefresh(w);
                echo ();
                curs_set(1);
                wgetnstr (w, s, 30);
                curs_set(0);
                noecho ();
                if (s[0] == 0)
                {
                }
                else
                {
                    if (x==0) strcpy(cfg.archive[i].archive_file,s);
                    if (x==1) strcpy(cfg.archive[i].archive_switches,s);
                    if (x==2) strcpy(cfg.archive[i].unarchive_file,s);
                    if (x==3) strcpy(cfg.archive[i].unarchive_switches,s);
                }

            }
        }
    }

}

void
ArchiverSetup ()
{
    int bDone = 0, x = 0, key;
    char s[81];
    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Archivers Setup");
        for (int i = 0; i < 9; i++)
        {
            mvwprintw (sub_win, 5 + i, 34, " [%d] : %s ", i,
                       cfg.archive[i].name);
        }
        mvwprintw (sub_win, 5 + (cfg.default_compression), 33, "*");
        mvwchgat (sub_win, 5 + x, 32, 15, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        if (key == KEY_DOWN || key == 'J')
            if (++x > 8)
                x = 0;
        if (key == KEY_UP || key == 'K')
            if (--x < 0)
                x = 8;
        if (key == ' ')
        {
            cfg.default_compression = x;
        }
        if (key == 0xA)
            key = '0' + x;		// ENTER key pressed
        if (key >= '0' && key <= '8')
        {
            x = key - '0';
            edit_archiver (x);
        }
        if (key == 'Q' || key == ESC) bDone = 1;

    }
}

void
UplinkManager ()
{
    int bDone=0,x=1,key,current_node;
    clear ();
    display_main ();
    display_title ("Uplink Manager");
    while (!bDone) {
        display_uplink(x);
        key=toupper(getch());
        if (key=='Q'|| key==ESC) bDone=1;
        if (key=='J'|| key==KEY_RIGHT||key==']') if (++x > cfg.num_uplinks) x=1;
        if (key=='K'|| key==KEY_LEFT||key=='[') if (--x < 1) x=cfg.num_uplinks;
        if (key==0xA) {
            edit_uplink(x);
        }
        if (key==KEY_IC) {
            x=cfg.num_uplinks;
            if (++x > MAX_NODES) {
                Notify("Warning! Maximum of 50 nodes!");
                x=MAX_NODES;    // error
            }
            cfg.num_uplinks=num_uplink=x;
            edit_uplink(x);
        }
        if (key==KEY_DC) {
            // TODO: delete_uplink(x);
            char key2=Ask("Delete! Are you sure?");
            if (key2=='Y') {
                strcpy(uplink[x].address,"");
                strcpy(uplink[x].areafix_prog,"");
                strcpy(uplink[x].areafix_pw,"");
                strcpy(uplink[x].areas_filename,"");
                strcpy(uplink[x].origin_address,"");
                uplink[x].add_plus=0;
                uplink[x].unconditional=0;
                uplink[x].areas_file_type=0;
                current_node=x;
                num_uplink--;
                for (int j=x; j<=num_uplink; j++)
                    uplink[j]=uplink[j+1];
                if (num_uplink<1)
                    num_uplink=1;
                x=current_node;
                if (x<1)
                    x=1;
                if (x>num_uplink)
                    x=num_uplink;
                cfg.num_uplinks=num_uplink;
            }
            else Notify("Not deleting...");

        }
    }
}

void
SystemData ()
{
    int bDone = 0, x = 0, key;
    display_main ();
    while (!bDone)
    {
        display_title ("System Data");
        mvwprintw (sub_win, 5, 30, "[1] Site Info");
        mvwprintw (sub_win, 6, 30, "[2] Network Addresses");
        mvwprintw (sub_win, 7, 30, "[3] Path Setup");
        mvwprintw (sub_win, 8, 30, "[4] Archiver Setup");
        mvwprintw (sub_win, 9, 30, "[5] Group Setup");
        mvwprintw (sub_win, 10, 30, "[6] Uplink Manager");
        mvwchgat (sub_win, 5 + x, 30, 21, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        switch (key)
        {
        case 0xA:
            ungetch ('1' + x);
            break;
        case 'K':
        case KEY_UP:
            if ((--x) < 0)
                x = 5;
            break;
        case 'J':
        case KEY_DOWN:
            if ((++x) > 5)
                x = 0;
            break;

        case 'S':
        case '1':
            SiteInfo ();
            break;
        case 'N':
        case '2':
            NetworkAddresses ();
            break;
        case 'P':
        case '3':
            PathSetup ();
            break;
        case 'A':
        case '4':
            ArchiverSetup ();
            break;
        case 'G':
        case '5':
            Groups ();
            break;
        case '6':
        case 'U':
            UplinkManager();
            break;
        case 'Q':
        case ESC:
            bDone = 1;
        }
    }
}

void
MiscOptions ()
{
    char s[81];
    int bDone = 0, key;
    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Miscellaneous Options");
        switch (cfg.crash)
        {
        case 0:
            sprintf (s, "Route All");
            break;
        case 1:
            sprintf (s, "Crash My Net Only");
            break;
        case 2:
            sprintf (s, "Crash All");
            break;
        }
        mvwprintw (sub_win, 3, 2, "[A] Crash Mode : %s", s);
        mvwprintw (sub_win, 3, 40, "[1] Immediate Mode: %s",
                   yn (cfg.set_immediate));
        switch (cfg.direct)
        {
        case 0:
            sprintf (s, "Not Direct");
            break;
        case 1:
            sprintf (s, "Direct My Net Only");
            break;
        case 2:
            sprintf (s, "Direct All");
            break;
        }
        mvwprintw (sub_win, 4, 2, "[B] Direct Mode: %s", s);
        mvwprintw (sub_win, 4, 40, "[2] Auto Add Areas: %s", yn (cfg.auto_add));
        mvwprintw (sub_win, 5, 2, "[C] Center: %s", yn (cfg.center));
        mvwprintw (sub_win, 5, 40, "[3] Log Passthrus: %s", yn (cfg.log_feeds));
        mvwprintw (sub_win, 6, 2, "[D] Delete Sent: %s", yn (cfg.delete_sent));
        mvwprintw (sub_win, 6, 40, "[4] Pass High ASCII: %s",
                   yn (cfg.high_ascii));
        mvwprintw (sub_win, 7, 2, "[E] Import Received Mail: %s",
                   yn (cfg.import_recd));
        mvwprintw (sub_win, 7, 40, "[5] Days to Keep Log: %d", cfg.log_days);
        mvwprintw (sub_win, 8, 2, "[F] Use AreaFix: %s", yn (cfg.use_areafix));
        mvwprintw (sub_win, 8, 40, "[6] Colorize TO: Line: %s",
                   yn (cfg.color_to));
        mvwprintw (sub_win, 9, 2, "[G] Skip Mail: %s", yn (cfg.skip_mail));
        mvwprintw (sub_win, 9, 40, "[7] Initial Quotes? : %s",
                   yn (cfg.initial_quote));
        mvwprintw (sub_win, 10, 2, "[H] Cleanup Nets: %s", yn (cfg.cleanup));
        mvwprintw (sub_win, 10, 40, "[8] Bounce Bad Email: %s",
                   yn (cfg.bounce_mail));
        mvwprintw (sub_win, 11, 2, "[I] Route Mail?: %s", yn (cfg.route_me));
        mvwprintw (sub_win, 11, 40, "[9] Check for Dupes: %s",
                   yn (cfg.check_dupes));
        mvwprintw (sub_win, 12, 2, "[J] Check Origin: %s",
                   yn (cfg.pass_origin));
        mvwprintw (sub_win, 13, 2, "[K] Mailer Type: %s",
                   mailer_type_string (cfg.mailer));
        wrefresh (sub_win);
        key = toupper (getch ());
        switch (key)
        {
        case 'A':
            if (++cfg.crash < 3)
            {
            }
            else
            {
                cfg.crash = 0;
            }
            break;
        case 'B':
            if (++cfg.direct < 3)
            {
            }
            else
            {
                cfg.direct = 0;
            }
            break;
        case 'C':
            cfg.center = !cfg.center;
            break;
        case 'D':
            cfg.delete_sent = !cfg.delete_sent;
            break;
        case 'E':
            cfg.import_recd = !cfg.import_recd;
            break;
        case 'F':
            cfg.use_areafix = !cfg.use_areafix;
            break;
        case 'G':
            cfg.skip_mail = !cfg.skip_mail;
            break;
        case 'H':
            cfg.cleanup = !cfg.cleanup;
            break;
        case 'I':
            cfg.route_me = !cfg.route_me;
            break;
        case 'J':
            cfg.pass_origin = !cfg.pass_origin;
            break;
        case 'K':
            if (++cfg.mailer < 5)
            {
            }
            else
            {
                cfg.mailer = 0;
            }
            break;
        case '1':
            cfg.set_immediate = !cfg.set_immediate;
            break;
        case '2':
            cfg.auto_add = !cfg.auto_add;
            break;
        case '3':
            cfg.log_feeds = !cfg.log_feeds;
            break;
        case '4':
            cfg.high_ascii = !cfg.high_ascii;
            break;
        case '5':
            mvwchgat (sub_win, 7, 62, 5, A_REVERSE, 0, NULL);
            wrefresh (sub_win);
            echo ();
            wgetnstr (sub_win, s, 5);
            noecho ();
            if (s[0] == 0)
            {
                break;
            }
            else
            {
                cfg.log_days = atoi (s);
            }
            break;
        case '6':
            cfg.color_to = !cfg.color_to;
            break;
        case '7':
            cfg.initial_quote = !cfg.initial_quote;
            break;
        case '8':
            cfg.bounce_mail = !cfg.bounce_mail;
            break;
        case '9':
            cfg.check_dupes = !cfg.check_dupes;
            break;
        case 0xA:
        case 'Q':
        case ESC:
            bDone = 1;
        }
    }
}


void
NodeManager ()
{
    int bDone=0,x=1,key,current_node;
    read_nodes_config();
    clear ();
    display_main ();
    display_title ("Node Manager");
    while (!bDone) {
        display_node(x);
        key=toupper(getch());
        if (key=='Q'|| key==ESC) bDone=1;
        if (key==KEY_HOME) x=1;
        if (key==KEY_END) x=num_nodes;
        if (key=='J'|| key==KEY_RIGHT || key==']') if (++x > num_nodes) x=1;
        if (key=='K'|| key==KEY_LEFT || key=='[') if (--x < 1) x=num_nodes;
        if (key==0xA) edit_node(x);
        if (key==KEY_DC) {
            char key2;
            key2=Ask("Delete! Are you sure?");
            if (key2=='Y') {
                current_node=x;
                num_nodes--;
                for (int j=x; j<=num_nodes; j++)
                    nodes[j]=nodes[j+1];
                if (num_nodes<1) num_nodes=1;
                x=current_node;
                if (x<1) x=1;
                if (x>num_nodes) x=num_nodes;
                cfg.num_nodes=num_nodes;
            }
        }
        if (key==KEY_IC)
        {
            if (++num_nodes > MAX_NODES )
            {
                Notify("Warning! Maximum of 20 nodes!");
                num_nodes = MAX_NODES - 1;
            }
            x=num_nodes;
            nodes[x].address[0]=0;
            nodes[x].sysop_name[0]=0;
            nodes[x].pkt_pw[0]=0;
            nodes[x].compression=0;
            nodes[x].groups=0;
            edit_node(x);
            cfg.num_nodes=num_nodes;
        }
    }
    write_nodes_config();
}

void
edit_route (int i)
{
    char s[80];
    char temp[80];
    int route_to;
    FIDOADR fadr;
    mvwchgat (sub_win, 4 + i, 12, 18, A_REVERSE, COLOR_BLUE, NULL);
    wrefresh (sub_win);
    echo ();
    curs_set (1);
    wgetnstr (sub_win, s, 50);
    if (strlen(s) > 1) {
        fidoadr_split(s, &fadr);
        cfg.route_to[i].zone=fadr.zone;
        cfg.route_to[i].net=fadr.net;
        cfg.route_to[i].node=fadr.node;
        cfg.route_to[i].point=fadr.point;
    }

    curs_set (0);
    noecho ();
    mvwprintw (sub_win, 4 + i, 12, "%d:%d/%d.%d", cfg.route_to[i].zone, cfg.route_to[i].net, cfg.route_to[i].node, cfg.route_to[i].point);
    wrefresh (sub_win);
    mvwchgat (sub_win, 4 + i, 34, 5, A_REVERSE, COLOR_BLUE, NULL);
    wrefresh (sub_win);
    echo ();
    curs_set (1);
    wgetnstr (sub_win, s, 5);
    curs_set (0);
    noecho ();
    if (s) {
        if (route_to=atoi(s)) cfg.route_to[i].route_zone = route_to;
        mvwprintw (sub_win, 4 + i, 34, "%d", cfg.route_to[i].route_zone);
    }
    wrefresh (sub_win);
}

void
RouteManager ()
{
    int x = 0, bDone = 0, key;
    char s[80];
    char temp[50];
    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Route Manager");
        mvwprintw (sub_win, 2, 5, "#");
        mvwprintw (sub_win, 2, 12, "Route To");
        mvwprintw (sub_win, 2, 34, "Zone");
        for (int i = 0; i < 11; i++)
        {
            sprintf (temp, "%d:%d/%d.%d", cfg.route_to[i].zone,
                     cfg.route_to[i].net, cfg.route_to[i].node,
                     cfg.route_to[i].point);
            if ((cfg.route_to[i].zone == 999) || (cfg.route_to[i].zone == 0))
                strcpy (temp, " ");
            sprintf (s, "%-2d     %-18s      %-5d", i, temp,
                     cfg.route_to[i].route_zone);
            mvwprintw (sub_win, i + 4, 5, s);
        }
        mvwchgat (sub_win, 4 + x, 3, 36, A_REVERSE|A_BOLD, 0, NULL);
        wrefresh (sub_win);
        key = toupper (getch ());
        if (key == 0xA)
        {
            edit_route (x);
        }
        if (key == KEY_DC)
        {
            cfg.route_to[x].zone=0;
            cfg.route_to[x].net=0;
            cfg.route_to[x].node=0;
            cfg.route_to[x].point=0;
            cfg.route_to[x].route_zone=0;
        }
        if (key == 'J' || key == KEY_DOWN)
            if (++x >= 11)
                x = 0;
        if (key == 'K' || key == KEY_UP)
            if (--x < 0)
                x = 10;
        if (key == ESC || key == 'Q')
            bDone = 1;
    }
}

void
TranslationTables ()
{
    clear ();
    display_main ();
    display_title ("Translation Tables");
    refresh ();
    getch ();
}

void exp_node()
{
}
void exp_area() {}
void exp_group() {}
void exp_list() {}
void exp_subtype() {}

void
ExportManager ()
{
    int x=0,bDone=0,key;

    clear ();
    display_main ();
    while (!bDone)
    {
        display_title ("Export Manager");
        mvwprintw(sub_win,5,32,"  Node Config  ");
        mvwprintw(sub_win,6,32,"  Area Config  ");
        mvwprintw(sub_win,7,32,"  Area Groups  ");
        mvwprintw(sub_win,8,32,"   Echo List   ");
        mvwprintw(sub_win,9,32," WWIV Subtypes ");
        mvwchgat(sub_win,5+x,32,16,A_REVERSE|A_BOLD,0,NULL);
        wrefresh(sub_win);
        key=toupper(getch());
        if (key=='Q'||key==ESC) bDone=1;
        if (key=='J'||key==KEY_DOWN) if (++x > 4) x=0;
        if (key=='K'||key==KEY_UP) if (--x < 0) x=4;
        if (key==0xA) {
            if (x==0) exp_node();
            if (x==1) exp_area();
            if (x==2) exp_group();
            if (x==3) exp_list();
            if (x==4) exp_subtype();
        }
    }
}

void
MakeSubsLsts ()
{
    int Done=0;
    char key;
    while (!Done) {
        key=Ask("Make SUBS.LST? ");
        if (key=='Y') Done=1;
    }
    //    clear ();
    //    display_main ();
    //    display_title ("Make Subs List");
    //    refresh ();
    //    getch ();

}

void
AboutWWIVTOSS ()
{
    WINDOW *w;
    clear ();
    display_main ();
    display_title ("About WWIVToss");
    w=newwin(11,41,6,20);
    box(w,0,0);
    mvwprintw (w, 1, 1, "WWIVTOSS");
    mvwprintw (w, 2, 1, "Version: %s",VERSION);
    mvwprintw (w, 3, 1, "Written by Craig Dooley");
    mvwprintw (w, 4, 1, "Copyright (c) 1996-2014 by Craig Dooley");
    mvwprintw (w, 5, 1, "All Rights Reserved");
    mvwprintw (w, 6, 1, "Maintained by Mark Hoffman");
    mvwprintw (w, 8, 1, "WWIVTOSS Setup for Linux");
    mvwprintw (w, 9, 1, "Copyright (c) 2014-2018 Eric Manuel Pareja");
    wrefresh (w);
    refresh ();
    getch ();
    delwin(w);
}

void
CheckPassword ()
{
    char s[20];
    display_main ();
    sub_win = newwin (3, 40, 10, 20);
    box (sub_win, 0, 0);
    mvwprintw (sub_win, 1, 1, " Enter WWIVTOSS Setup Password: ");
    wrefresh (sub_win);
    refresh ();
    wgetnstr (sub_win, s, 19);
    if (strcmp (s, cfg.password))
    {
        delwin (sub_win);
        endwin ();
        printf ("Sorry. Wrong Password!\n");
        exit (1);
    }
    delwin (sub_win);
}

int
main (int argc, char *argv[])
{
    int bDone = 0, x = 0;
    FILE *fp;

    read_wwiv_config ();
    if ((fp = fopen("areas.dat","r"))==NULL)
    {
        init_areas_config();
    }
    else
    {
        fclose(fp);
    }
    if ((fp = fopen("xlat.dat","r"))==NULL)
        init_xlat_dat();
    else
        fclose(fp);
    if ((fp = fopen("wwivtoss.dat","r"))==NULL)
        init_wwivtoss_config();
    else
        fclose(fp);
    if ((fp = fopen("nodes.dat","r"))==NULL)
        init_nodes_config();
    else
        fclose(fp);
    if ((fp = fopen("domains.dat","r"))==NULL)
        init_domains_config();
    else
        fclose(fp);
    if ((fp = fopen("uplinks.dat","r"))==NULL)
        init_uplink_config();
    else
        fclose(fp);



    SetUpScreen ();

    read_wwivtoss_config ();
    read_domain_config ();
    read_nodes_config ();
    read_uplink_config ();
    if (strlen (cfg.password))
        CheckPassword ();
    sub_win = newwin (22, 78, 2, 1);
    while (!bDone)
    {
        display_main ();
        display_title ("Main Menu");
        mvwprintw (sub_win, 5, 30, "[1] System Data");
        mvwprintw (sub_win, 6, 30, "[2] Miscellaneous Options");
        mvwprintw (sub_win, 7, 30, "[3] Area Manager");
        mvwprintw (sub_win, 8, 30, "[4] Node Manager");
        mvwprintw (sub_win, 9, 30, "[5] Route Manager");
        mvwprintw (sub_win, 10, 30, "[6] Translation Tables");
        mvwprintw (sub_win, 11, 30, "[7] Export Manager");
        mvwprintw (sub_win, 12, 30, "[8] Make SUBS.LSTs");

        mvwprintw (sub_win, 13, 30, "[9] About WWIVTOSS");
        mvwchgat (sub_win, 5 + x, 30, 25, A_REVERSE|A_BOLD, 0, NULL);


        wrefresh (sub_win);
        int key = getch ();
        key = toupper (key);
        switch (key)
        {
        case 0xA:
            ungetch ('1' + x);
            break;
        case 'K':
        case KEY_UP:
            if ((--x) < 0)
                x = 8;
            break;
        case 'J':
        case KEY_DOWN:
            if ((++x) > 8)
                x = 0;
            break;
        case '0':
            print_wwivtoss_config ();
            break;
        case '1':
            SystemData ();
            break;
        case '2':
            MiscOptions ();
            break;
        case '3':
            AreaManager ();
            break;
        case '4':
            NodeManager ();
            break;
        case '5':
            RouteManager ();
            break;
        case '6':
            TranslationTables ();
            break;
        case '7':
            ExportManager ();
            break;
        case '8':
            MakeSubsLsts ();
            break;
        case '9':
            AboutWWIVTOSS ();
            break;
        //        case 'W':
        //            write_wwivtoss_config ();
        //            break;
        case ESC:
        case 'Q':
            bDone = 1;
            break;
        default:
            break;
        }

    }
    delwin (sub_win);
    clear();
    endwin ();
    write_uplink_config();
    write_domain_config();
    write_nodes_config();
    write_wwivtoss_config ();
    printf ("Thanks for using WWIVTOSS Alt Setup!\n");
    return (0);
}

void
sort_areas(void)
{
    int i,j;
    long pos;
    arearec a,b,c;

    for (i=1; i<num_areas+1; i++)
    {
        read_area(i,&a);
        for (j=1; j<num_areas+1; j++)
        {
            read_area(j,&b);
            if (strcmp(b.name,a.name)>0)
            {
                c=b;
                b=a;
                a=c;
                write_area(i,&a);
                write_area(j,&b);

            }
        }
    }
}
