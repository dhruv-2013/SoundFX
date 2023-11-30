#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
 
#define SAMPLE_RATE 44100  // Sample rate in Hz
#define BITS_PER_SAMPLE 16 // Bit depth
 
#define ECHO_DELAY 0.5     // Echo delay in seconds
#define ECHO_DECAY 0.5     // Decay rate of the echo
 
#define MAX_DELAY_MS 30   // Maximum chorus delay in milliseconds
#define DEPTH 0.5         // Chorus depth
#define RATE 1.5          // Chorus rate in Hz
#define MIX 0.5           // Mix level (0.0: only dry signal, 1.0: only wet signal)
 
int wav_to_pcm(char *input, char *ouput){
    size_t result;
    char  *buf;
    FILE *fp1=fopen(input,"rb");   //open wav file, read only
    FILE *fp2=fopen(ouput,"wb");   //create and open pcm file, write only
    fseek(fp1,0,SEEK_END);              //move the pointer to the end of file
    long filesize;
    filesize=ftell(fp1);                //calculate the bit number of the file
 
    if(fp1==NULL||fp2==NULL)            //check if both files are correctly opened
    {  
        printf("file open failed!!");
        return 0;
    }
 
    rewind(fp1);                                    //reset the pointer
    fseek(fp1,44,SEEK_SET);                         //move the pointer to the 44 bit of wav file,
    buf=(char *)malloc(sizeof(char)*filesize);      //allocate memory for buffer array
 
    if(buf==NULL)
    {
        printf ("memory  error");
        return 0;
    }  
 
    result =fread(buf,1,(filesize-44),fp1);         //read every bit and store into buffer, count the bit numberf
    if(result!=filesize-44)                         //check if the counter equals to the file size at the end
    {
        printf("readng error!!");
        return 0;
    }      
    fwrite(buf,1,(filesize-44),fp2);                //wite into pcm file
    fclose(fp1);                                    //close the file pointer
    fclose(fp2);
    free (buf);                                     //release the memory
    return 0;
}
 
int pcm_to_wav(char *input, char *ouput)
{
    typedef struct {
        char riff[4];                               // "RIFF"
        unsigned int overall_size;
        char wave[4];                               // "WAVE"
        char fmt_chunk_marker[4];                   // "fmt "
        unsigned int length_of_fmt;
        unsigned short format_type;
        unsigned short channels;
        unsigned int sample_rate;
        unsigned int byterate;
        unsigned short block_align;
        unsigned short bits_per_sample;
        char data_chunk_header[4];                  // "data"
        unsigned int data_size;
    } wav_header;
 
    // Open PCM file and create WAV file
    FILE *pcm_file = fopen(input, "rb");
    FILE *wav_file = fopen(ouput, "wb");
 
    if (pcm_file == NULL || wav_file == NULL) {
        fprintf(stderr, "Error opening files!\n");
        return 1;
    }
 
    // Define WAV header
    wav_header wavHeader;
    int headerSize = sizeof(wav_header), filelength = 0;
 
    const int FORMAT_PCM = 1;
    const int CHANNELS_STEREO = 2;
    const int BYTE_RATE = (SAMPLE_RATE * CHANNELS_STEREO * BITS_PER_SAMPLE) / 8;
    const int BLOCK_ALIGN = (CHANNELS_STEREO * BITS_PER_SAMPLE) / 8;
 
    // Initialize WAV header
    strncpy(wavHeader.riff, "RIFF", 4);
    strncpy(wavHeader.wave, "WAVE", 4);
    strncpy(wavHeader.fmt_chunk_marker, "fmt ", 4);
    wavHeader.length_of_fmt = 16;
    wavHeader.format_type = FORMAT_PCM;
    wavHeader.channels = CHANNELS_STEREO;
    wavHeader.sample_rate = SAMPLE_RATE;
    wavHeader.byterate = BYTE_RATE;
    wavHeader.block_align = BLOCK_ALIGN;
    wavHeader.bits_per_sample = BITS_PER_SAMPLE;
    strncpy(wavHeader.data_chunk_header, "data", 4);
 
    fseek(pcm_file, 0, SEEK_END);                   // seek to end of file
    filelength = ftell(pcm_file);                   // get current file pointer
    fseek(pcm_file, 0, SEEK_SET);                   // seek back to beginning of file
 
    wavHeader.data_size = filelength;
    wavHeader.overall_size = filelength + headerSize - 8;
 
    // Write the header
    fwrite(&wavHeader, headerSize, 1, wav_file);
 
    // Write the PCM data
    unsigned char *buffer = (unsigned char*) malloc(BYTE_RATE);
    int read = 0;
 
    while ((read = fread(buffer, 1, BYTE_RATE, pcm_file)) > 0) {
        fwrite(buffer, 1, read, wav_file);
    }
 
    // Clean up
    free(buffer);
    fclose(pcm_file);
    fclose(wav_file);
 
    return 0;
}
 
