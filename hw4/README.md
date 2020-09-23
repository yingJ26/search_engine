# HW3 Short Answer Questions  
With each HW Assignment, there will be a few short answer questions for you to fill out.  
These questions will hopefully require you to reflect back on your experiences with the assignment.  
Please write at least a few sentences for each question.

## Question 1
##### This homework had you write a program that worked very closely with the network. List a few (>=2) things that you had to account for when writing code that directly interacted with the network. (Interactions could include reading/writing to the network, managing connections, and setting up the network server in ServerSocket.cc)
1. I need to account for possible failures in calling functions like
 getaddrinfo, socket, and do different things to handle the failures. Like I
  need to directly return false when getaddrinfo failed, but I could loop to
   the next address when we failure to connect a socekt
2. The address could be IPv6 or IPv4 and I need to handle them differently
3. When reading and writing to the network, we need also need to take care of
 the security vulnerabilities like "cross-site scripting" flaw.


## Question 2
##### In this homework, you wrote a program that worked with concurrency. List a few (>=2) notable things that you encountered when writing code or read provided code that took advantage of concurrency or was run concurrently.
1. test code for HttpConnection.cc creates a thread to write on the other end
 of the socket to test the code
2. test code for HttpConnection.cc use the sleep property of the thread to
 wait for the reading, thus successfully test the implementation
3. the Run() method in HttpServer.cc creates a thread pool to dispatch
 connections to the working threads



## Question 3
##### Why do we try to read many bytes at a time with WrappedRead when we call HttpConnection::GetNextRequest? Please explain your reasoning. 
We choose to read many bytes at a time with WrappedRead is because Network IO
 has very high latency, so we choose to read many bytes at a time to reduce
  the frequency we call the Network IO.


