#!/bin/bash
# Script name: BATS.sh
# 
# Description: Automates testing of building, and testing of AMStartup with avatar
#   
#
# Command line options: None.
#
# Input: None.
#
# Output: If run successfully, a log file will be produced containing the start and end time of the build, 
#			hostname and operating system where the build was run, status at different stages of the build, 
#			and the results of some tests with AMStartup and avatar.
# 	
# Special considerations: None.
#
# Pseudocode: 
#	Build AMStartup and avatar
#	Run test cases on AMStartup
#		For each test case, print description, expected result and system output


# the test log
log_file=AMStartupTestlog.`date +"%a_%b_%d_%T_%Y"`		

# server hostname to test on
server="pierce.cs.dartmouth.edu"

declare -a TEST					# the type of test
declare -a TEST_CMD				# commands corresponding to different user runs
declare -a TEST_EXPD			# expected output

TEST[0]="Testing with no arguments"
TEST_EXPD[0]="Usage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]"
TEST_CMD[0]="./AMStartup"

TEST[1]="Testing with invalid option"
TEST_EXPD[1]=`printf "./AMStartup: invalid option -- 'b'\nUsage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]"`
TEST_CMD[1]="./AMStartup -b"

TEST[2]="Testing --help"
TEST_EXPD[2]=`printf "The Amazing Project\nComponent: AMStartup\nVersion 1.0\nUsage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]"`
TEST_CMD[2]="./AMStartup --help"

TEST[3]="Testing with no option flags"
TEST_EXPD[3]="Usage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]"
TEST_CMD[3]="./AMStartup 2 0 $server"  		

TEST[4]="Testing with insufficient number of arguments"
TEST_EXPD[4]="Usage: ./AMStartup -n nAvatars -d Difficulty -h Hostname [--help]"
TEST_CMD[4]="./AMStartup -n 2 -d 0"

TEST[5]="Testing with invalid number of avatars (ex. 11)"
TEST_EXPD[5]="Error: Number of avatars must be an integer from 1 to 10"
TEST_CMD[5]="./AMStartup -n 11 -d 0 -h $server"

TEST[6]="Testing with non-integer number of avatars (ex. 5.6)"
TEST_EXPD[6]="Error: Number of avatars must be an integer from 1 to 10"
TEST_CMD[6]="./AMStartup -n 5.6 -d 0 -h $server"

TEST[7]="Testing with non-numeric number of avatars (ex. g)"
TEST_EXPD[7]="Error: Number of avatars must be an integer from 1 to 10"
TEST_CMD[7]="./AMStartup -n g -d 0 -h $server"

TEST[8]="Testing with invalid difficulty level (ex. 10)"
TEST_EXPD[8]="Error: Difficulty must be an integer from 0 to 9"
TEST_CMD[8]="./AMStartup -n 1 -d 10 -h $server"

TEST[9]="Testing with non-integer difficulty level (ex. 5.6)"
TEST_EXPD[9]="Error: Difficulty must be an integer from 0 to 9"
TEST_CMD[9]="./AMStartup -n 1 -d 5.6 -h $server"

TEST[10]="Testing with non-numeric difficulty level (ex. g)"
TEST_EXPD[10]="Error: Difficulty must be an integer from 0 to 9"
TEST_CMD[10]="./AMStartup -n 1 -d g -h $server"

TEST[11]="=========BEGIN: UNIT TESTING FUNCTIONS OF AVATAR========="
TEST_EXPD[11]="Exit status of testing program is 0 (0 = success)"
TEST_CMD[11]="./testing"	

TEST[12]="Testing correct command line options, with 2 avatars at difficulty 0"
TEST_EXPD[12]=`printf "Success! Solved maze of difficulty 0, with 2 avatars, in 79 moves.\nThe hash returned by AM_MAZE_SOLVED is ___.\nLog file complete!"`
TEST_CMD[12]="./AMStartup -n 2 -d 0 -h $server"

TEST[13]="Testing correct command line options, with 4 avatars at difficulty 2"
TEST_EXPD[13]=`printf "Success! Solved maze of difficulty 2, with 4 avatars, in 256 moves.\nThe hash returned by AM_MAZE_SOLVED is ___.\nLog file complete!"`
TEST_CMD[13]="./AMStartup -n 4 -d 2 -h $server"


# start the log
num_tests=14	
touch $log_file

# start the log
printf "TESTING AMStartup with avatar...\n"
echo "---------------------------------------------------" >> $log_file
echo "" >> $log_file
echo "========== TESTING AMStartup with avatar ==========" >> $log_file
echo "" >> $log_file
echo "---------------------------------------------------" >> $log_file
printf "\n" >> $log_file
printf "\n" >> $log_file
printf "\n-----------------------------------------\n" >> $log_file

printf "Date: `date`\n" >> $log_file
printf "=========================================\n\n" >> $log_file
printf "Hostname: $(hostname)\n" >> $log_file
printf "Operating System: $(uname -o)\n\n" >> $log_file
printf "Build start: `date`\n\n" >> $log_file

# build AMStartup and avatar
make >> $log_file

# get the time stamp  at the end of the build
printf "\nBuild end: `date`\n\n" >> $log_file	

printf "=========================================\n\n" >> $log_file



# now start the tests
for ((i = 0; i < $num_tests; i++)); do
	printf "${TEST[i]}\n" >> $log_file
	if ((i != 11)); then
		printf "\n${TEST_CMD[i]}\n" >> $log_file
	fi
	# print the expected result
	printf "\nEXPECTED RESULT: \n${TEST_EXPD[i]}\n" >> $log_file
	printf "\nSYSTEM OUTPUT: \n" >> $log_file
	# run the command to get the system output
	${TEST_CMD[i]}  >> $log_file  2>&1
	if ((i == 11)); then
		printf "\n\nExit status of testing program is $? (0 = success)\n" >> $log_file
		printf "\n=========END: UNIT TESTING FUNCTIONS OF AVATAR=========" >> $log_file
	fi
	if ((i == 12)); then
		echo "" >> $log_file
		tail -3 Amazing_$USER\_2_0.log >> $log_file
		# remove avatar-produced logfile
		rm -f Amazing_$USER\_2_0.log
	fi
	if ((i == 13)); then
		echo "" >> $log_file
		tail -3 Amazing_$USER\_4_2.log >> $log_file
		# remove avatar-produced logfile
		rm -f Amazing_$USER\_4_2.log
	fi

	printf "\n---------------------------------------\n" >> $log_file

done

#now clean up AMStartup and avatar
make clean


printf "Testing Complete!\n"
printf "See $log_file for log\n"

exit 0

