// Function to change the playback speed of the audio
int speed_change(char *input, char *output, int speed){
    char* psTest = (char*)malloc(4);
    int tmp = 0;
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_change = fopen(output,"wb+");
 
    // Loop through the input file
    while (!feof(fp))
    {
        fread(psTest,1,4,fp);
        // Skip samples according to the specified speed
        if(tmp % speed != 0)
        {
            fwrite(psTest,1,2,fp_change);
            fwrite(psTest+2,1,2,fp_change);
        }
        tmp++;
    }
 
    // Free allocated memory and close files
    free(psTest);
    fclose(fp);
    fclose(fp_change);
    return 0;
}
 
// Function to cut a segment from the audio
int cut(char *input, char *output, int start, int length){
    char* psTest = (char*)malloc(4);
    int tmp = 0;
    int begin_time = start * 100000; // Calculate start time in samples
    int run_time = length * 100000; // Calculate duration in samples
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_cut = fopen(output,"wb+");
    FILE* fp_txt = fopen("interception info.txt","wb+");
 
    // Loop through the input file
    while (!feof(fp))
    {
        fread(psTest,1,2,fp);
 
        // Write the specified segment to the output file
        if (tmp > begin_time && tmp <= (begin_time + run_time))
        {
            fwrite(psTest,1,2,fp_cut);
            short sample_data = psTest[1];
            sample_data = sample_data * 256;
            sample_data = sample_data + psTest[0];
            fprintf(fp_txt,"%6d,",sample_data);
 
            if (tmp % 10 == 0)
            {
                fprintf(fp_txt,"\n");
            }
        }
        tmp++;
    }
 
    // Free allocated memory and close files
    free(psTest);
    fclose(fp);
    fclose(fp_cut);
    fclose(fp_txt);
    return 0;
}
 
// Function to separate audio into two channels (left and right)
int channel_seperate(char *input){
    char* psTest = (char*)malloc(4);
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_L = fopen("L.pcm","wb+");
    FILE* fp_R = fopen("R.pcm","wb+");
 
    // Loop through the input file
    while (!feof(fp))
    {
        fread(psTest,1,4,fp);
        fwrite(psTest,1,2,fp_L);    // Write left channel
        fwrite(psTest+2,1,2,fp_R);  // Write right channel
    }
 
    // Free allocated memory and close files
    free(psTest);
    fclose(fp);
    fclose(fp_L);
    fclose(fp_R);
    return 0;
}
 
// Function to change the volume of the audio
int volume_change(char *input, char *output, float x){
    char* psTest = (char*)malloc(4);
    x = 1/x;
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_change = fopen(output,"wb+");
 
    // Loop through the input file
    while (!feof(fp))
    {
        short* ps = NULL;
        fread(psTest,1,4,fp);
        ps = (short*)psTest;
        *ps = *ps / x;  // Adjust volume
 
        fwrite(psTest,1,2,fp_change);
        fwrite(psTest+2,1,2,fp_change);
    }
 
    // Free allocated memory and close files
    free(psTest);
    fclose(fp);
    fclose(fp_change);
    return 0;
}
 
