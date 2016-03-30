This is a unfinished project, intended to analyze interactions of the double networks of twitter (the network of user and the network of information). The ultimate goal is to construct a prediction model based on behavior analysis. 

A temperary result is shown in the document result-Trump2016.xlsx, where one could observe how the users' network grows around a specific piece of information (in this case, #Trump2016). Such dynamic description of "network pieces" would be the training data for Stage3 and Stage4 of this project. 

This 2nd version of the project is effectively shut down, due to several reasons: 1st, current method of tracking dynamic changing of social network is inefficient; 2nd, current codes were not written with consideration of integrating with Luigi; 3rd, current code is inefficient in terms of hardware usage.

########################################################################################

1. written with C++;

2. need ODBC packages, rapidjson (already included)

3. need SQL server 2014; better if has SQL server management studio,
since there are a few operations that is not performed by the code

4. original data file already included; it is good for testing Stage1 and Stage2_1
However, for Stage2_2, it is recommended to collect a bigger data file, at least 1 GB, 
so that one could has sufficient time span to observe the graph for certain hash tag.

5. ONLY codes are included; Data base not included; C++ project files not included.
