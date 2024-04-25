#include <stdio.h>

int main() {
    // File pointer for input and output file
    FILE *input_file, *output_file;
    char ch;

    // Open the input file in read mode
    input_file = fopen("/dev/mydriver", "r");

    // Check if the input file exists and opened successfully
    if (input_file == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    // Open the output file in write mode
    output_file = fopen("output.txt", "w");

    // Check if the output file opened successfully
    if (output_file == NULL) {
        printf("Error opening the file.\n");
        fclose(input_file);
        return 1;
    }

    // Read character by character from the input file and write to the output file
    while ((ch = fgetc(input_file)) != EOF) {
        fputc(ch, output_file);
    }

    // Close the files
    fclose(input_file);
    fclose(output_file);

    printf("File copied successfully.\n");

    return 0;
}
