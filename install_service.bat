

set service_path=%cd%\ParentalControl.exe
sc create my_parental_control binpath= %service_path%  
sleep 1
sc config my_parental_control start= auto
sleep 1
sc start my_parental_control 