// Function to add an echo effect to the audio
int echo(char *input, char *output){
    int16_t *buffer, *echoBuffer;
    int bufferSize, echoBufferSize, echoOffset, i;
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_change = fopen(output,"wb+");
 
    if (fp == NULL || fp_change == NULL) {
        perror("Error opening file");
        return 1;
    }
 
    // Calculate echo buffer size and offset
    echoOffset = SAMPLE_RATE * ECHO_DELAY;
    fseek(fp, 0, SEEK_END);
    bufferSize = ftell(fp) / sizeof(int16_t);
    echoBufferSize = bufferSize + echoOffset;
 
    // Allocate memory for buffers
    buffer = (int16_t *)malloc(bufferSize * sizeof(int16_t));
    echoBuffer = (int16_t *)calloc(echoBufferSize, sizeof(int16_t));
    if (buffer == NULL || echoBuffer == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
 
    // Read input PCM data
    fseek(fp, 0, SEEK_SET);
    fread(buffer, sizeof(int16_t), bufferSize, fp);
 
    // Apply echo effect
    for (i = 0; i < bufferSize; i++) {
        echoBuffer[i] += buffer[i];
        if (i + echoOffset < echoBufferSize) {
            echoBuffer[i + echoOffset] += buffer[i] * ECHO_DECAY;
        }
    }
 
    // Write output PCM data
    fwrite(echoBuffer, sizeof(int16_t), echoBufferSize, fp_change);
 
    // Clean up
    fclose(fp);
    fclose(fp_change);
    free(buffer);
    free(echoBuffer);
 
    return 0;
}
 
// Function to add a chorus effect to the audio
int chorus(char *input, char *output){
    int16_t *buffer, *echoBuffer;
    int bufferSize, echoBufferSize, echoOffset, i;
 
    FILE* fp = fopen(input,"rb+");
    FILE* fp_change = fopen(output,"wb+");
 
    if (fp == NULL || fp_change == NULL) {
        perror("Error opening file");
        return 1;
    }
 
    int maxDelaySamples = (int)((MAX_DELAY_MS / 1000.0) * SAMPLE_RATE);
    short *delayBuffer = (short *)malloc(maxDelaySamples * sizeof(short));
    if (!delayBuffer) {
        perror("Memory allocation failed");
        return 1;
    }
 
    int readSamples;
    short inputSample, outputSample;
    int bufferIndex = 0;
    double lfoPhase = 0;
    double lfoIncrement = 2 * M_PI * RATE / SAMPLE_RATE;
 
    // Loop through the input file
    while ((readSamples = fread(&inputSample, sizeof(short), 1, fp)) > 0) {
        double delay = (sin(lfoPhase) + 1) / 2 * maxDelaySamples * DEPTH;
        int delaySamples = (int)delay;
 
        int readIndex = bufferIndex - delaySamples;
        if (readIndex < 0) readIndex += maxDelaySamples;
 
        short delayedSample = delayBuffer[readIndex];
        outputSample = (short)((1 - MIX) * inputSample + MIX * delayedSample);
 
        delayBuffer[bufferIndex] = inputSample;
        bufferIndex = (bufferIndex + 1) % maxDelaySamples;
 
        fwrite(&outputSample, sizeof(short), 1, fp_change);
 
        lfoPhase += lfoIncrement;
        if (lfoPhase >= 2 * M_PI) lfoPhase -= 2 * M_PI;
    }
 
    // Free allocated memory and close files
    free(delayBuffer);
    fclose(fp);
    fclose(fp_change);
 
    return 0;
}
 
 
int reverse(char *input, char *ouput)
{
    FILE* fp = fopen(input,"rb+");
    FILE* fp_change = fopen(ouput,"wb+");
 
    if (fp == NULL || fp_change == NULL) {
        perror("Error opening file");
        return 1;
    }
 
    // Seek to the end of the file to determine the file size
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);
 
    // Calculate the number of samples
    int numSamples = fileSize / sizeof(short);
 
    // Allocate memory for the audio data
    short *buffer = (short *)malloc(fileSize);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(fp);
        fclose(fp_change);
        return 1;
    }
 
    // Read the audio data
    fread(buffer, sizeof(short), numSamples, fp);
 
    // Write the audio data in reverse order
    for (int i = numSamples - 1; i >= 0; --i) {
        fwrite(&buffer[i], sizeof(short), 1, fp_change);
    }
 
    // Clean up
    free(buffer);
    fclose(fp);
    fclose(fp_change);
 
    return 0;
}
 // Function to apply different audio effects on WAV files
