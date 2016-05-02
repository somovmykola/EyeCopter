//on bootup this program is called which infinitely takes photos

#include <stdio.h>
#include <stdlib.h>

void main(){
	system("> /home/pi/camera/PNames.txt");
	while(1){
		system("/home/pi/camera.sh");
		sleep(1);
	}	
}
