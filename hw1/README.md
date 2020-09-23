# HW1 Short Answer Questions  
With each HW Assignment, there will be a few short answer questions that for you to fill out.  
These questions will hopefully require you to reflect back on your experiences with the assignment.  
The questions can be found on the HW specification on the website. Please write at least a few sentences for each question.

## Question 1
##### To take this class, you must have done some Object-Oriented Programming in Java. Name a few (>=3) things that you noticed were more complex and/or difficult when building data structures in C.
1. Pointers in C will make things confusing and messy. If I write code in
 Java, I don't need to care about the pointers.
2. hard to debug. Constantly showing up seg-faults but don't tell you why
. Even though we can backtrace the error, but there are still many ways to cause
 the seg-fault, for example, dereferencing NULL pointers, * Incorrect use of the "&" (address of) and "*" (dereferencing) operators.
 Figuring out which one could be difficult.
3. need to take care of the memory myself. I need to manually allocate and free
 memory. I also need to avoid memory leaks myself.

## Question 2
##### What are some (>=2)  advantages to developing data structures in C when compared to languages like Java, which have a higher level of abstraction?
1. I can immediately free anything that I finished using, thus I can have a
 greater usage of the memory
2. performance will be better since C directly compile code directly into
 machine code. However, java need to interpret code into bytecode and 
 run them on Java virtual machine.

## Question 3
##### For these data structures to be generic, we used a void\* for the Payloads of such structures. What difficulties did you experience with this implementation of generics? What pitfalls does this implementation of generics have?
We can't dereference a void pointer, and we also need to cast it to suppress the warning. For example, we need to pass in & of a pointer and cast it into type (void*) to the second argument of LLIterator_Get, which took me some while to figure it out.
