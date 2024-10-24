## Goal:
This is a file managment system that processes multiple large files in parallel using multiprocessing and multithreading, while employing inter-process communication (IPC) mechanisms for message passing between processes. 

## How to Use:

For Google Colab(what I used):
1. Download files and Structure your directory like this:  
  ![image](https://github.com/user-attachments/assets/74dc5881-0a9f-49c6-bc41-ec0a0370db50)
   
2. Replace 'the' with desired target word, '4' with desired number of threads, '0' with the desired mode of processing
   
   **0 for Multithreaded mode, 1 for single threaded mode**
     
![image](https://github.com/user-attachments/assets/7b694ea8-8a35-4936-b247-449fc9f2852c)

   
3. Run 



For Linux:
1. download files into the same directory
2. ensure dependent libraries are installed: gcc, pthread, and librt
3. bash:  
   gcc fileManager.c -o fileManager -pthread -lrt  
   ./fileManager the 4 0 /content/bib /content/paper1 /content/paper2 /content/progc /content/progl /content/progp /content/trans
