￼
# HW2 Short Answer Questions  
With each HW Assignment, there will be a few short answer questions for you to fill out.  
These questions will hopefully require you to reflect back on your experiences with the assignment.  
Please write at least a few sentences for each question.

## Question 1
##### In this assignment, you had to make substantial use of the data structures in C that you have built so far. As a user of these data-structures, what difficulties did you encounter? If you did not have many difficulties, describe anything notable you encountered while using them. Write >= 3 detailed sentences.
1. Since we hide the implementation of these struct, we need to “LinkedList a = LinkedList_Allocate()” to create a LinkedList and make the storage size known instead of merely “LinkedList a”;
2. These data-structures are all struct in nature and for data structure as commonly used as HashTable and LinkedList, we need to include the definition and the implementation of them every time we use them.
3. Manipulating data structures in C is harder than in Python. Not only
 because Python has built-in functions for data structure like LinkedList and HashTable, but also because the parameters for those functions are simpler: We don’t need to worry about pointers when we pass in the parameters.


## Question 2
##### Creating the file system crawler, indexer, and search engine required working with C-strings (char*). What were a few (>=3) things you had to consider when working with char* compared to the use of a more properly defined String object that would be provided in other languages?
1. C-string has less built-in functions to use. When we need to parse a string, Java and Python String all have split method.
2. C-string is char array in nature. However, python string is not. We need to cast it into list to make it something similar to a char array.
3. Strings in Java and Python are all immutable, meaning we cannot overwrite their values but we can reassign them. However, for C-string’s char array, we can change any letter we want.



## Question 3
##### In this homework, we made use of the POSIX API to read files, and crawl directories. List a couple (>= 2) plausible reasons as to why we used POSIX instead of the C standard library functions for this assignment. Be sure to explain your reasoning.

1. Because c standard library doesn’t provide directory manipulation like the
 function like readdir
2. POSIX calls are unbuffered and we don’t need a buffer for this assignment. Having a buffer for this project will not improve the performance that much since we read the whole file once inside ReadFileToString and then parse them into the word position HashTable inside HandelFile and then search inside the table.


## Question 4
##### In searchshell.c you had to come up with ways of dealing with user input. What were some things (>= 2) you encountered when handling user input that were challenging? How does user input handling compare to other languages you have programmed in? (>=2 comparisons)

challenging
1. The function that handle the user input isn’t smart enough. For example, fgets will include \n in the end and I will need to manually remove that. My searchshell didn’t work at first and it took me hours to check my implementation logic again and again, and it turns out to be because I didn’t remove \n at the end. 
2. If we want to parse the sentence into array of words, we need to read them into a buffer array first character by character and parse them word by word and store them into an array. It took more steps and details than Java or Python. 

comparison
1. Python input(), will convert all input into string first. However in C, we can use scanf() and accept formatted input and more types of input.
2. Java uses Scanner to take input. We have to create a new Scanner object first. In C, we don’t need that. We directly call the function. However, java’s String library has many useful function, like split, to make the parsing easy.




