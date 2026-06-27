For the extension, I implemented leak detection. To do so, I added three frame_t 
in each header to track the three previous frames. I also added requested_size
to track how many bytes the user actually requested, allowing malloc_report() to
access that information. Finally, I modified the cstart.c file so that 
malloc_report() is always called after main(). 

One thing I noticed is that adding this extra data impacts the performance in
heap_workflow.c. This makes sense, but I am wondering what tradeoffs exist
between performance and error detection.  