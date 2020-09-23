# HW3 Short Answer Questions  
With each HW Assignment, there will be a few short answer questions for you to fill out.  
These questions will hopefully require you to reflect back on your experiences with the assignment.  
Please write at least a few sentences for each question.

## Question 1
##### HW3 had you writing the in-memory structure from HW2 into a file on disk. What are some (>=2) advantages of storing this structure to a file? What is a disadvantage?
advantage
1. the data will be more persistent since file will still be there after I
 terminate my program.
2. the Hashtable will be re-loaded quicker since I don't need to re-build it
 to start my search engine. I will only need to read the file.
disadvantage
1. I need to do more work. Not only do I need to build the Hashtable first
, but also I need to write a program to write the Hashtable into a file.
2. anytime the Hashtable got updated, I will need to re-read the table
 into a file, which is not suitable for a web-searching engine since the data
  got updated every seconds and I will unlikely need to re-load the old
   version of the file.


## Question 2
##### At this point you have written some very similar non-trivial programs in C and C++. In HW2 you implemented searchshell.c and Memindex.c, while in this HW 3 you implemented the similar programs QueryProcessor.cc and filesearchshell.cc. List (>=2) notable differences in the experience you had writing in C vs in C++.
1. C++'s STL is very powerful. It supports operation like sort, push_back
, insert, etc, which are very convenient. If I need these operations in C, I
 will need to implement them myself.
2. C uses structs to group features into things. C++ has uses more classes and
 objects to group features into things.


## Question 3
##### Note that in FileIndexReader.cc STEP 1, we set it so that reading/writing to the index file was unbuffered. We do this so that we can use our own strategy of reading the "in-file" HashTable, and avoid the default buffering strategy provided by the C standard library. Why would turning off the buffer be more efficient than leaving it on in our use case of reading files?  
###### (Recall that if you were to read the first byte of a file with fread and buffering on, it would instead read the first many (likely >= 512) bytes and store it in an internal buffer before returning the one byte you requested. This is so that if you want to read the next byte in the file, fread() doesn't have to go to the OS and disk to read it, instead just returning the bytes it already read).
Since we don't need to re-access what we've read. We read them once and store
 them inside the hashtable. If we use buffer, we will need to copy things
  into buffer first then parse them into the hashtable.


