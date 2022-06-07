superspeedyoptionspricingmake: superspeedyoptionspricing.cpp
	g++ -o superspeedyoptionspricing-linux-x64 -static superspeedyoptionspricing.cpp
	x86_64-w64-mingw32-g++ -static -o superspeedyoptionspricing-windows.exe superspeedyoptionspricing.cpp
