#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

// Funksjon for å gjette tallet mellom en gitt rekkevidde
int guess(int max, int min){ 
    return (max + min)/2; 
}

int main(){
    int file_descriptor1[2], file_descriptor2[2]; // Filbeskrivelser for pipes

    // Opprette pipes
    if(pipe(file_descriptor1) == -1 ){  
        printf("Feil: Kunne ikke opprette pipe \n");
        return 1;
    }

    if(pipe(file_descriptor2) == -1 ){ 
        printf("Feil: Kunne ikke opprette pipe2 \n");
        return 2;
    }

    // Fork en barneprosess
    pid_t id = fork();  

    // Feilhåndtering for forking
    if(id == -1){   
        printf("Feil: Kunne ikke utføre forking \n");
        return 3;
    }
    
    // Barneprosess
    else if(id == 0){ 
        printf("Child id= %d, med prosessid %d\n",id,getpid());
        bool found = false;
        int guessed_number, answer;

        // Generer et tilfeldig tall for gjettelek
        srand(time(NULL));
        int random_number = (rand() % 998) + 1;
        
        // Lukk unødvendige ender av pipene
        close(file_descriptor1[0]);
        close(file_descriptor2[1]);
        
        // Løkke for å sjekke om tallet er funnet
        while(!found){
            read(file_descriptor2[0], &guessed_number, sizeof(guessed_number));

            if(guessed_number < random_number){ // Hvis gjettet er for lavt
                answer = 1;
                write(file_descriptor1[1], &answer, sizeof(int)); 
            }

            else if(guessed_number > random_number){ // Hvis gjettet er for høyt
                answer = -1;
                write(file_descriptor1[1], &answer, sizeof(int));
            }

            else{ // Hvis gjettet er riktig
                answer = 0;
                write(file_descriptor1[1], &answer, sizeof(int));
                found = true;
            }
        }
    }

    // Foreldreprosess
    else if (id > 0){   

        bool found2 = false;
        int guess_number, answer;
        int max = 1000;
        int min = 0, amount = 0;

        // Lukk unødvendige ender av pipene
        close(file_descriptor1[1]);
        close(file_descriptor2[0]);

        // Løkke for å sjekke om tallet er funnet
        while(!found2){ 
            guess_number = guess(max, min); 
            
            // Send gjett til barneprosessen
            write(file_descriptor2[1], &guess_number, sizeof(int));
            // Motta respons fra barneprosessen
            read(file_descriptor1[0],&answer ,sizeof(int));
            
            // Øk antall forsøk
            amount ++;
            if(answer == -1){ // Hvis gjettet er for høyt
                max = guess_number - 1;
            }
            else if (answer == 1){ // Hvis gjettet er for lavt
                min = guess_number + 1;    
            }
            else{ // Hvis gjettet er riktig
                printf("Parrent id = %d, med prosessid %d\n", id, getpid());
                printf("Tallet er funnet = %d, og antall forsøk = %d\n", guess_number, amount);
                found2 = true;
            }
        }
        // Lukk gjenværende ender av pipene
        close(file_descriptor1[0]);
        close(file_descriptor2[1]);
    
        // Vent på at barneprosessen skal avslutte
        wait(NULL); 
    }
    return 0;
}
