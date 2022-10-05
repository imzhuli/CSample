#include <xel/X_GetOpts.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    for(int i = 0 ; i < argc; ++i) {
        printf("%i: %s\n", i, argv[i]);
    }

    X_opterr = 0;
    int opt = 0;
    while(true) {
        opt = X_getopt(argc, argv, "a:bc:");
        if (opt == -1) {            
            if(X_optind < argc) {
                printf("error on index=%i, param=%s, continue...\n", X_optind, argv[X_optind]);                
                X_optind++;
                X_optreset = 1;
                continue;
            }
            printf("quit getopt\n");
            break;
        }

        printf("Option: %c, optopt: %c, last_arg=%s \n", (char)opt, (char)X_optopt, (X_optarg ? X_optarg : ":nil"));
        switch(opt) {
            case 'a':
                continue;
            case 'b':
                continue;
            case 'c':
                continue;
            case '?':
                continue;
            case ':':
                continue;
            default:
                printf("unknown params\n");
                exit(-1);
        }
    }

    return 0;
}
