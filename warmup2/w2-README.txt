Documentation for Warmup Assignment 2
=====================================

+-------+
| BUILD |
+-------+

Comments: Build the file using make command. warmup2 is the target file specified as per make command
		:make warmup2

+-----------------+
| SKIP (Optional) |
+-----------------+

All the tests in the test suite are working and if required prints the appropriate error messages.

+---------+
| GRADING |
+---------+

Basic running of the code : 100 out of 100 pts

Missing required section(s) in README file : n/a
Cannot compile : n/a
Compiler warnings : n/a
"make clean" : cleans the executable warmup2.
Segmentation faults : not seen for the given test cases
Separate compilation : n/a
Using busy-wait : 0
Handling of commandline arguments:
    1) -n : yes,no points to be deducted
    2) -lambda : yes,no points to be deducted
    3) -mu : yes,no points to be deducted
    4) -r : yes,no points to be deducted
    5) -B : yes,no points to be deducted
    6) -P : yes,no points to be deducted
Trace output :
    1) regular packets: printing according to the required format.
    2) dropped packets: printing according to the required format.
    3) removed packets: printing according to the required format.
    4) token arrival (dropped or not dropped): printing according to the required format.
Statistics output :
    1) inter-arrival time : output is within the required precision.
    2) service time :  output is within the required precision.
    3) number of customers in Q1 : output is within the required precision.
    4) number of customers in Q2 : output is within the required precision.
    5) number of customers at a server :  output is within the required precision.
    6) time in system : output is within the required precision.
    7) standard deviation for time in system : printing correctly
    8) drop probability : output is within the required precision.
Output bad format : Output is printed considering the given specification.
Output wrong precision for statistics (should be 6-8 significant digits) : output is printed within the required precision.
Large service time test : working properly
Large inter-arrival time test : working properly
Tiny inter-arrival time test : working properly
Tiny service time test : working properly
Large total number of customers test : working properly
Large total number of customers with high arrival rate test : working properly
Dropped tokens test : working properly
Cannot handle <Cntrl+C> at all (ignored or no statistics) : handling
Can handle <Cntrl+C> but statistics way off : Can handle Ctrl+C
Not using condition variables and do some kind of busy-wait : not using busy wait.
Synchronization check : synchronization check is taken care of.
Deadlocks : no deadlocks

+------+
| BUGS |
+------+

Comments: bug-free

+------------------+
| OTHER (Optional) |
+------------------+

Comments on design decisions:
Comments on deviation from spec:


