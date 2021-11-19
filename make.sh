echo Building allegro_test
cd allegro_test
g++ -lallegro -lallegro_image -lallegro_font -o allegro_test allegro_test.cpp || exit
echo Done
