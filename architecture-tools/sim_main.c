//
// Nis Wegmann, 2017
//
// sim_main.c
//

#include "isa.h"
#include "sim_dump.h"
#include "sim_step.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------------------------- //

// Memory size.
static size_t const kMemorySize = 0x10000; // 1 MB

extern int main(int argc, const char * argv[])
{
    // Resources.
    
    FILE * inputFile = NULL;
    FILE * traceFile = NULL;
    isa_State * state = NULL;
    unsigned char * backup = NULL;
    Annotation * annotation = NULL;
    
    // Validate input arguments.
    
    if (argc < 2)
    {
        printf("Usage: asm input-file [options*]\n");
        printf("  -nodiff               no diff of result state\n");
        
        annotation_usage();
        
        return EXIT_SUCCESS;
    }
    
    // Create the annotator.
    
    annotation = create_annotation(argc - 1, argv + 1);
    
  #if 0
    if (annotation == NULL)
    {
        printf("Could not create the annotator!\n");
        
        goto cleanup;
    }
  #endif
    
    // Open input-file.
    
    char const * inputFilename = argv[1];
    
    inputFile = fopen(inputFilename, "r");
    
    if (inputFile == NULL)
    {
        printf("Couldn't open input-file '%s'\n", inputFilename);
        
        goto cleanup;
    }
    
    // Open trace-file.
    
    char const * traceFilename = "trace.txt";
    
    traceFile = fopen(traceFilename, "w+");
    
    if (traceFile == NULL)
    {
        printf("Couldn't open trace file '%s'\n", inputFilename);
        
        goto cleanup;
    }
    
    // Allocate and read memory.
    
    state = isa_State_create(kMemorySize); 
    
    backup = malloc(kMemorySize);
    
    if (state == NULL || backup == NULL)
    {
        printf("Memory allocation failure.\n");
        
        goto cleanup;
    }
    
    fread(state->memory, 1, kMemorySize, inputFile);
    
    if (getc(inputFile) != EOF)
    {
        printf("Memory block is not large enough to contain program.\n");
        
        goto cleanup;
    }
    
    memcpy(backup, state->memory, kMemorySize);
    
    // Run simulation.
    
    isa_Status status = isa_Status_aok;
    
    for (int k = 0; k < 1000; k++)
    {
        status = sim_step(state, traceFile, annotation);
        
        switch (status)
        {
            case isa_Status_aok:
                
                continue;
                
            case isa_Status_adr:
                
                fprintf(stderr, "bad access at: %016" PRIX64 "\n", state->ip);
                
                goto cleanup;
                
            case isa_Status_hlt:
                
                goto dump;
                
            case isa_Status_ins:
                
                fprintf(stderr, "bad instruction at: %016" PRIX64 "\n", state->ip);
                
                goto cleanup;
        }
    }
    
dump:
    
    // Dump simulator state and changes to the memory.
    
    sim_dumpState(state, backup);
    
    if (annotation != NULL)
    {
        print_annotation(annotation);
    }
    
    // Free resources.
    
cleanup:
    
    if (backup != NULL)
    {
        free(backup);
    }
    
    if (state != NULL)
    {
        isa_State_destroy(state);
    }
    
    if (traceFile != NULL)
    {
        fclose(traceFile);
    }
    
    if (inputFile != NULL)
    {
        fclose(inputFile);
    }
    
    if (annotation != NULL)
    {
        free_annotation(annotation);
    }
}

// ---------------------------------------------------------------------------------------------- //