int edit_wav(char *inputFile, int effectChoice, int interceptStart, int interceptLength, int speedChange, int volumeMultiply){
    char output[50];
    int len = strlen(inputFile);
   // Switch case based on the effect choice
    if (effectChoice == 1){
        // Convert WAV to PCM format
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, ".pcm");
        wav_to_pcm(inputFile, output);
    }
   
    else if (effectChoice == 2){
        wav_to_pcm(inputFile, "temp.pcm");
        channel_seperate("temp.pcm");
        pcm_to_wav("L.pcm", "L.wav");
        pcm_to_wav("R.pcm", "R.wav");
 
        remove("temp.pcm");
        remove("L.pcm");
        remove("R.pcm");
    }
           
    else if (effectChoice == 3){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_intercepted.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        cut("temp.pcm", "temp2.pcm", interceptStart, interceptLength);
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else if (effectChoice == 4){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_speedchanged.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        speed_change("temp.pcm", "temp2.pcm", speedChange);
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else if (effectChoice == 5){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_volumechanged.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        volume_change("temp.pcm", "temp2.pcm", volumeMultiply);
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else if (effectChoice == 6){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_echo.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        echo("temp.pcm", "temp2.pcm");
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else if (effectChoice == 7){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_chorus.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        chorus("temp.pcm", "temp2.pcm");
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else if (effectChoice == 8){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_reversed.wav");
 
        wav_to_pcm(inputFile, "temp.pcm");
        reverse("temp.pcm", "temp2.pcm");
        pcm_to_wav("temp2.pcm", output);
 
        remove("temp.pcm");
        remove("temp2.pcm");
    }
    else{
        return 0;
    }
    return 0;
}
 
// Function to apply different audio effects on PCM files
int edit_pcm(char *inputFile, int effectChoice, int interceptStart, int interceptLength, int speedChange, int volumeMultiply) {
    char output[50];
    int len = strlen(inputFile);
 // Switch case based on the effect choice
    if (effectChoice == 1){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, ".wav");
 
        pcm_to_wav(inputFile, output);
    }
   
    else if (effectChoice == 2){
        channel_seperate(inputFile);
    }
           
    else if (effectChoice == 3){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_intercepted.pcm");
 
        cut(inputFile, output, interceptStart, interceptLength);
    }
    else if (effectChoice == 4){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_speedchanged.pcm");
 
        speed_change(inputFile, output, speedChange);
    }
    else if (effectChoice == 5){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_volumechanged.pcm");
 
        volume_change(inputFile, output, volumeMultiply);
    }
    else if (effectChoice == 6){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_echo.pcm");
 
        echo(inputFile, output);
    }
    else if (effectChoice == 7){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_chorus.pcm");
 
        chorus(inputFile, output);
    }
    else if (effectChoice == 8){
        strcpy(output, inputFile);
        output[len - 4] = '\0';
        strcat(output, "_reverse.pcm");
 
        reverse(inputFile, output);
    }
    else{
        return 0;
    }
    return 0;
}
 
int main(int argc, char *argv[]){
     // Check for correct number of arguments
    if (argc == 1) {
        fprintf(stderr, "Incorrect Argument Number\n");
        return 1;
    }
 
      // Parse command line arguments
    int fileType = atoi(argv[1]);  // File type (WAV or PCM)
    char *inputFile = argv[2];     // Input file name
    int effectChoice = atoi(argv[3]);  // Effect to be applied
    int interceptStart = atoi(argv[4]); // Start time for intercepting
    int interceptLength = atoi(argv[5]); // Length of intercept
    int speedChange = atoi(argv[6]);    // Factor for speed change
    int volumeMultiply = atof(argv[7]); // Factor for volume change
 
    if (fileType == 1) {
        edit_wav(inputFile, effectChoice, interceptStart, interceptLength, speedChange, volumeMultiply);
    } else if (fileType == 2) {
        edit_pcm(inputFile, effectChoice, interceptStart, interceptLength, speedChange, volumeMultiply);
    } else{
        printf("Invalid output, please enter again\n");
    }
    return 0;
}