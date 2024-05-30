CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread
INC_DIR = -Iheaders

Vulkan: boat_runner.cpp
	g++ $(CFLAGS) -o BoatRunner boat_runner.cpp $(LDFLAGS) $(INC_DIR)

.PHONY: run clean

run: Vulkan
	./BoatRunner

clean:
	rm -f BoatRunner
