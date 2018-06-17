# parental_control
parental control for windows

PARENTAL CONTROL service and agent (x64):


1) Sevice Installation
   - install the service by running the following script: "install_service.bat"
   - it will install a service called my_parental_control running process parental_control.exe (task manager)
   - windows -> run -> services.msc you will see it 
   - the script will install the service as persistent, so it will start automatically on boot
   - if for some reason you don't want anymore this service running on your computer run the "uninstall_service.bat"
   
   
2) policy.txt format example:
   - dzivi 20:00 24:00 60 
   - mean user called dzivi is allowed to access the computer from 20:00 to 24:00 for duration of maximum 60 minutes
   - note: currently only the duration limitation is working 
   - user that doesn't appair in the policy file will not be limited at all
   
3) User Loging history:
   - is the folder where the service is running a file for every user will be generated, this file will contain all the logging history of the user
   
4) Agent installation:
   - in order to run the parental control agent on every loging a "shortcut" of the "ParentalControlAgent.exe" need to be copy to the "Startup" directory which is for dzivi user under:
     C:\Users\dzivi\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\
     
     
5) when running parental control agent a dll may miss in your system, this dll is called vcruntime.dll. To install it run the vc_redistx64.exe file 
    
   
   
   
 Sequence diagram
 
 
 
1)	The contain two main parts:
a.	Parental Control Server 
b.	Parental Control Agent
Parental Control Server 
Parental control Server is responsible to establish a named pipe and listening for new users that just logged in. On every login the Parental Control Service will create a new thread called “Agent Handler” that will monitor the time consumption of the new connected user. 
Parental Control Agent 
On every loggin a parental control agent process start automatically and create a connection with Parental Control Server. The Parental Control Server monitor the time consumption of the client and when it’s over a “logout” message is sent. Upon “logout” message arrival the Agent generate an 30 second alert to end user in order to save files and then the Agent close the current windows session.

The technical challenges encountered in this project were as follow:
•	Create a reliable communication channel between server and agent
•	Create a windows service that start automaticaly on startup 
•	Create a agent process that start automaticaly for every user logged in
•	Create a new monitoring agent thread for every connection

Testing
The testing procedure includes the following aspects:
•	Sanity check i.e. checking basic functionality of the program 
•	Platform check i.e. checking the functionality of the program on various windows operating systems (windows 7,8 and 10)
•	Check on isolated user
•	Check on multiple users
During the following stages of testing various problem were found:
Sanity check:
•	Wrong configuration of the named pipe
•	Communication issues between client and server
•	Failure to accumulate time

Platform check: 
•	failure on windows 8 due to missing runtime dll






