#include <xel/X_GetOpts.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    for(int i = 0 ; i < argc; ++i) {
        printf("%i: %s\n", i, argv[i]);
    }

    X_OptErr = 0;
    int opt = 0;
    while(true) {
        opt = X_GetOpt(argc, argv, "a:bc:");
        if (opt == -1) {            
            if(X_OptInd < argc) {
                printf("error on index=%i, param=%s, continue...\n", X_OptInd, argv[X_OptInd]);                
                X_OptInd++;
                X_OptReset = 1;
                continue;
            }
            printf("quit getopt\n");
            break;
        }

        printf("Option: %c, optopt: %c, last_arg=%s \n", (char)opt, (char)X_OptOpt, (X_OptArg ? X_OptArg : ":nil"));
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
