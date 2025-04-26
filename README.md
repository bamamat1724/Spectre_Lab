# Spectre_Lab
Lab for ECE 699 going through the spectre side channel attack



# PREPARING THE ENVIRONMENT

download VMWare

https://blogs.vmware.com/workstation/2024/05/vmware-workstation-pro-now-available-free-for-personal-use.html

Download the ubuntu image (likely will work with other images when this one goes EOL)

https://releases.ubuntu.com/jammy/

Run these commands
sudo apt-get update
sudo apt-get upgrade
sudo apt install gcc
sudo apt install g++
sudo apt install git

Ensure you are vulnerable to the spectre_v1 bug

cat /proc/cpuinfo | grep bugs

Get Spectre Lab files from my github

cd ~

git clone https://github.com/bamamat1724/Spectre_Lab.git

cd Spectre_Lab



# PROFILING

Run the appendix C code from the spectre paper.

gcc ./spectre_appendix_c.c -o default_spectre

./default_spectre

Are you able to find the hidden phrase?
What address did the hidden phrase string start? 

Lets figure out a good cache hit/miss threshold. Lets run the profiler and plot it out as a histogram. It may be worthwhile running this step again even if you have profiled in the past due to changes in processor load or from differences in profiling methods others may use

gcc ./spectre_profiler.c -o profile

./profile > histogram.csv

plot the results in excel or openoffice. The first column is # clock cycles, the second is hits, and the third is misses

How do the scores for the spectre_appendix_c change when you use a better threshold value?
Modify line 33 with an appropriate threshold value found from the histogram.

gcc ./spectre_appendix_c.c -o default_spectre

./default_spectre




# CHANGING NUMBER OF RUNS

Lets change to the spectre_development.c program which has been modified to better display more accurate score values. Be sure to update the threshold value in the spectre_development program.

gcc ./spectre_development.c -o modified_spectre

./modified_spectre

Change the number of tries in the code and recompile. How does this effect the number of errors in the secret word?




# BENCHMARKING SPECTRE

spectre_development_1kb.c has a code word that is 1024 bytes long and runs a timer which times how long it takes to read the kB of data. 

gcc ./spectre_development_1kb.c -o benchmark

./benchmark

How long does it take to read a kB of data at 100 tries? 50 tries? 5 tries? 1 try? (change line 49)
How many errors do you count for each of these options?

The spectre paper claimed to be able to read 10kB/s with error rate of <0.01%. Does this roughly match what you are seeing, if not, how?




# EXPLORING MEMORY

Use the arbitrary address program to read the addresses before and after the hidden string. It has two additional input arguments for a memory location and a number of bytes to read. Since most memory locations have 0x00, this program has been modified to not display values of 0x00 so that the text is easier to see without as much searching. Remember to change the threshold and number of tries!

gcc ./spectre_development_mem_dump.c -o mem_dump

./mem_dump 0xffffffffffffe000 8192

Do you see any other strings that you recognize? What is the lowest address that you can find readable text and what does it say? (Hint: No need to search before 0xffffffffffffbb00)

Open the executable file you just ran in a text editor (e.g. mem_dump). What is the first word you see in the text editor? What conclusions can you make about the data being read with the spectre attack based on this?




# COMPARING MEMORY TO EXECUTABLE

Run the program_dump program and store the output to a file. Do not change the tries value, but do change the threshold in this program. 

gcc ./spectre_program_dump.c -o file_dump

./file_dump 0xffffffffffffb000 25000 > test_file

open file_dump and test_file in text editors and compare them side by side. Compare this new file to the original file. What differences do you see? Why do you think this is? Remove any extra symbols before the \7F ELF if that helps to compare. 

Does increasing the number of tries fix any of the differences you saw?
